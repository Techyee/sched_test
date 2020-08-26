#include "sched_simul.h"

int page_per_block = 128;
int main(void)
{
	int i, j;
	task_info** test_task;
	task_info* test_task2;
	
	//taskset generation main code.
	srand(time(NULL));
	test_task = generate_taskset(5,1.25,16);
	test_TTC(5,test_task);
	return 0;
}
