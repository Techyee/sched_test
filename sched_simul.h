#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

//paramters. based on Utilitarion Performance Isolation(UPI) 
#define PAGE_PER_BLOCK 128
#define OP_RATE 0.50
#define CHANNEL_NB  4 //bin-packing only support 4 chan now.
#define WAY_NB 4 //bin-packing only support 4 way now.
#define READ_LTN 90
#define WRITE_LTN 540
#define ERASE_LTN 5000
#define DATA_TRANS 40
#define GC_EXEC (550*128*0.50 + 5000)
//!parameters

typedef struct _task_info{
	int task_id;
	int bin_alloc;
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

//taskset generation code
task_info* generate_taskinfo(int tid, double util1, double util2, int rnum, int wnum);
task_info* generate_ampcheck(int tid, double util, int num, int chip, int rw, FILE* fp);
task_info** generate_taskset(int task_num, double util, int chip,int long_p);
int generate_overhead(task_info* task, int chip);
int generate_dt(task_info* task, int chip);
int destroy_taskinfo(int task_num, task_info** task);
int print_taskinfo(task_info* task);

//schedulability test function
int myceil(float input);
int test_PARTFTL(int task_num, task_info** task, FILE* fp);
int test_TTC(int task_num, task_info** task, FILE* fp);
int test_sched(int task_num, task_info** task, int* ttc_alloc);
int* test_naive(int task_num, task_info** task);
int* test_UPI(task_info* task);

//bin_packing function
int find_least_in_bin(int task_num, int target_bin, task_info** task);
int pack_bin(int task_num, task_info** task,double util_sum,int way);
int pack_3bin(int task_num, task_info** task, double util_sum,int way);
int pack_4bin(int task_num, task_info** task, double util_sum, int way);
int pack_waybin(int task_num, task_info** task,double util_sum);

//utils
void swap(task_info* a, task_info* b);
void quick_sort(task_info** task, int low, int high);
