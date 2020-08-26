#include "sched_simul.h"

int pack_channelbin(int task_num, task_info** task,double util_sum)
{
	/* assuming channel is partitioned, allocate tasks to each bin in WFD.
	 * WFD orders tasks according to utilization of GC.
	 * pack the tasks even though bin overflows.(just try to evenly spread)
	 * possible bin = [1,1,1,1] [2,1,1] [2,2] [3,1]
     */

	int res, i, j, bin_max, target_idx;
	float util1, util2;
	double bins[4];

	if(util_sum <= 2.0){

		 bins[0] = 0.0;
		 bins[1] = 0.0;
		 bins[2] = -1.0;
		 bins[3] = -1.0;

		//case2: [3-chan,1-chan] bin
		for(i=0;i<task_num;i++){//we sort the task according to worst case gc among the bins.
			generate_gcinfo(task[i],4);
		}
		
		//init and sort.
		bin_max = 2;
		target_idx = 0;
		quick_sort(task,0,task_num-1);
		
		//WFD packing
		for(i=0;i<task_num;i++){
			task_info temp;
			temp = *task[i];
			generate_gcinfo(&temp,12);
			util1 = temp.task_util;
			util2 = task[i]->task_util;
			if(1.0-bins[0]-util1 < 1.0-bins[1]-util2)//tweaked-WFD
			{
				bins[1] += util2;
			}
			else
			{
				bins[0] += util1;
			}
		}
		if((bins[0] <= 1.0) && (bins[1] <= 1.0))
		{
			printf("channel packing succedded[3,1], %f %f\n",bins[0],bins[1]);
		}
		else
		{
			printf("channel packing failed[3,1], %f %f\n",bins[0],bins[1]);
		}
		//case 2 end.


	    //case1: [2-chan,2-chan] bin
		 for(i=0;i<task_num;i++)
		 {
			 generate_gcinfo(task[i],8);
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
				if(1.0 - bins[j] >= 1.0 - bins[target_idx]){ //WFD
					target_idx = j;
				}
			 }
			 bins[target_idx] += task[i]->task_util;
		 }
		 //BP end
		
		 //check bin status.
		 if((bins[0] <= 1.0 ) &&(bins[1] <= 1.0))
		 {
			printf("channel packing succeeded[2,2],%f %f\n",bins[0],bins[1]);
			return 0;
		 }
		 else
		 {
			 printf("channel packing failed[2,2], %f %f\n",bins[0],bins[1]);
			 return 0;
		 }
		//case 1 end.
	}
}    

int pack_channel_3bin(int task_num, task_info** task, double util_sum)
{
	/*packing tasks with 3 bin. partition setting = [2,1,1]*/
	int res, i, j, bin_max, target_idx;
	float util1, util2;
	double bins[3] = {0.0 , };
	for(i=0;i<task_num;i++){
		generate_gcinfo(task[i], 4);
	}
	bin_max = 3;
	target_idx = 0;
	quick_sort(task,0,task_num-1);
	
	//tweked-WFD packing
	for(i=0;i<task_num=i++){
		task_info temp;
		temp = *task[i];
		generate_gcinfo(&temp,8);
		util1 = temp.task_util;
		util2 = task[i]->task_util;
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
			if(1.0 - bins[1] >= 1.0 - bins[2])
			{
				target_idx = 1;
			} 
			else 
			{
				target_idx = 2;
			}
		}



		

int pack_waybin(int task_num, task_info** task,double util_sum)
{
	/* partition in both of channel and way.
	 * WFD orders tasks according to utilization of GC.
	 * bin overflow directly results in schedulability test fail.
	 */
	return 1;

}

