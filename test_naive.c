#include "sched_simul.h"

int* test_naive(int task_num, task_info** task){
	/* a test code for naive allocation.
	 * naively allocate chips to each task in mutually-exclusive manner.
	 * in here, we do not consider utilization.
	 * return pointer to array which records allocation result.
	 */

	//init
	int i;
	int chip = CHANNEL_NB * WAY_NB;
	int chip_per_task = chip / task_num;
	int* ttc_alloc = (int*)malloc(sizeof(int)*chip);
	int j = 0;
	//!init
	
	if (chip_per_task == 0){
		printf("naive allocation impossible!\n");
		return -1;
	}

	for(i=0;i<chip;i++){//record the alloc.
		//i-th chip to j-th task(starting from 0th).
		ttc_alloc[i] = j;
		if((i+1) % chip_per_task == 0){
		//if j-th task get its share, shift.
			if(j < task_num)
				j++;
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
	int i,j;
	int chip = CHANNEL_NB * WAY_NB;
	int num_chip_per_task[task_num];
	int cur_task;
	int cur_sched;	
	int status[task_num];
	int idx_task = 0;
	int idx_chip = 0;
	for(i=0;i<task_num;i++)
		num_chip_per_task[i] = 0;
	for(i=0;i<task_num;i++)
		status[i] = 0;
	//!init
	//record number of chip per task.
	for(i=0;i<chip;i++){
		for(j=0;j<task_num;j++){
			if(ttc_alloc[i] == j)
				num_chip_per_task[j] += 1;
		}
	}
	//!record
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
			if(status[j] != 0){
				generate_overhead(task[j],num_chip_per_task[j]);
				generate_dt(task[j],WAY_NB - status[j]);
				if(task[j]->task_util > 1.0){
					cur_sched = 1;
					printf("task is unschedulable\n");
					return 1;
				}
				else if (task[j]->task_util <= 1.0){
				}
			}	
		}
	 		
		//reset the status value.
		for(j=0;j<task_num;j++)
			status[j] = 0;
			
	}
	
	//return schedulability.
	if(cur_sched == 0){
		printf("taskset is schedulable\n");
		return 0;
	}
	
}		
			
		
		
		
	
		

