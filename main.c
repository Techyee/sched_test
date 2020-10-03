#include "sched_simul.h"

#define reverse
int page_per_block = 128;
int main(void)
{
	int i,j;
	int res, success_part, fail_part;
	int res_rev, success_rev, fail_rev;
	int res2, success_chan, success_way, fail_ttc;
	int res3, success_naive, fail_naive;

	int task_num;
	int* res_ptr;
	FILE *fp;
	FILE *fp2;
	FILE *fp2_rev;
	FILE *fp3;
	FILE *fp4;
	fp = fopen("PARTFTL_result.csv","w");
	fp2 = fopen("TTC_result.csv","w");
	fp2_rev = fopen("TTC_result_rev.csv","w");
	fp3 = fopen("naive_result.csv","w");
	fp4 = fopen("util_profile.csv","w");
	task_info** test_task;
	task_info* test_task2;
	alloc_set** test_allocset;
	
	task_num = 16;
	srand(time(NULL));
	//testing maximum bandwith in best-effort vs RT.
	float temp = calc_RT_write(1, 1.0, 1);
	float temp2 = calc_empty_write(1,1);
	printf("write calc : %f vs %f\n",temp,temp2);
	temp = calc_RT_read(1,1.0,1);
	temp2 = calc_empty_read(1,1);
	printf("read calc : %f vs %f\n",temp,temp2);

	//testing bandwidth calculation result.
	int temp3;

	float throughput[2] = {0.0,0.0};
	float throughput2[2] = {0.0,0.0};
	int details[4] = {0, };
	test_task = generate_taskset(task_num,4.0,16,1);
	temp3 = test_TTC_new(task_num,test_task,fp2,details,throughput);
	temp3 = test_TTC_reverse(task_num,test_task,fp2_rev,throughput2);
	printf("throughput : WONLY : %f, RONLY : %f\n",throughput[0],throughput[1]);
	printf("throughput : WONLY : %f, RONLY : %f\n",throughput2[0],throughput2[1]);

	/*
	//init
	success_chan = 0;
	success_way = 0;
	fail_ttc = 0;
	success_rev = 0;
	fail_rev = 0;
	srand(time(NULL));
	int* details = (int*)malloc(sizeof(int)*4);
	float* throughput = (float*)malloc(sizeof(float)*2);
	float* throughput_rev = (float*)malloc(sizeof(float)*2);
	int both = 0;
	int not4 = 0;
	int lost_sched = 0;
	//!init

	//testing a bandwidth calculator.
	test_task = generate_taskset(task_num,2.0,16,1);
	res = test_TTC_new(task_num,test_task,fp2,details,throughput);
	//res_rev = test_TTC_reverse(task_num,test_task,fp2_rev,throughput_rev);
	printf("(ori)throughputs => write-only : %f Kb/s, read-only : %f Kb/s\n",
	throughput[0],throughput[1]);
	//printf("(rev)throughputs => write-only : %f Kb/s, read-only : %f Kb/s\n",
	//throughput_rev[0],throughput_rev[1]);
	*/
	
	/*
	//testing original ttc vs reverse packing
	for(int i=0;i<4;i++)
		details[i] = 0;
	for(int i=0;i<40;i++){
		success_chan = 0;
		success_way = 0;
		fail_ttc = 0;
		success_rev = 0;
		fail_rev = 0;
		for(int j=0;j<100;j++){
			test_task = generate_taskset(task_num,0.1*i+0.1,16,1);
			res = test_TTC_new(task_num,test_task,fp2, details);

			//details check(4 bin fail, but 2~3bin pass)
			
			if(details[0] || details[1] || details[2]){
				if(details[3])
					both++;
				else
					not4++;
			}

			if (res == 0)
				success_chan += 1;
			else if (res == 1)
				success_way += 1;
			else if (res == -1)
				fail_ttc += 1;		
		}
		fprintf(fp2, " %d, %d, %d, %d\n",success_chan,success_way,fail_ttc, success_chan+success_way+fail_ttc);	
	}
	printf("only 2~3bin : %d, both of bin : %d\n",not4,both);
	free(details);
	*/
	/*		
	srand(time(NULL));
	for(i=0;i<40;i++){
		success_part = 0;
		success_chan = 0;
		success_way = 0;
		success_naive = 0;
		fail_part = 0;
		fail_ttc = 0;
		fail_naive = 0;
		for(j=0;j<200;j++){
			test_task = generate_taskset(task_num,0.1*i + 0.1,16,1);
			res = test_PARTFTL(task_num,test_task, fp);
			res2 = test_TTC(task_num,test_task,fp);
			res_ptr = test_naive(task_num,test_task);
			res3 = test_sched(task_num,test_task,res_ptr);
			destroy_taskinfo(task_num,test_task);
			if(res == 0){
				success_part += 1;
			}
			else{
				fail_part += 1;
			}

			if(res2 == 0){
				success_chan += 1;
			}
			else if(res2 == 1){
				success_way += 1;
			}
			else if(res2 == 2){
				fail_ttc += 1;
			}

			if(res3 == 0){
				success_naive += 1;
			}
			else if(res3 == 1){
				fail_naive += 1;
			}
			
		}
		fprintf(fp,"%f, %d, %d, %d\n",0.1*i + 0.1,success_part,fail_part,success_part+fail_part);
		fprintf(fp2,"%f, %d, %d, %d, %d\n",0.1*i + 0.1,success_chan,success_way,fail_ttc,success_chan+success_way+fail_ttc);
		fprintf(fp3,"%f, %d, %d, %d\n",0.1*i+ 0.1,success_naive,fail_naive,success_naive+fail_naive);
	}
	*/	
	fclose(fp);
	fclose(fp2);
	fclose(fp2_rev);
	fclose(fp3);
	fclose(fp4);
	return 0;
}
