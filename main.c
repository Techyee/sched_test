#include "sched_simul.h"

int main(void)
{
	task_info** test_task;
	//test_task = generate_taskinfo(test_task,1,0.05,0.4,16);
	test_task = generate_taskset(3,0.9);
	return 0;
}
