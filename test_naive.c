#include "sched_simul.h"

alloc_set** pack_naive(int task_num, int bin_num, task_info** task,int* bin_config, int* task_config){
	
	alloc_set** res = (alloc_set**)malloc(sizeof(alloc_set*)*bin_num);
	for(int i=0;i<bin_num;i++)
		res[i] = (alloc_set*)malloc(sizeof(alloc_set)*bin_num);
	//init bin.
	for(int i=0;i<bin_num;i++)
		init_set(res[i],bin_config[i]);
	
	//alloc task.
	for(int i=0;i<task_num;i++){
		int target_bin = task_config[i];
		generate_overhead(task[i],res[target_bin]->chip_num);
		insert_task_to_set(res[target_bin],task[i]);
	}
	return res;
}
int test_naive(int task_num, task_info** task, float* throughput){
	/* a test code for naive allocation.
	 * naively allocate chips to each task in mutually-exclusive manner.
	 * in here, we do not consider utilization.
	 * return pointer to array which records allocation result.
	 */

	//calculate configuration. currently hard coded for 8 12 16 32.
	//bin_config -> chip per bin.
	//task_config -> bin index for task.
	int* bin_config;
	int* task_config;
	int bin_num;
	if(task_num >= 8 && task_num <= 11){
		bin_num = task_num;
		bin_config = (int*)malloc(sizeof(int)*task_num);
		task_config = (int*)malloc(sizeof(int)*task_num);
		for(int i=0;i<task_num;i++){
			task_config[i] = i;
			bin_config[i] = 1;
		}
		for(int i=0;i<16 - task_num;i++)
			bin_config[i]++;
	}
	else if (task_num >= 12 && task_num <= 15){
		bin_num = task_num;
		bin_config = (int*)malloc(sizeof(int)*task_num);
		task_config = (int*)malloc(sizeof(int)*task_num);
		for(int i=0;i<task_num;i++){
			task_config[i] = i;
			bin_config[i] = 1;
		}
		for(int i=0;i<16-task_num;i++)
			bin_config[i]++;
	}
	else if (task_num >= 16 && task_num <= 32){
		bin_num = 16;
		bin_config = (int*)malloc(sizeof(int)*task_num);
		task_config = (int*)malloc(sizeof(int)*task_num);
		for(int i=0;i<16;i++){
			task_config[i] = i;
			bin_config[i] = 1;
		}
		for(int i=16;i<task_num;i++)
			task_config[i] = i % 16;
	}
	else if (task_num == 32){
		bin_num = 16;
		bin_config = (int*)malloc(sizeof(int)*16);
		task_config = (int*)malloc(sizeof(int)*task_num);
		for(int i=0;i<task_num;i++){
			task_config[i] = i/2;
			if(i<16)
				bin_config[i] = 1;
		}
	}
	else if (task_num >= 4 && task_num <= 7){
		bin_num = task_num;
		bin_config = (int*)malloc(sizeof(int)*task_num);
		task_config = (int*)malloc(sizeof(int)*task_num);
		for(int i=0;i<task_num;i++){
			task_config[i] = i;
			bin_config[i] = 2;
		}
		for(int i=0;i<8-task_num;i++)
			bin_config[i] += 2;
	}
	else{
		printf("naive allocation does not support this number of task.\n");
		abort();
	}

	alloc_set** naive_set = pack_naive(task_num,bin_num,task,bin_config,task_config);

	//test naive_set info.
	for(int i=0;i<bin_num;i++){
		printf("set info : %d, ", naive_set[i]->chip_num);
		printf("%d, ",naive_set[i]->task_num);
		printf("%f\n",naive_set[i]->total_task_util);
	}
	//test schedulability of naive_set.
	int bin_pass = 0;
	int sched = 0;
	for(int i=0;i<bin_num;i++){
		float total_util = 0.0;
		int blocking_period = calc_blocking_set(naive_set[i]);
		printf("original util : %f, bp : %d, new_util : %f\n",
		naive_set[i]->total_task_util,
		calc_blocking_set(naive_set[i]),
		naive_set[i]->total_task_util + (float)ERASE_LTN / (float)blocking_period);
		naive_set[i]->total_task_util += (float)ERASE_LTN / (float)blocking_period;
		if(naive_set[i]->total_task_util < 1.0)
			bin_pass++;
	}
	if (bin_pass == bin_num)
		sched = 0;
	else
		sched = -1;
	
	//calc bandwidth of each bin.
	for(int i=0;i<bin_num;i++){
		throughput[0] += calc_RT_write(1,1.0 - naive_set[i]->total_task_util,naive_set[i]->chip_num);
		throughput[1] += calc_RT_read(1,1.0 - naive_set[i]->total_task_util,naive_set[i]->chip_num);
	}
	if(sched == -1){
		throughput[0] = 0.0;
		throughput[1] = 0.0;
	}
	return sched;
}

		
		
		
	
		

