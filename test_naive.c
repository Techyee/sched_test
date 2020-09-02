#include "sched_simul.h"

int* test_naive(int task_num, task_info** task){
	/* a test code for naive allocation.
	 * naively allocate chips to each task in mutually-exclusive manner.
	 * in here, we do not consider utilization.
	 * return pointer to array which records allocation result.
	 */

	//init
	int i;
	int* ttc_alloc = (int*)malloc(sizeof(int)*task_num);
	int chip = CHANNEL_NB * WAY_NB;
	int chip_per_task = chip / task_num;
	int j = 0;
	//!init
	
	if (chip_per_task == 0){
		printf("naive allocation impossible!\n");
		return;
	}

	for(i=0;i<chip;i++){//record the alloc.
		//i-th chip to j-th task(starting from 0th).
		ttc_alloc[i] = j;
		if(i % chip_per_task == 0){
		//if j-th task get its share, shift.
			if(task_idx < task_num)
				task_idx++;
		}	
	}
	return ttc_alloc;
}

int test_sched(int task_num, task_info** task, int* ttc_alloc){
	/* a schedulability test code given a task_info and task to chip allocation.
	 * check if each chip is OK with given workload.
	 * must check if how many tasks are sharing a channel.
	 * task utilization can be calculated considering dt_overhead and gc_overhead.
	 */

	//init
	int i;
	int chip = CHANNEL_NB * WAY_NB;
	int num_chip_per_task[task_num] = {0, };
	int cur_task;
	int cur_sched;	
	//!init

	//check a number of chip per task.
	int status[task_num]= {-1, };
	int idx_task = 0;
	int idx_chip = 0;
	for(i=0;i<chip;i++){
		for(j=0;j<task_num;j++){
			if(ttc_alloc[i] == task_num)
				num_chip_per_task[j] += 1;
		}
	}

	//search through channels.
	for(i=0;i<CHANNEL_NB;i++){
		cur_sched = 0;
		
		//find the current chan status through checking chips
		for(j=0;j<WAY_NB;j++){
			cur_task = ttc_alloc[4*i+j];
			status[cur_task] += 1;
		}

		//check sched if current channel has j-th task on some chip
		for(j=0;j<task_num;j++){
			if(status[j] != -1){
				generate_overhead(task[j],num_chip_per_task[j]);
				generate_dt(task[j],CHANNEL_NB - status[j]);
			}
		}

		//problem case : task_sched > 1.0
		if(task[j]->task_util > 1.0){
			cur_sched = 1;//detected
		}
		
		//reset the status value.
		for(j=0;j<task_num;j++)
			status[j] = -1;
			
	}
	
	//return schedulability.
	if(cur_sched == 0){
		printf("taskset is schedulable\n");
		return 0;
	}
	else{
		printf("taskset is unschedulable\n");
		return 1;
	}
}			
			
		
		
		
	
		

