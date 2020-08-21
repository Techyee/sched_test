#include "sched_test.h"

int pack_channelbin(int task_num, task_info** task,double util_sum)
{
	/* assuming channel is partitioned, allocate tasks to each bin in WFD.
	 * WFD orders tasks according to utilization of GC.
	 * pack the tasks even though bin overflows.(just try to evenly spread)
	 * possible bin = [1,1,1,1] [2,1,1] [2,2] [3,1]
     */
	int res;
	double bins[4];
	if(util_sum <= 2.0){

		/*regenerate the total taskset utilization according to 
 		 */
		 bins[0] = 0.0;
		 bins[1] = 0.0;
		 bins[2] = -1.0;
		 bins[3] = -1.0;
	}

	else if(util_sum <= 3.0){
		bins[0] = 0.0;
		bins[1] = 0.0;
		bins[2] = 0.0;
	}

}    
	

int pack_waybin(int task_num, task_info** task)
{
	/* partition in both of channel and way.
	 * WFD orders tasks according to utilization of GC.
	 * bin overflow directly results in schedulability test fail.
	 */


}

