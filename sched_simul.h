#include <stdio.h>
#include <stdlib.h>

typedef struct _task_info{
	int read_num;
	int write_num;
	int read_period;
	int write_period;
	float task_util;
}task_info;

int taskinfo_generation(task_info* res, double util); 
int taskset_generation(int task_num, double util);
