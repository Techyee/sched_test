#include "sched_simul.h"

int test_TTC_BFD(int task_num, task_info** task, FILE* fp, float* throughputs){
    int sched_res = 0;
	alloc_set** res;

	sched_res = n_chan_test_BFD(task_num,task,1,throughputs);
	if(sched_res == 0){
		return 0;
	}
	printf("!!!! 1 channel dense packing ended.!!!!\n");
	
	//n-channel dense packing.
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	sched_res = n_chan_test_BFD(task_num,task,2,throughputs);
	if(sched_res == 0){
		return 0;
	}
	printf("!!!! 2 channel dense packing ended.!!!!\n");
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	sched_res = n_chan_test_BFD(task_num,task,3,throughputs);
	if(sched_res == 0){
		return 0;
	}
	printf("!!!! 3 channel dense packing ended.!!!!\n");
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	sched_res = n_chan_test_BFD(task_num,task,4,throughputs);
	if(sched_res == 0){
		return 0;
    }
	printf("!!!! 4 channel dense packing ended.!!!!\n");
	printf("task is unschedulable(chip)\n");

	sched_res = glob_chan_test_BFD(task_num,task,throughputs);
	if(sched_res == 0){
		return 0;
	}
	//scheduling impossible. BE task also fails.
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	return -1;
}

int glob_chan_test_BFD(int task_num, task_info** task, float* throughputs){
	
	int config0[1] = {4};
	int config1[2] = {3,1};
	int config2[2] = {2,2};
	int config3[3] = {2,1,1};
	int config4[4] = {1,1,1,1};
	int sched_res = -1;
	alloc_set** temp; 

	temp = pack_bin_new_BFD(task_num,4,task,config4,0,&sched_res);
	if(sched_res == 0){
		calc_glob_throughput(temp,4,throughputs);
		free(temp);
		return 0;
	}
	else{
		free(temp);
	}

	temp = pack_bin_new_BFD(task_num,3,task,config3,0,&sched_res);
	if(sched_res == 0){
		calc_glob_throughput(temp,3,throughputs);
		free(temp);
		return 0;
	}
	else{
		free(temp);
	}

	temp = pack_bin_new_BFD(task_num,2,task,config2,0,&sched_res);
	if(sched_res == 0){
		calc_glob_throughput(temp,2,throughputs);
		free(temp);
		return 0;
	}
	else{
		free(temp);
	}

	temp = pack_bin_new_BFD(task_num,2,task,config1,0,&sched_res);
	if(sched_res == 0){
		calc_glob_throughput(temp,2,throughputs);
		free(temp);
		return 0;
	}
	else{
		free(temp);
	}
	
	temp = pack_bin_new_BFD(task_num,1,task,config0,0,&sched_res);
	if(sched_res == 0){
		calc_glob_throughput(temp,1,throughputs);
		free(temp);
		return 0;
	}
	else{
		free(temp);
	}

	printf("end of glob_chan test function. task is unschedulable!\n");
	return -1;
}
int chan_test_naive(int task_num, task_info** task, float* throughputs){
	//this function support only when max bin_num == 4
	int bin_num = 4;
	int bin_pass = 0;
	int* bin_config = malloc(sizeof(int)*task_num);
	int* task_config = malloc(sizeof(int)*task_num);
	for(int i=0;i<task_num;i++){
		task_config[i] = i % bin_num;
		bin_config[i] = 1;
	}
	alloc_set** naive_set = pack_naive(task_num,4,task,bin_config,task_config);
	for(int i=0;i<bin_num;i++){
		printf("set info : %d, ", naive_set[i]->chip_num);
		printf("%d, ",naive_set[i]->task_num);
		printf("%f\n",naive_set[i]->total_task_util);
	}
	for(int i=0;i<bin_num;i++){
		float total_util = 0.0;
		int blocking_period = calc_blocking_set(naive_set[i]);
		printf("original util : %f, bp : %d, new_util : %f\n",
		naive_set[i]->total_task_util,
		calc_blocking_set(naive_set[i]),
		naive_set[i]->total_task_util + (float)ERASE_LTN / (float)blocking_period);
		naive_set[i]->total_task_util += (float)ERASE_LTN / (float)blocking_period;
		if(naive_set[i]->total_task_util < 1.0)
			bin_pass++;
	}
	for(int i=0;i<bin_num;i++){
		printf("calc rt params %f, %d\n",naive_set[i]->total_task_util,naive_set[i]->chip_num);
		throughputs[0] += calc_RT_write(1,1.0 - naive_set[i]->total_task_util,naive_set[i]->chip_num);
		throughputs[1] += calc_RT_read(1,1.0 - naive_set[i]->total_task_util,naive_set[i]->chip_num);
	}

	printf("naive bandwidth: %f, %f\n",throughputs[0],throughputs[1]);
	if (bin_pass == bin_num)
		return 0;
	else 
		return -1;

}
int chan_test_global(int task_num, task_info** task, float* throughputs){
	int config[1] = {4};
	alloc_set** temp;
	int sched_res;
	temp = pack_bin_new_BFD(task_num,1,task,config,1,&sched_res);
	if(sched_res == 0){
        calc_chan_throughput(temp,1,throughputs);
		free(temp);
	}
	else{
		free(temp);
	}
}


int n_chan_test_BFD(int task_num, task_info** task, int channum,float* throughputs){
	throughputs[0] = 0.0; //reset throughput value first.
	throughputs[1] = 0.0;
	int way_task_num = 0;
	task_info** way_task = NULL;
	int config0[1] = {4};
	int config1[2] = {3,1};
	int config2[2] = {2,2};
	int config3[3] = {2,1,1};
	int config4[4] = {1,1,1,1};

	//divide task to each channel
	int safe_chan = 0;
	int sched_res = -1;
	int* chan_config = (int*)malloc(sizeof(int)*channum);
	int* chan_status = (int*)malloc(sizeof(int)*channum);
	for(int i=0;i<channum;i++){
		chan_config[i] = 1;
		chan_status[i] = -1; //init as invalid.
	}
	alloc_set** multi_chan = pack_bin_new(task_num,channum,task,chan_config,0,&sched_res);
	alloc_set** temp;
	//WFD packing.

	for(int i=0;i<channum;i++){
		way_task_num = multi_chan[i]->task_num;
		way_task = multi_chan[i]->task_info_ptrs;
		temp = pack_bin_new_BFD(way_task_num,4,way_task,config4,1,&sched_res);
		if(sched_res == 0){
			calc_chan_throughput(temp,4,throughputs);
            chan_status[i] = 1;
			free(temp);
			continue;
		}
		else{
			free(temp);
		}
		temp = pack_bin_new_BFD(way_task_num,3,way_task,config3,1,&sched_res);
		if(sched_res == 0){
            calc_chan_throughput(temp,3,throughputs);
			chan_status[i] = 1;
			free(temp);
			continue;
		}
		else{
			free(temp);
		}

		temp = pack_bin_new_BFD(way_task_num,2,way_task,config2,1,&sched_res);
		if(sched_res == 0){
            calc_chan_throughput(temp,2,throughputs);
			chan_status[i] = 1;
			free(temp);
			continue;
		}
		else{
			free(temp);
		}

		temp = pack_bin_new_BFD(way_task_num,2,way_task,config1,1,&sched_res);
		if(sched_res == 0){
            calc_chan_throughput(temp,2,throughputs);
			chan_status[i] = 1;
			free(temp);
			continue;
		}
		else{
			free(temp);
		}

		temp = pack_bin_new_BFD(way_task_num,1,way_task,config0,1,&sched_res);
		if(sched_res == 0){
            calc_chan_throughput(temp,1,throughputs);
			chan_status[i] = 1;
			free(temp);
			continue;
		}
		else{
			free(temp);
		}
	}//!end of RT-channel scheduling + calc.

	int free_chan = CHANNEL_NB - channum;
	for(int i=0;i<free_chan;i++)
		calc_chan_throughput(NULL,0,throughputs);
	//!end of free-channel bandwidth calc.

	for(int i=0;i<channum;i++){
		if(chan_status[i] == 1)
			safe_chan++;
	}
	if(safe_chan == channum)
		return 0;
	else
		return -1;
	//!end of check sched.
}
