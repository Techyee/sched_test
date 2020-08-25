#include "sched_simul.h"

int test_TTC(int task_num, task_info** task)
{
	int i;
	int res;
	task_info** temp;
	float util_sum = 0.0;
	
	//testing global allocation.
	printf("testing TTC allocation.\n");

	for(i=0;i<task_num;i++)
	{
		generate_gcinfo(task[i],16);
		printf("after gcinfo is generated,\n");
		print_taskinfo(task[i]);
		util_sum += task[i]->task_util;
	}

	printf("total utilization(global) is %f\n",util_sum);
	//global allocation is enough
	if(util_sum < 1.0)
	{
		printf("task is schedulable(global)\n");
		return 0;
	}

	else if(util_sum < (double)CHANNEL_NB) //try channel-level BP.
	{
		res = pack_channelbin(task_num, task, util_sum);
		if(res == 0)
		{
			printf("task is schedulable(channel)\n");
			return 0;
		}
	}

	else //if channel-level BP fails, we can also try chip-level BP.
	{
		res = pack_waybin(task_num, task, util_sum);
		if(res == 0)
		{
			printf("task is schedulable(way)\n");
			return 0;
		}
		else
		{
			printf("task is unschedulable!\n");
			return 1;
		}
	}

	return 0;

}
