#include "sched_simul.h"

int page_per_block = 128;
int main(void)
{
	int i,j;
	int res, success_part, fail_part;
	int res2, success_ttc, fail_ttc;
	int success_chan, success_way;
	int task_num;
	FILE *fp;
	FILE *fp2;
	fp = fopen("PARTFTL_result.csv","w");
	fp2 = fopen("TTC_result.csv","w");
	
	task_info** test_task;
	task_info* test_task2;

	srand(time(NULL));
	task_num = 3;
	
	test_task = generate_taskset(task_num,0.6,16);
	res = test_PARTFTL(task_num,test_task,fp2);
	
	/*
	//0.0~4.0 util generation.
	for(j=0;j<40;j++){
		success_chan = 0;
		success_way = 0;
		fail_ttc = 0;
		
		for(i=0;i<200;i++){
			test_task = generate_taskset(task_num,0.1*j+0.1, 16);
			res = test_TTC(task_num,test_task,fp2);
			if (res == 0){
				success_chan++;
			}
			else if(res == 1){
				success_way++;
			}
			else if(res == 2){
				fail_ttc++;
			}
			destroy_taskinfo(task_num,test_task);
		}	
		fprintf(fp2,"success_chan : %d, success_way : %d, fail : %d\n",success_chan, success_way, fail_ttc);
	}
	*/

	/*		
	srand(time(NULL));
	for(i=0;i<20;i++){
		success_part = 0;
		success_ttc = 0;
		fail_part = 0;
		fail_ttc = 0;
		for(j=0;j<200;j++){
			test_task = generate_taskset(10,0.1*i + 0.1,16);
			//res = test_PARTFTL(10,test_task, fp);
			res2 = test_TTC(10,test_task,fp);
			destroy_taskinfo(10,test_task);
			if(res == 0){
				success_part += 1;
			}
			else{
				fail_part += 1;
			}
			if(res2 == 0){
				success_ttc += 1;
			}
			else{
				fail_ttc += 1;
			}
		}
		//fprintf(fp,"%f, %d, %d, %d\n",0.1*i + 0.1,success_part,fail_part,success_part+fail_part);
		fprintf(fp2,"%f, %d, %d, %d\n",0.1*i + 0.1,success_ttc,fail_ttc,success_ttc + fail_ttc);
	}
	*/	
	fclose(fp);
	fclose(fp2);
	return 0;
}
