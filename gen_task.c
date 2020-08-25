#include "sched_simul.h"

//parameters
//based on Utilitarian Performance Isolation(UPI) paper

//!parameters

task_info* generate_taskinfo(int tid, double util1, double util2, int rnum, int wnum)
{
	/* making a task_info structure based on random.
	   utilization is given, and read/write amount is selected as random.
	   we will read/write 1~30 pages randomly. 
	   garbage collection overhead is also accounted.
	   task_util is summation of read and write. */
	
	
	//record basic information of the task according to the input.
	task_info* new_task = (task_info*)malloc(sizeof(task_info));	
	new_task->task_id = tid;
	new_task->read_num = rnum;
	new_task->write_num = wnum;

	//if num is not specified, randomnize the num.
	if(rnum == -1)
		new_task->read_num = rand()%30 + 1;
	if(wnum == -1)
		new_task->write_num = rand()%30 + 1;
	
	//decide the period of each task according to utilization.(gc period set to -1)
	new_task->read_period = new_task->read_num*READ_LTN * 1.0/util1;
	new_task->write_period = new_task->write_num*WRITE_LTN * 1.0/util2;
	new_task->gc_period = -1;
	//handling edge cases.
	if(util1 == 0.0)
	{
		new_task->read_num = 0;
		new_task->read_period = -1;
	}
	if(util2 == 0.0)
	{
		new_task->write_num = 0;
		new_task->write_period = -1;
	}
	//sum all utilization.
	new_task->task_util = (float)new_task->read_num*READ_LTN / (float)new_task->read_period + 
						  (float)new_task->write_num*WRITE_LTN / (float)new_task->write_period;
	
	printf("task ID %d is generated.\n",new_task->task_id);
	return new_task;
}

int generate_gcinfo(task_info* task, int chip)
{
	
	double gc_threshold = (double)chip*(1.0-OP_RATE)*PAGE_PER_BLOCK;
	double gc_period_ratio = 0.0;
	double gc_period_float = 0.0;
	int temp;
	//generate GC only if there's write task(identified by positive write period).
	if(task->write_period > 0)
	{
		//check how many GCs are necessary for single write job.
		gc_period_ratio = (float)task->write_num/(float)gc_threshold;

		//determine the gc period.
		if(gc_threshold < task->write_num)
		{
			gc_period_float = (float)task->write_period/gc_period_ratio;
		}
		else
		{
			temp = (int)(1.0/gc_period_ratio);
			gc_period_float = (float)task->write_period*temp;
		}

		//record the gc period.
		task->gc_period = (int)gc_period_float;
		
		printf("gc_exec : %f, gc_period : %f, util : %f\n",(float)GC_EXEC , (float)task->gc_period ,(float)GC_EXEC / (float)task->gc_period);
		//update the task_util.
		task->task_util = (float)task->read_num*READ_LTN / (float)task->read_period +
						  (float)task->write_num*WRITE_LTN / (float)task->write_period +
						  (float)GC_EXEC / (float)task->gc_period;
	}
	else
	{
		/*do nothing*/
	}
	return 0;
}
int destroy_taskinfo(int task_num, task_info** task)
{
	int i;
	for(i=0;i<task_num;i++)
	{
		printf("task ID %d will be freed.\n",task[i]->task_id);
		free(task[i]);
	}
	return 0;
}

int print_taskinfo(task_info* task)
{
	printf("====== taskID : %d ======\n",task->task_id);
	printf("read_page : %d\n",task->read_num);
	printf("read_exec : %d\n",task->read_num*READ_LTN);
	printf("read_period : %d\n",task->read_period);
	printf("write_page : %d\n",task->write_num);
	printf("write_exec : %d\n",task->write_num*WRITE_LTN);
	printf("write_period : %d\n",task->write_period);
	printf("gc_period : %d\n",task->gc_period);
	if(task->gc_period == -1)
		printf("total_util : %f, r,w,gc :(%f,%f,-10.)\n",task->task_util,
								    (float)task->read_num*READ_LTN / (float)task->read_period,
								    (float)task->write_num*WRITE_LTN / (float)task->write_period);
	else
	{
		printf("total_util : %f, r,w,gc :(%f,%f,%f)\n",task->task_util,
									(float)task->read_num*READ_LTN / (float)task->read_period,
									(float)task->write_num*WRITE_LTN / (float)task->write_period,
									(float)GC_EXEC / (float)task->gc_period);
	}
	printf("======= INFO END =======\n");
}

task_info* generate_wandgc(int tid, double util, int wnum, int chip,FILE* fp)
{
	/* make an examplary task to compare the write intensity and gc overhead.
	 * use this information to check if the limit of write util according to the chip num.
	 */
	double gc_threshold = (float)chip*(1.0-OP_RATE)*PAGE_PER_BLOCK;
	double gc_period_ratio = 0.0;
	double gc_period_float = 0.0;
	int temp;
	
	task_info* new_task = (task_info*)malloc(sizeof(task_info));	
	new_task->task_id = tid;
	new_task->read_num = 0;
	new_task->write_num = wnum;
	new_task->read_period = 1;
	new_task->write_period =(float)wnum * (float)WRITE_LTN * 1.0 / util;

	gc_period_ratio = (float)new_task->write_num/(float)gc_threshold;
	if(gc_threshold < new_task->write_num)
	{
		gc_period_float = (float)new_task->write_period/gc_period_ratio;
	}
	else
	{
		temp = (int)(1.0/gc_period_ratio);
		gc_period_float = (float)new_task->write_period * temp;
	}
	new_task->gc_period = (int)gc_period_float;
	
	new_task->task_util = (float)new_task->read_num*READ_LTN / (float)new_task->read_period + 
						  (float)new_task->write_num*WRITE_LTN / (float)new_task->write_period +
						  (float)GC_EXEC / (float)new_task->gc_period;
	fprintf(fp,"%f, %f, %f\n",(float)new_task->write_num*WRITE_LTN/(float)new_task->write_period,
					   		  (float)GC_EXEC / (float)new_task->gc_period,
					   		  new_task->task_util);
	return new_task;
	
}

task_info** generate_taskset(int task_num, double util,int chip)
{
	// make a taskset using a generate_taskinfo function.
	int i;
	double rand_util, util1, util2;
	int rand_ratio1, rand_ratio2;
	task_info** taskset;
	
	taskset = (task_info**)malloc(sizeof(task_info*)*task_num);

	//generate uniform utilization taskset.
	for(i=0;i<task_num;i++)
	{
		rand_util = rand()%9 * 0.2 / 5 + 0.1;
		rand_ratio1 = rand()%10;
		rand_ratio2 = rand()%10;
		util1 = rand_util * (float)rand_ratio1 / ((float)rand_ratio1 + (float)rand_ratio2);
		util2 = rand_util * (float)rand_ratio2 / ((float)rand_ratio1 + (float)rand_ratio2);
		taskset[i] = generate_taskinfo(i,util1,util2,-1,-1);
		print_taskinfo(taskset[i]);
	}
	return taskset;	
}	

