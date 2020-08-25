#include "sched_simul.h"

void swap(task_info* a, task_info* b)
{
	task_info temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

void quick_sort(task_info** task, int low, int high)
{
	printf("sorting %d to %d\n",low,high);
	int pivot,i,j;
	if(low < high)
	{
		pivot = low;
		i = low;
		j = high;
		while(i<j)
		{
			while((task[i]->task_util <= task[pivot]->task_util) && (i<high))
			{
				printf("i is %d\n",i);
				i++;
			}
			while((task[j]->task_util > task[pivot]->task_util) && (j>low))
			{
				printf("j is %d\n",j);
				j--;
			}
			if(i<j)
			{
				swap(task[i],task[j]);
			}
		}
		swap(task[j],task[pivot]);
		quick_sort(task,low,j-1);
		quick_sort(task,j+1,high);
	}
}
