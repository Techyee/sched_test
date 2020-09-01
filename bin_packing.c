#include "sched_simul.h"

int find_least_in_bin(int task_num, int target_bin, task_info** task)
{
	//among the tasks which are inside bin, chose the least period.
	int res, i;
	res = -1;
	i = 0;
	for(i=0;i<task_num;i++){
		if(task[i]->bin_alloc == target_bin){
			if(res == -1){ //init
				res = task[i]->read_period;
				if (task[i]->read_period >= task[i]->write_period){
					res = task[i]->write_period;
				}
			}
			else{//choose the minimum task_period!
				if(res >= task[i]->read_period){
					res = task[i]->read_period;
				}
				if(res >= task[i]->write_period){
					res = task[i]->write_period;
				}
			}
		}
	}
	return res;	
}

int pack_bin(int task_num, task_info** task,double util_sum,int way)
{
	/* assuming channel is partitioned, allocate tasks to each bin in WFD.
	 * WFD orders tasks according to utilization of GC.
	 * pack the tasks even though bin overflows.(just try to evenly spread)
	 * possible bin = [1,1,1,1] [2,1,1] [2,2] [3,1]
     */

	int res, i, j, k, bin_max, target_idx;
	float util1, util2;
	double bins[4];
	int blocking_period;

	if(util_sum <= 2.0){

		 bins[0] = 0.0;
		 bins[1] = 0.0;
		 bins[2] = -1.0;
		 bins[3] = -1.0;

		//case2: [3-chan,1-chan] bin
		for(i=0;i<task_num;i++)
		{//we sort the task according to worst case gc among the bins.
			if(way == 0)
				generate_overhead(task[i],4);
			else
				generate_overhead(task[i],1);
		}
		
		//init and sort.
		bin_max = 2;
		target_idx = 0;
		quick_sort(task,0,task_num-1);
		
		//WFD packing
		for(i=0;i<task_num;i++){
			task_info temp;
			temp = *task[i];
			if(way == 0)
				generate_overhead(&temp,12);
			else
				generate_overhead(&temp,3);
			util1 = temp.task_util;
			util2 = task[i]->task_util;
			printf("comparing util : %f, %f\n",util1,util2);
			if(1.0-bins[0]-util1 < 1.0-bins[1]-util2)//tweaked-WFD
			{
				bins[1] += util2;
				task[i]->bin_alloc = 1;
				printf("task %d(util:%f) in bin 1\n",i,util2);
			}
			else
			{
				bins[0] += util1;
				task[i]->bin_alloc = 0;
				printf("task %d(util:%f) in bin 0\n",i,util1);
			}
		}
		//add blocking factor.
		for(i=0;i<bin_max;i++)
		{
			blocking_period = find_least_in_bin(task_num,i,task);
			printf("original bin : %f, Blocking : %f, res : %f,\n",bins[i]
																  ,(float)ERASE_LTN / (float)blocking_period
														,bins[i] + (float)ERASE_LTN / (float)blocking_period);
			bins[i] += (float)ERASE_LTN / (float)blocking_period;
		}
		//!add blocking factor.
		
		//sched test.
		if((bins[0] <= 1.0) && (bins[1] <= 1.0))
		{
			printf("channel packing succedded[3,1], %f %f\n",bins[0],bins[1]);
//			printf("bin allocation result is...\n");
//			for(k=0;k<task_num;k++)
//			{
//				printf("%d ",task[k]->bin_alloc);
//			}
//			printf("\n");	
			return 0;
		}
		else
		{
			printf("channel packing failed[3,1], %f %f\n",bins[0],bins[1]);
		}
		//!sched test.
		//case 2 end.


	    //case1: [2-chan,2-chan] bin
		 for(i=0;i<task_num;i++)
		 {
			 if(way==0)
				 generate_overhead(task[i],8);
			 else
				 generate_overhead(task[i],2);
		 }
		 
		 //reinit and sort.
		 bins[0] = 0.0;
		 bins[1] = 0.0;
		 bin_max = 2;
		 target_idx = 0;
		 quick_sort(task,0,task_num-1);
		
		 //BP
		 for(i=0;i<task_num;i++){
			 for(j=0;j<bin_max;j++){
				if(1.0 - bins[j] >= 1.0 - bins[target_idx]){ 
					target_idx = j;
				}
			 }
			 bins[target_idx] += task[i]->task_util;
			 task[i]->bin_alloc = target_idx;
		 }
		 //BP end
		
		 //add blocking factor
		 for(i=0;i<bin_max;i++){
			blocking_period = find_least_in_bin(task_num,i,task);	
//			printf("original bin : %f, Blocking : %f, res : %f,\n",bins[i]
//																  ,(float)ERASE_LTN / (float)blocking_period
//														,bins[i] + (float)ERASE_LTN / (float)blocking_period);
			bins[i] += (float)ERASE_LTN / blocking_period;
		 }
		 //check bin status.
		 if((bins[0] <= 1.0 ) &&(bins[1] <= 1.0))
		 {
			printf("channel packing succeeded[2,2],%f %f\n",bins[0],bins[1]);
			return 0;
		 }
		 else
		 {
			 printf("channel packing failed[2,2], %f %f\n",bins[0],bins[1]);
			 return 1;
		 }
		//case 1 end.
	}
	else{
		printf("2bin fails. util >= 2.0\n");
		return 1;
	}
}    

int pack_3bin(int task_num, task_info** task, double util_sum,int way)
{
	/*packing tasks with 3 bin. partition setting = [2,1,1]
	 *pack_channel and pack_channel_3bin will be merged someday.
	 */
	int res, i, j, bin_max, target_idx;
	float util1, util2;
	double bins[3] = {0.0 , };
	int blocking_period;
	task_info* temp;
	temp = (task_info*)malloc(sizeof(task_info));
	for(i=0;i<task_num;i++)
	{
		if(way == 0)
			generate_overhead(task[i], 4);
		else
			generate_overhead(task[i], 1);
	}

	//init and sort.
	bin_max = 3;
	target_idx = 0;
	quick_sort(task,0,task_num-1);
	
	//tweked-WFD packing
	for(i=0;i<task_num;i++)
	{
		memcpy(temp,task[i],sizeof(task_info));
		if(way == 0)
			generate_overhead(temp,8);
		else
			generate_overhead(temp,2);

		util1 = temp->task_util;
		util2 = task[i]->task_util;
		//printf("large bin util = %f, small bin util = %f\n",util1,util2);
		if(1.0 - bins[0] - util1 > 1.0 - bins[1] - util2){
			target_idx = 0;
			if(1.0 - bins[0] - util1 > 1.0 - bins[2] - util2){
				target_idx = 0;
			}
			else{
				target_idx = 2;
			}
		}
		else{
			target_idx = 1;
			if(1.0 - bins[1] >= 1.0 - bins[2]){
				target_idx = 1;
			} 
			else{
				target_idx = 2;
			}
		}
		bins[target_idx] += task[i]->task_util;
		task[i]->bin_alloc = target_idx;

	}
	for(i=0;i<bin_max;i++){
		blocking_period = find_least_in_bin(task_num,i,task);
//		printf("original bin : %f, Blocking : %f, res : %f,\n",bins[i]
//															  ,(float)ERASE_LTN / (float)blocking_period
//		   													  ,bins[i] + (float)ERASE_LTN / (float)blocking_period);
		 bins[i] += (float)ERASE_LTN / (float)blocking_period;
	}
	
	if((bins[0] <= 1.0) && (bins[1] <= 1.0) && (bins[2] <= 1.0)){
		printf("channel packing succedded[2,1,1], %f %f %f\n",bins[0],bins[1],bins[2]);
		return 0;
	}
	else{
		printf("channel packing failed[2,1,1], %f %f %f\n",bins[0],bins[1],bins[2]);
		return 1;
	}
}

int pack_4bin(int task_num, task_info** task, double util_sum,int way)
{
	int res, i, j, bin_max, target_idx;
	int blocking_period;
	float util1, util2;
	double bins[4] = {0.0, };	
	for(i=0;i<task_num;i++)
	{
		if(way == 0)
			generate_overhead(task[i],4);
		else
			generate_overhead(task[i],1);
	}

	//init and sort.
	bin_max = 4;
	target_idx = 0;
	quick_sort(task,0,task_num - 1);
	//BP
	for(i=0;i<task_num;i++){
		for(j=0;j<bin_max;j++){
    		if(1.0 - bins[j] >= 1.0 - bins[target_idx]){ //WFD
				target_idx = j;
			}
		}
		bins[target_idx] += task[i]->task_util;
		task[i]->bin_alloc = target_idx;
	}
	//BP end

	//blocking calc
	for(i=0;i<bin_max;i++){
		blocking_period = find_least_in_bin(task_num,i,task);		
//		printf("original bin : %f, Blocking : %f, res : %f,\n",bins[i]
//															  ,(float)ERASE_LTN / (float)blocking_period
//						  									  ,bins[i] + (float)ERASE_LTN / (float)blocking_period);	
		bins[i] += (float)ERASE_LTN / (float)blocking_period;
	}//!blocking calc

	if((bins[0] <= 1.0) && (bins[1] <= 1.0) && (bins[2] <= 1.0) && (bins[3] <= 1.0)){
		printf("channel packing succeeded[1,1,1,1], %f %f %f %f\n",bins[0],bins[1],bins[2],bins[3]);
		return 0;
	}
	else
	{
		printf("channel packing failed[1,1,1,1], %f %f %f %f\n",bins[0],bins[1],bins[2],bins[3]);
		return 1;
	}
}




int pack_waybin(int task_num, task_info** task,double util_sum)
{
	/* partition in both of channel and way.
	 * WFD orders tasks according to utilization of GC.
	 * bin overflow directly results in schedulability test fail.
	 */
	int i,j,k,tnum, res;
	int schedulable;
	int blocking_period;
	float util_sum_per_chan = 0.0;
	double bins[4] = {0.0,};
	task_info** task_per_chan;
	
	//make channel alloc info for each tasks.
	pack_4bin(task_num,task,util_sum,0);
	//!make channel alloc

	//test way-level bp on every channel.
	for(i=0;i<CHANNEL_NB;i++){
		//init
		task_per_chan = (task_info**)malloc(sizeof(task_info*)*task_num);
		tnum = 0;
		util_sum_per_chan = 0.0;
		//!init 

		printf("grouped task indexes:");
		//group the tasks according to the bin_alloc value.
		for(j=0;j<task_num;j++){
			if(task[j]->bin_alloc == i){
				task_per_chan[tnum] = (task_info*)malloc(sizeof(task_info));
				memcpy(task_per_chan[tnum],task[j],sizeof(task_info));
				tnum++;
				util_sum_per_chan += task[j]->task_util;
				printf("%d ",j);
			}
			
		}
		printf("\n");
		//!group task.
		blocking_period = find_least_in_bin(task_num,i,task);
		util_sum_per_chan += (float)ERASE_LTN / (float)blocking_period;
		printf("channel %d task_num : %d, util_sum : %f\n",i,tnum,util_sum_per_chan);

		if(util_sum_per_chan <= 1.0){
			for(j=0;j<tnum;j++){
				free(task_per_chan[j]);
			}
			continue;
		}

		//way-level bp
		res = pack_bin(tnum,task_per_chan,util_sum_per_chan,1);
		res = pack_3bin(tnum,task_per_chan,util_sum_per_chan,1);
		res = pack_4bin(tnum,task_per_chan,util_sum_per_chan,1);
		//!way-level bp

		//free malloc spaces.
		for(j=0;j<tnum;j++)
			free(task_per_chan[j]);
		//!free

		if(res == 0)
			printf("channel idx %d is schedulable\n",i);
		else{
			printf("channel idx %d is unschedulable\n",i);
			schedulable = 1;
		}
	}
	//!end testing way-level bp
	if(schedulable == 1)
		return 1;
	else
		return 0;
}

