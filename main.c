#include "sched_simul.h"

#define reverse

//#define single
#define unit_test

//#define util_change
//#define heavyset
//#define test

int page_per_block = 256;
int dual_check = 1;
int divided_check = 0;

int main(int argc, char** argv)
{
	//params : task num, util_limit, skewness, skew util 
	int i,j;
	int res, success_GLO, fail_GLO;
	int res2, success_BFD, fail_BFD;
	int res3, success_BFD2, fail_BFD2;
	int res4, success_naive, fail_naive;
	int res5, success_PART, fail_PART;
	int res6, success_clu, fail_clu;
	const char* _task_num = argv[1];
	const char* _util_limit = argv[2];
#ifdef test
	int task_num = atoi(_task_num);
	int util_limit = atoi(_util_limit);
	int skewness = atoi(argv[3]);
	double skew_util = atof(argv[4]);
#endif
	int* res_ptr;

	FILE *fp;
	FILE *fp2;
	FILE *fp3;
	FILE *fp4;
	FILE *fp5;
	FILE *fp6;
	FILE *fp7;
	FILE *fp8;
	FILE *fp9;
	FILE *fp10;

	fp = fopen("result_global.csv","w");
	fp2 = fopen("result_BFD.csv","w");
	fp3 = fopen("result_naive.csv","w");
	fp4 = fopen("result_PART.csv","w");

	fp5 = fopen("result_bdw_global.csv","w");
	fp6 = fopen("result_bdw_BFD.csv","w");
	fp7 = fopen("result_bdw_naive.csv","w");
	fp8 = fopen("result_bdw_PART.csv","w");

	fp9 = fopen("result_cluster.csv","w");
	//fp10 = fopen("result_bdw_cluster.csv","w");

	task_info** test_task;
	task_info** test_task2;
	alloc_set** test_allocset;
	int min_task = 2;
	int max_task = 16;
	float fixed_util = 3.0;

//	task_num = 4;
//	util_limit = 30;
#ifdef single
	int res_single;
	//example 1. RTIO needs good allocation.
	float throughputs[2];
	test_task = (task_info**)malloc(sizeof(task_info*)*4);
	test_task[0] = generate_taskinfo(0,0.3,0.1,40,12);
	test_task[1] = generate_taskinfo(1,0.3,0.1,40,12);
	test_task[2] = generate_taskinfo(2,0.1,0.3,40,12);
	test_task[3] = generate_taskinfo(3,0.0,0.7,0,6);
	for(int i=0;i<4;i++)
	generate_overhead(test_task[i],4);
	for(int i=0;i<4;i++)
		print_taskinfo(test_task[i]);
	printf("===BFD===\n");
	res_single = n_chan_test_BFD(3,test_task,1,throughputs);
	//printf("===GLOB===\n");
	//res_single = chan_test_global(3,test_task,throughputs);
	printf("===naive===\n");
	res_single = chan_test_naive(3,test_task,throughputs);
	//printf("===test1 end====\n");
	//example 2. BEIO also needs good allocation.
	/*
	test_task2 = (task_info**)malloc(sizeof(task_info*)*4);
	test_task2[0] = generate_taskinfo(0,0.0,0.15,0,12);
	test_task2[1] = generate_taskinfo(1,0.0,0.15,0,12);
	test_task2[2] = generate_taskinfo(2,0.0,0.15,0,12);
	test_task2[3] = generate_taskinfo(3,0.0,0.15,0,12);
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	res_single = n_chan_test_BFD(4,test_task2,1,throughputs);
	printf("[BFD_THR]%f, %f\n",throughputs[0],throughputs[1]);
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	res_single = chan_test_global(4,test_task2,throughputs);
	printf("[GLO_THR]%f, %f\n",throughputs[0],throughputs[1]);
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	res_single = chan_test_naive(4,test_task2,throughputs);
	printf("[NAI_THR]%f, %f\n",throughputs[0],throughputs[1]);
	*/
//analyzing overhead
for(i=1;i<5;i++){
	printf("=====[%d chip]=====\n",i);
	generate_overhead(test_task[0],i);
	printf("[0.4]overhead : write %f, gc %f sum %f\n",0.4/i,(float)GC_EXEC/(float)test_task[0]->gc_period, 0.4/i+(float)GC_EXEC/(float)test_task[0]->gc_period);
	generate_overhead(test_task[1],i);
	printf("[0.5]overhead : write %f, gc %f sum %f\n",0.5/i,(float)GC_EXEC/(float)test_task[1]->gc_period, 0.5/i+(float)GC_EXEC/(float)test_task[1]->gc_period);
	generate_overhead(test_task[2],i);
	printf("[0.6]overhead : write %f, gc %f sum %f\n",0.6/i,(float)GC_EXEC/(float)test_task[2]->gc_period, 0.6/i+(float)GC_EXEC/(float)test_task[2]->gc_period);
	generate_overhead(test_task[3],i);
	printf("[0.7]overhead : write %f, gc %f sum %f\n",0.7/i,(float)GC_EXEC/(float)test_task[3]->gc_period, 0.7/i+(float)GC_EXEC/(float)test_task[3]->gc_period);
}

#endif

#ifdef unit_test
	int task_num = 8;
	int set_num = 16;
	srand(time(NULL));
	float throughput[2] = {0.0, };
	int* details = (int*)malloc(sizeof(int)*4);
	test_task = generate_taskset(task_num,3.5,16,0.0,1);
	res = test_TTC_cluster(task_num,test_task,16,&set_num);
	printf("result : %d\n",res);
#endif

#ifdef test
	srand(time(NULL));
	float throughput[2] =  {0.0, };
	float throughput2[2] = {0.0, };
	float throughput3[2] = {0.0, };
	float throughput4[2] = {0.0, };
	double avg_GLO_throughput[2];
	double avg_BFD_throughput[2];
	double avg_naive_throughput[2];
	double avg_PART_throughput[2];
	//!init


	//testing algorithms using BFD and etc
#ifdef util_change
	for(int i=0;i<util_limit;i++){
#endif
#ifdef heavyset
	for(int i=0;i<task_num;i++){
#endif
		//init params
		for(j=0;j<2;j++){
			avg_GLO_throughput[j] = 0.0;
			avg_BFD_throughput[j] = 0.0;
			avg_naive_throughput[j] = 0.0;
			avg_PART_throughput[j] = 0.0;
		}
		int cnt[4] = {0, };

		success_GLO = 0;
		fail_GLO = 0;
		success_BFD = 0;
		fail_BFD = 0;
		success_BFD2 = 0;
		fail_BFD2 = 0;
		success_naive = 0;
		fail_naive = 0;
		success_PART = 0;
		fail_PART = 0;
		success_clu = 0;
		fail_clu = 0;

		//set sched percentage counter. 

		for(int j=0;j<100;j++){
			//gen task. specify skewness if necessary.
#ifdef util_change
			test_task = generate_taskset(task_num,0.1*i+0.1,16,0.0,1);
#endif
#ifdef heavyset
			test_task = generate_heavyset(task_num,i,0.5);
#endif
			//test schedulability.
			res = test_global(task_num,test_task,fp, throughput);
			res2 = test_TTC_BFD2(task_num,test_task,fp,throughput2);
			res3 = test_naive(task_num,test_task,throughput3);
			res4 = test_PARTFTL(task_num,test_task,fp, throughput4);
			res5 = test_TTC_cluster(task_num,test_task,16);
			if(dual_check == 1){
				if((res2 == 0) && (res3 == 0)){
					for(int k=0;k<2;k++){//for RW
						avg_GLO_throughput[k] = (avg_GLO_throughput[k]*cnt[0] + throughput[k]) / (cnt[0]+1);
						throughput[k] = 0.0;
						avg_BFD_throughput[k] = (avg_BFD_throughput[k]*cnt[0] + throughput2[k]) / (cnt[0]+1);				
						throughput2[k] = 0.0;
						avg_naive_throughput[k] = (avg_naive_throughput[k]*cnt[0] + throughput3[k]) / (cnt[0]+1);
						throughput3[k] = 0.0;
						avg_PART_throughput[k] = (avg_PART_throughput[k]*cnt[0] + throughput4[k]) / (cnt[0]+1);
						throughput4[k] = 0.0;
					}
					cnt[0]++;
				}
			}
			if(divided_check == 1){
				if(res == 0){
					for(int k=0;k<2;k++){//for RW
						avg_GLO_throughput[k] = (avg_GLO_throughput[k]*cnt[0] + throughput[k]) / (cnt[0]+1);
						throughput[k] = 0.0;
					}
					cnt[0]++;
				}
				if(res2 == 0){
					for(int k=0;k<2;k++){//for RW
						avg_BFD_throughput[k] = (avg_BFD_throughput[k]*cnt[1] + throughput2[k]) / (cnt[1]+1);				
						throughput2[k] = 0.0;
					}
					cnt[1]++;
				}
				if(res3 == 0){
					for(int k=0;k<2;k++){//for RW
						avg_naive_throughput[k] = (avg_naive_throughput[k]*cnt[2] + throughput3[k]) / (cnt[2]+1);				
						throughput3[k] = 0.0;
					}
					cnt[2]++;
				}
				if(res4 == 0){
					for(int k=0;k<2;k++){//for RW
						avg_PART_throughput[k] = (avg_PART_throughput[k]*cnt[3] + throughput4[k]) / (cnt[3]+1);				
						throughput4[k] = 0.0;
					}
					cnt[3]++;
				}
			}

			//record results to arr.
			if (res == 0)
				success_GLO++;
			else if (res == -1)
				fail_GLO++;		
			if (res2 == 0)
				success_BFD++;
			else if(res2 == -1)
				fail_BFD++;

			if (res3 == 0)
				success_naive++;
			else if(res3 == -1)
				fail_naive++;

			if (res4 == 0)
				success_PART++;
			else if (res4 == -1)
				fail_PART++;

			if (res5 == 0)
				success_clu++;
			else if (res5 == -1)
				fail_clu++;	
		}
		
		//record the scheduling results.
		fprintf(fp, "%d, %d, %d\n",success_GLO,fail_GLO, success_GLO+fail_GLO);	
		fprintf(fp2, "%d, %d, %d\n",success_BFD,fail_BFD,success_BFD+fail_BFD);
		fprintf(fp3, "%d, %d, %d\n",success_naive,fail_naive, success_naive+fail_naive);
		fprintf(fp4, "%d, %d, %d\n",success_PART,fail_PART,success_PART + fail_PART);
		fprintf(fp9, "%d, %d, %d\n",success_clu,fail_clu,success_clu+fail_clu);
		
		//record the bandwdith results.(MB/s)
		fprintf(fp5, "%lf, %lf\n",avg_GLO_throughput[0] / 1024.0 , avg_GLO_throughput[1] / 1024.0);
		fprintf(fp6, "%lf, %lf\n",avg_BFD_throughput[0] / 1024.0, avg_BFD_throughput[1] / 1024.0);
		fprintf(fp7, "%lf, %lf\n",avg_naive_throughput[0] / 1024.0, avg_naive_throughput[1] / 1024.0);
		fprintf(fp8, "%lf, %lf\n",avg_PART_throughput[0] / 1024.0, avg_PART_throughput[1] / 1024.0);
		//fprintf(fp7, "W : %f, %f, %f R : %f, %f, %f\n",stat_minimum[0],stat_maximum[0],stat_avg[0],stat_minimum[1],stat_maximum[1],stat_avg[1]);
	}
	
#endif
	fclose(fp);
	fclose(fp2);
	fclose(fp3);
	fclose(fp4);
	fclose(fp5);
	fclose(fp6);
	fclose(fp7);
	fclose(fp8);
	fclose(fp9);
	return 0;
}
