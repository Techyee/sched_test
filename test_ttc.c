#include "sched_test.h"

int test_TTC(int task_num, task_info** task)
{
	int i;
	int res;
	task_info** temp;
	float util_sum = 0.0;
	for(i=0;i<task_num;i++)
	{
		util_sum += temp[i]->task_util;
	}
	//global allocation is enough
	if(util_sum < 1.0)
	{
		printf("task is schedulable(global)\n");
		return 0;
	}

	else if(util_sum < (double)CHANNEL_NB )
	{
		res = pack_channelbin(int task_num, task_info** task);
		if(res == 1)
		{
			printf("task is schedulable(channel)\n");
			return 0;
		}
	}

	else//if channel-level BP fails, we can also try chip-level BP.
	{
		res = pack_chipbin(int task_num, task_info** task);
		if(res == 1)
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
