#include "sched_simul.h"

int main(void)
{
	int i, j;
	task_info** test_task;
	//test_task = generate_taskinfo(test_task,1,0.05,0.4,16);
	//test_task = generate_taskset(1,0.5,16);
	//test_task = generate_taskset(1,0.5,4);
	//test_task = generate_taskset(1,0.5,1);
	srand(time(NULL));
	for(j=0;j<3;j++){
		for(i=1;i<20;i++){
			test_task = generate_taskset(1,(float)i * 0.05,16);
		}
	}
	
	return 0;
}
