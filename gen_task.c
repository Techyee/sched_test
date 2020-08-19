#include "sched_simul.h"

//parameters
//based on Utilitarian Performance Isolation(UPI) paper
int PAGE_PER_BLOCK = 128;
double OP_RATE = 0.75;
int CHANNEL_NB = 4;
int WAY_NB = 4;
int READ_LTN = 50;
int WRITE_LTN = 500;
int ERASE_LTN = 5000;
int DATA_TRANS = 40;
int GC_EXEC = 550*128*0.75 + 5000;
//!parameters

task_info* generate_taskinfo(task_info* task,int tid, double util1, double util2, int chip)
{
	/* making a task_info structure based on random.
	   utilization is given, and read/write amount is selected as random.
	   we will read/write 1~30 pages randomly. 
	   garbage collection overhead is also accounted.
	   task_util is summation of read, write and gc.
	 */
	
	double gc_threshold = (double)chip*(1.0-OP_RATE)*PAGE_PER_BLOCK;
	double gc_period_ratio = 0.0;
	double gc_period_float = 0.0;
	int temp;
	
	task_info* new_task = (task_info*)malloc(sizeof(task_info));	
	new_task->task_id = tid;
	new_task->read_num = rand()%30 + 1;
	new_task->write_num = rand()%30 + 1;
	
	//decide the period of each task according to utilization.
	new_task->read_period = new_task->read_num*READ_LTN * 1.0/util1;
	new_task->write_period = new_task->write_num*WRITE_LTN * 1.0/util2;
	gc_period_ratio = (float)new_task->write_num/(float)gc_threshold;
	if(gc_threshold < new_task->write_num)
	{
		gc_period_float = (float)new_task->write_period/gc_period_ratio;
	}
	else
	{
		temp = (int)(1.0/gc_period_ratio);
		gc_period_float = (float)new_task->write_period*temp;
	}
	new_task->gc_period = (int)gc_period_float;
	
	//sum all utilization.
	new_task->task_util = (float)new_task->read_num*READ_LTN / (float)new_task->read_period + 
						  (float)new_task->write_num*WRITE_LTN / (float)new_task->write_period +
						  (float)GC_EXEC / (float)new_task->gc_period;
	
	printf("task ID %d is generated.\n",new_task->task_id);
	return new_task;
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
	printf("total_util : %f, r,w,gc :(%f,%f,%f)\n",task->task_util,
												   (float)task->read_num*READ_LTN / (float)task->read_period,
												   (float)task->write_num*WRITE_LTN / (float)task->write_period,
												   (float)GC_EXEC / (float)task->gc_period);

	printf("======= INFO END =======\n");
}

task_info** generate_taskset(int task_num, double util,int chip)
{
	// make a taskset using a generate_taskinfo function.
	int i;
	task_info* taskset[task_num];
	//generate uniform utilization taskset.
	for(i=0;i<task_num;i++)
	{
		taskset[i] = generate_taskinfo(taskset[i],i,util/(task_num*2.0),util/(task_num*2.0),chip);
		print_taskinfo(taskset[i]);
	}
	return taskset;	
}	

