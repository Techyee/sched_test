#include "sched_simul.h"

int test_TTC(int task_num, task_info** task,FILE* fp)
{
	int i;
	int res, res1, res2;
	int blocking_period;
	task_info** temp;
	float util_sum = 0.0;
	
	int channel_alloc_pass;
	int way_alloc_pass;

	//testing global allocation.
	printf("testing TTC allocation.\n");


	float rutil, rutil_sum;
	float wutil, wutil_sum;
	float gcutil, gcutil_sum;
	rutil_sum = 0.0;
	wutil_sum = 0.0;
	gcutil_sum = 0.0;

	//global allocation test code
	for(i=0;i<task_num;i++)
	{
		generate_overhead(task[i],16);
		util_sum += task[i]->task_util;
		task[i]->bin_alloc = 0;
		
		rutil = (float)(task[i]->read_num * READ_LTN) / (float)task[i]->read_period;
		wutil = (float)(task[i]->write_num * WRITE_LTN) / (float)task[i]->write_period;
		gcutil = (float)GC_EXEC / (float)task[i]->gc_period;
		rutil_sum += rutil;
		wutil_sum += wutil;
		gcutil_sum += gcutil;
		printf("rutil,wutil,gcutil = %f, %f, %f\n",rutil,wutil,gcutil);
		
	}
	blocking_period = find_least_in_bin(task_num,0,task);
	printf("blocking util : %f, b_period :%d, b_exec : %d\n",(float)ERASE_LTN / (float)blocking_period,ERASE_LTN,blocking_period);
	util_sum += (float)ERASE_LTN / (float)blocking_period;

	//global allocation is enough
	if(util_sum < 1.0)
	{
		printf("task is schedulable(global)\n");
		return 0;
	}
	//!global allocation test.

	if(util_sum < (double)CHANNEL_NB) //try channel-level BP.
	{
		res = pack_bin(task_num, task, util_sum,0);
		res1 = pack_3bin(task_num, task, util_sum,0);
		res2 = pack_4bin(task_num, task, util_sum,0);
		if(res == 0 || res1 == 0 || res2 == 0)
		{
			printf("task is schedulable(channel)\n");
			return 0;
		}	
		else
		{
			printf("task is unschedulable(channel)\n");
		}
	}

	if(util_sum < (double)CHANNEL_NB * (double)WAY_NB)//we can also try chip-level BP.
	{
		res = pack_waybin(task_num, task, util_sum);
		if(res == 0)
		{
			printf("task is schedulable(way)\n");
			return 1;
		}
		else
		{
			printf("task is unschedulable!\n");
			return 2;
		}
	}

	return 0;

}
