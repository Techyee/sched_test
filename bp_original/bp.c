#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int compare(const void *a, const void *b)
{
	int num1 = *(int*)a;
	int num2 = *(int*)b;
	
	if(num1 < num2)
		return -1;
	if(num1 > num2)
		return 1;

	return 0;
}


int main(void)
{
	int i,j, target_idx;
	int bins[4] = {0, 0, 0, 0};
	int idx[4] = {0,0,0,0};
	int numbers[10] = {0,};
	int b_items[4][10] = {{0,},{0,},{0,},{0,}};
	
	srand(time(NULL));
	for(i=0;i<10;i++)
	{
		numbers[i] = rand()%99 + 1;
	}
	
	qsort(numbers,sizeof(numbers)/sizeof(int),sizeof(int),compare);
	//now the numbers are sorted.
	//complexity = O(n*m);

	for(i=9;i>-1;i--){

		//first, find the target bin with the LEAST bulk.
		target_idx = 0;
		for(j=0;j<4;j++){
			if(bins[j] <= bins[target_idx]){
				target_idx = j;
			}
		}
		printf("target bin is %d\n",target_idx);
		//pack the target.
		bins[target_idx] += numbers[i];
		b_items[target_idx][idx[target_idx]] = numbers[i];
		idx[target_idx] += 1;
		
	}

	//print the informations.
	for(i=0;i<10;i++)
		printf("%d ",numbers[i]);
	printf("\n");
	for(j=0;j<4;j++)
		printf("%d ",bins[j]);
	printf("\n");
	for(i=0;i<4;i++){
		for(j=0;j<10;j++){
			printf("%d ",b_items[i][j]);
		}
		printf("\n");
	}
	//!print the informations.

	return 0;
}
		
	
	

