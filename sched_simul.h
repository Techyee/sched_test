#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


typedef struct _task_info{
	int task_id;
	int read_num;
	int write_num;
	int read_period;
	int write_period;
	int gc_period;
	float task_util;
}task_info;

typedef struct _chipset{
	int idx;
	int chip_num;
	double utilization;
}chipset;

task_info* generate_taskinfo(int tid, double util1, double util2, int rnum, int wnum);
int generate_gcinfo(task_info* task, int chip);
task_info* generate_wandgc(int tid, double util, int wnum, int chip, FILE* fp);
task_info** generate_taskset(int task_num, double util,int chip);
int destroy_taskinfo(int task_num, task_info** task);
int print_taskinfo(task_info* task);

//schedulability test function
int test_PARTFTL(int task_num, task_info** task);
int test_naive(task_info* task);
int test_UPI(task_info* task);
int test_TTC(task_info* task);

//bin_packing function
int pack_channelbin(int task_num, task_info** task);
int pack_waybin(int task_num, task_info** task);

