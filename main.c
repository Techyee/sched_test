#include "sched_simul.h"

int page_per_block = 128;
int main(void)
{
	int i, j;
	task_info** test_task;
	task_info* test_task2;
	
	//taskset generation main code.
	srand(time(NULL));
	test_task = generate_taskset(10,(float)i * 0.05,16);
	
	/* 
	//wandgc generation main code.
	int chip = 16;
	FILE *fp;
	fp = fopen("wandgc.csv","w");

	for(j=1;j<17;j++)
	{
		fprintf(fp,"%d\n",j);
		for(i=1;i<20;i++)
		{
			test_task2 = generate_wandgc(0,(float)i * 0.05,j*page_per_block/16+1,j, fp);
			print_taskinfo(test_task2);
		}
	}
	fclose(fp);
	*/
	
	quick_sort(test_task,0,9);
	for(i=0;i<9;i++)
	{
		print_taskinfo(test_task[i]);
	}
	
	return 0;
}
