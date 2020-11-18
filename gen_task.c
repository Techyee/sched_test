#include "sched_simul.h"

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
	new_task->bin_alloc = -1;
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

	new_task->task_util = 0.0;
	if(new_task->write_num != 0)
		new_task->task_util += (float)new_task->write_num*WRITE_LTN / (float)new_task->write_period;
	if(new_task->read_num != 0)
		new_task->task_util += (float)new_task->read_num*READ_LTN / (float)new_task->read_period;
	
	printf("task ID %d is generated.\n",new_task->task_id);
	return new_task;
}

int generate_overhead(task_info* task, int chip)
{
	
	double gc_threshold = (double)chip*(1.0-OP_RATE)*PAGE_PER_BLOCK;
	double gc_period_ratio = 0.0;
	double gc_period_float = 0.0;
	int temp;
	int dt_delay;
	dt_delay = ((WAY_NB - (chip%WAY_NB))%WAY_NB)*DATA_TRANS;
	//!generate DT

	//generate GC only if there's write task(identified by positive write period).
	if(task->write_num != 0)
	{
		//check how many GCs are necessary for single write job.
		gc_period_ratio = (float)task->write_num/(float)gc_threshold;
		
		//determine the gc period.
		if(gc_threshold < task->write_num)
		{
			temp = myceil(gc_period_ratio);
			gc_period_float = (float)task->write_period/temp;
		}
		else
		{
			temp = (int)(1.0/gc_period_ratio);
			gc_period_float = (float)task->write_period*temp;
		}

		//record the gc period.
		task->gc_period = (int)gc_period_float;
		
		//update the task_util.
		if(task->read_num != 0){
			task->task_util = (float)(task->read_num*(READ_LTN + dt_delay)) / (float)task->read_period +
							  (float)(task->write_num*(WRITE_LTN + dt_delay)) / (float)task->write_period +
							  (float)GC_EXEC / (float)task->gc_period;
		}
		else{
			task->task_util = (float)(task->write_num*(WRITE_LTN + dt_delay)) / (float)task->write_period +
							  (float)GC_EXEC / (float)task->gc_period;
		}
	}
	else
	{
		task->gc_period = -1;
		task->task_util = (float)task->read_num*(READ_LTN + dt_delay) / (float)task->read_period;
	}
	//!generate GC

	return 0;
}
int generate_dt(task_info* task, int chip){
	int dt_delay = 0;
	int temp;
	if(chip < WAY_NB)
		dt_delay = (WAY_NB - chip)*DATA_TRANS;
	
	if(task->gc_period > 0){
		task->task_util = (float)(task->read_num*(READ_LTN + dt_delay)) / (float)task->read_period +
						  (float)(task->write_num*(WRITE_LTN + dt_delay)) / (float)task->write_period +
						  (float)GC_EXEC / (float)task->gc_period;
	}
	else{
		task->task_util = (float)(task->read_num*(READ_LTN + dt_delay)) / (float)task->read_period +
						 (float)(task->write_num*(WRITE_LTN + dt_delay)) / (float)task->write_period;
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
	float rutil = (float)task->read_num*READ_LTN / (float)task->read_period;
	float wutil = (float)task->write_num*WRITE_LTN / (float)task->write_period;
	float gcutil = (float)GC_EXEC / (float)task->gc_period;
	printf("====== taskID : %d ======\n",task->task_id);
	printf("read_page : %d\n",task->read_num);
	printf("read_exec : %d\n",task->read_num*READ_LTN);
	printf("read_period : %d\n",task->read_period);
	printf("write_page : %d\n",task->write_num);
	printf("write_exec : %d\n",task->write_num*WRITE_LTN);
	printf("write_period : %d\n",task->write_period);
	printf("gc_period : %d\n",task->gc_period);
	if(task->gc_period == -1)
		printf("total_util : %f, r,w,gc :(%f,%f, none.)\n",task->task_util, rutil, wutil);
	else
	{
		printf("total_util : %f, r,w,gc :(%f,%f,%f), util_wo_block : %f \n",task->task_util,
																			rutil, wutil, gcutil,
																			rutil+wutil+gcutil);
	}
	printf("======= INFO END =======\n");
}

task_info* generate_ampcheck(int tid, double util, int num, int chip, int rw, FILE* fp)
{
	/* make an examplary task to compare the write intensity and gc overhead.
	 * use this information to check the limit of write util according to the chip num.
	 */
	double gc_threshold = (float)chip*(1.0-OP_RATE)*PAGE_PER_BLOCK;
	double gc_period_ratio = 0.0;
	double gc_period_float = 0.0;
	int temp;
	
	task_info* new_task = (task_info*)malloc(sizeof(task_info));	
	new_task->task_id = tid;
	if(rw == 0){//write amp check.
		new_task->read_num = 0;
		new_task->write_num = num;
		new_task->read_period = 1;
		new_task->write_period =(float)num * (float)WRITE_LTN * 1.0 / util;
	}
	else if (rw == 1){//read amp check.
		new_task->write_num = 0;
		new_task->read_num = num;
		new_task->write_period = 1;
		new_task->read_period = (float)num * (float)READ_LTN * 1.0 / util;
	}

	generate_overhead(new_task, chip);

	if(rw == 0){//record write amp result.
		fprintf(fp,"%f, %f, %f\n",
		(float)new_task->write_num*WRITE_LTN/(float)new_task->write_period,
		(float)GC_EXEC / (float)new_task->gc_period,
		new_task->task_util);
	}
	else if (rw == 1){//record read amp result.
		fprintf(fp, "%f, %f, %f\n",
		(float)new_task->read_num*READ_LTN/(float)new_task->read_period,
		0.0,
		new_task->task_util);
	}
	return new_task;
	
}

task_info** generate_heavyset(int task_num, int heavy_task_num, float threshold){
	int r_period, r_num;
	int w_period, w_num;
	float w_util, r_util;
	int leftover = (0.7 - threshold)*1000;//task util = 0.5~0.7
	int thres_div = threshold*1000;
	task_info** taskset;
	taskset = (task_info**)malloc(sizeof(task_info*)*task_num);
	for(int i=0;i<task_num;i++){
		if(i<heavy_task_num){
			w_util = threshold + rand()%leftover*0.001;
			w_period = (rand()%400 + 100) * 1000; //100~500ms
			w_num = (int)(w_util*(float)w_period / (float)WRITE_LTN);
			taskset[i] = generate_taskinfo(i,0.0,w_util,0,w_num);
		}
		else{
			w_util = rand()%thres_div * 0.001 / 2;
			w_period = (rand()%400 + 100) * 1000;
			r_util = rand()%thres_div * 0.001 / 2;
			r_period = (rand()%400 + 100) * 1000;
			w_num = (int)(w_util*(float)w_period / (float)WRITE_LTN);
			r_num = (int)(r_util*(float)r_period / (float)WRITE_LTN);
			if(r_num == 0){
				r_num = 1;
				r_period = (int)((float)(r_num * READ_LTN)/r_util);
			}
			if(w_num == 0){
				w_num = 1;
				w_period = (int)((float)(w_num * WRITE_LTN)/w_util);
			}
			taskset[i] = generate_taskinfo(i,r_util,w_util,r_num,w_num);
		}
		generate_overhead(taskset[i],16);
		print_taskinfo(taskset[i]);
	}
	return taskset;
}

task_info** copy_taskset(int task_num, task_info** target){
	task_info** new = (task_info**)malloc(sizeof(task_info*)*task_num);
	for(int i=0;i<task_num;i++){
		new[i] = (task_info*)malloc(sizeof(task_info));
		memcpy(new[i],target[i],sizeof(task_info));
	}
	return new;
}
task_info** generate_taskset(int task_num, double util,int chip,float skew, int long_p)
{
	// make a taskset using a generate_taskinfo function.
	// if long_p is specified, generate period > 100ms to ignore the blocking factor.

	int i;
	double total_util, util1, util2;
	int rand_ratio1, rand_ratio2;
	int util_ratios[task_num];
	int util_ratio_sum;
	
	int r_period;
	int w_period;
	int r_num;
	int w_num;
	int randutil_task_num; //specify a number of tasks that gets a random ratio.
	double randutil_task_utilsum; //specify a utilization which is shared between tasks.
	//init parameters
	task_info** taskset;
	total_util = util;
	util_ratio_sum = 0;
	taskset = (task_info**)malloc(sizeof(task_info*)*task_num);
	int safe_util = -1;
	//generate util ratio randomly.
	//do the exception handling(if util > 1.0, re-generate the ratio.)

util_gen:
	safe_util = -1;

	if(skew > 0.0){
		randutil_task_num = task_num/2;
		if(task_num >= 16)
			randutil_task_num = task_num/4 * 3;
		randutil_task_utilsum = util*(1.0 - skew);
	}
	else{
		randutil_task_num = task_num;
		randutil_task_utilsum = util;
	}

	while(safe_util != 1){
		util_ratio_sum = 0;
		for(i=0;i<randutil_task_num;i++){
			util_ratios[i] = rand()%10 + 1;
			util_ratio_sum += util_ratios[i];
		}
		int safe_util_cnt = 0;
		for(i=0;i<randutil_task_num;i++){
			if((float)util_ratios[i] /(float)util_ratio_sum * randutil_task_utilsum >= 1.0){
				printf("task util over 1.0. regenerating...\n");
				safe_util = -1;
				break;
			}
			else{
				safe_util_cnt += 1;
			}
		}

		if(safe_util_cnt == randutil_task_num){safe_util = 1;}

		printf("ratios :: ");
		for(i=0;i<task_num;i++)
			printf("%d ",util_ratios[i]);
		printf("\n");
	}

	//generate Uunifast style utilization taskset.
	//if necessary, specify the period, calculate the number of page, and insert it as parameter.
	for(i=0;i<task_num;i++){
		rand_ratio1 = rand()%10+1;
		rand_ratio2 = rand()%10+1;

		if(i>=randutil_task_num){
			util1 = (total_util*skew/(task_num-randutil_task_num)) * (float)rand_ratio1 / (float)(rand_ratio1+rand_ratio2);
			util2 = (total_util*skew/(task_num-randutil_task_num)) * (float)rand_ratio2 / (float)(rand_ratio1+rand_ratio2);
			printf("util1 + util2 = %f",util1+util2);
		}
		else{
			util1 = total_util*(1-skew) * ((float)util_ratios[i] / (float)util_ratio_sum) * (float)rand_ratio1 / ((float)rand_ratio1 + (float)rand_ratio2);
			util2 = total_util*(1-skew) * ((float)util_ratios[i] / (float)util_ratio_sum) * (float)rand_ratio2 / ((float)rand_ratio1 + (float)rand_ratio2);
		}

#ifdef WONLY
		util2 += util1;
		util1 = 0.0;
#endif

#ifdef RONLY
		util1 += util2;
		util2 = 0.0;
#endif
		if(long_p == 1){
			r_period = (rand()%400 + 100) * 1000;
			w_period = (rand()%400 + 100) * 1000;
	    	r_num = (int)(util1*(float)r_period / (float)READ_LTN);
			w_num = (int)(util2*(float)w_period / (float)WRITE_LTN);

			//in a case when 100~500ms is not enough to read or write at least 1 page.

#ifndef WONLY
			if(r_num == 0){//if r_num is 0 although this is NOT write-only task,
				r_num = 1;
				r_period = (int)((float)(r_num * READ_LTN)/util1);
			}
#endif

#ifndef RONLY
			if(w_num == 0){
				w_num = 1;
				w_period = (int)((float)(w_num * WRITE_LTN)/util2);
			}
			//edge case done
		}
#endif
		else{
			r_num = rand()%30 + 1;
			w_num = rand()%30 + 1;
		}
		taskset[i] = generate_taskinfo(i,util1,util2,r_num,w_num);


		generate_overhead(taskset[i],16);
		if(taskset[i]->task_util > 1.0)
		{
			printf("task %d is too intensive, util %f\n",i,taskset[i]->task_util);
			goto util_gen;
		}
		//print_taskinfo(taskset[i]);
	}
	return taskset;	
}


