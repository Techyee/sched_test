#include "sched_simul.h"
int check_sched(alloc_set** targ, int num, int sched_res){
	if(sched_res == 0){
		
		return 1;
	}
	else
		return 0;
}

int test_TTC_new(int task_num, task_info** task, FILE* fp, int* detail, float* throughputs){
	//possible allocation param.
	int config0[1] = {4};
	int config1[2] = {3,1};
	int config2[2] = {2,2};
	int config3[3] = {2,1,1};
	int config4[4] = {1,1,1,1};

	int way_task_num = 0;
	task_info** way_task = NULL;
	alloc_set** res;
	int sched_res = 0;
	int way_sched_res[4] = {0, };
	float BE_write_throughput[4];
	float BE_read_throughput[4];
	for(int i=0;i<4;i++)
		detail[i] = 0;

	//global-packing sched_test
	res = pack_bin_new(task_num, 1, task, config0, 0, &sched_res);
	if(sched_res == 0){
		printf("task is schedulable(global)\n");
		for(int i=0;i<1;i++){
			BE_write_throughput[0] = calc_RT_write(1,1.0 - res[i]->total_task_util,16);
			BE_read_throughput[0] = calc_RT_read(1,1.0 - res[i]->total_task_util,16);
		}
		throughputs[0] = BE_write_throughput[0];
		throughputs[1] = BE_read_throughput[0];
		free(res);
		return 0;
	}
	//channel-packing sched_test.
	int cur_thr_idx = 0;
	int channel_pass = -1;
	if(sched_res == -1){

		BE_read_throughput[cur_thr_idx] = 0;
		BE_write_throughput[cur_thr_idx] = 0;
		res = pack_bin_new(task_num, 2, task, config1, 0, &sched_res);
		if(check_sched(res, 2, sched_res)){
			detail[0] = 1;
			channel_pass = 1;
			for(int i=0;i<2;i++){
				BE_write_throughput[cur_thr_idx] += calc_RT_write(1,1.0 - res[i]->total_task_util,config1[i]*WAY_NB);
				BE_read_throughput[cur_thr_idx] += calc_RT_read(1,1.0 - res[i]->total_task_util,config1[i]*WAY_NB);
			}
			throughputs[0] = BE_write_throughput[cur_thr_idx];
			throughputs[1] = BE_read_throughput[cur_thr_idx];
			return 0;
		}
		free(res);
		
		BE_read_throughput[cur_thr_idx] = 0;
		BE_write_throughput[cur_thr_idx] = 0;
		res = pack_bin_new(task_num, 2, task, config2, 0, &sched_res);
		if(check_sched(res, 2, sched_res)){
			detail[1] = 1;
			channel_pass = 1;
			for(int i=0;i<2;i++){
				BE_write_throughput[cur_thr_idx] += calc_RT_write(1,1.0 - res[i]->total_task_util,config2[i]*WAY_NB);
				BE_read_throughput[cur_thr_idx] += calc_RT_read(1,1.0 - res[i]->total_task_util,config2[i]*WAY_NB);	
			}
			throughputs[0] = BE_write_throughput[cur_thr_idx];
			throughputs[1] = BE_read_throughput[cur_thr_idx];
			return 0;
		}
		free(res);
		cur_thr_idx++;
		BE_read_throughput[cur_thr_idx] = 0;
		BE_write_throughput[cur_thr_idx] = 0;
		res = pack_bin_new(task_num, 3, task, config3, 0, &sched_res);
		if(check_sched(res, 3, sched_res)){
			detail[2] = 1;
			channel_pass = 1;
			for(int i=0;i<3;i++){
				BE_write_throughput[cur_thr_idx] += calc_RT_write(1,1.0 - res[i]->total_task_util,config3[i]*WAY_NB);
				BE_read_throughput[cur_thr_idx] += calc_RT_read(1,1.0 - res[i]->total_task_util,config3[i]*WAY_NB);	
			}
			throughputs[0] = BE_write_throughput[cur_thr_idx];
			throughputs[1] = BE_read_throughput[cur_thr_idx];
			return 0;
		}
		free(res);
		cur_thr_idx++;
		BE_read_throughput[cur_thr_idx] = 0;
		BE_write_throughput[cur_thr_idx] = 0;
		res = pack_bin_new(task_num, 4, task, config4, 0, &sched_res);
		if(check_sched(res, 4, sched_res)){
			detail[3] = 1;
			channel_pass = 1;
			for(int i=0;i<4;i++){
				BE_write_throughput[cur_thr_idx] += calc_RT_write(1,1.0 - res[i]->total_task_util,config4[i]*WAY_NB);
				BE_read_throughput[cur_thr_idx] += calc_RT_read(1,1.0 - res[i]->total_task_util,config4[i]*WAY_NB);
			}
			throughputs[0] = BE_write_throughput[cur_thr_idx];
			throughputs[1] = BE_read_throughput[cur_thr_idx];
			return 0;
		}
		cur_thr_idx++;
	}
	
	//way-packing sched test.
	alloc_set** way_res;
	//reset throughput.
	//collect throughput from each channel.
	for(int i=0;i<CHANNEL_NB;i++){
		BE_read_throughput[i] = 0;
		BE_write_throughput[i] = 0;
	}
	if(channel_pass == -1 && sched_res == -1){
		for(int i=0;i<CHANNEL_NB;i++){
			way_task_num = res[i]->task_num;
			way_task = res[i]->task_info_ptrs;
			//packing starts from largest bin case.
			way_res = pack_bin_new(way_task_num, 1, way_task, config0, 1, &sched_res);
			if(check_sched(way_res, 1, sched_res)){
				way_sched_res[i] = 0;
				//add channel throughput by adding be-throuput of each allocset
				for(int j=0;j<1;j++){
					BE_write_throughput[i] += calc_RT_write(1,1.0 - way_res[j]->total_task_util,config0[j]);
					BE_read_throughput[i] += calc_RT_read(1,1.0 - way_res[j]->total_task_util,config0[j]);
				}
				continue;
			}
			free(way_res);

			way_res = pack_bin_new(way_task_num, 2, way_task, config1, 1, &sched_res);
			if(check_sched(way_res, 2, sched_res)){
				way_sched_res[i] = 0;
				for(int j=0;j<2;j++){
					BE_write_throughput[i] += calc_RT_write(1,1.0 - way_res[j]->total_task_util,config1[j]);
					BE_read_throughput[i] += calc_RT_read(1,1.0 - way_res[j]->total_task_util,config1[j]);
				}
				continue;
			}
			free(way_res);

			way_res = pack_bin_new(way_task_num, 2, way_task, config2, 1, &sched_res);
			if(check_sched(way_res, 2, sched_res)){
				way_sched_res[i] = 0;
				for(int j=0;j<2;j++){
					BE_write_throughput[i] += calc_RT_write(1,1.0 - way_res[j]->total_task_util,config2[j]);
					BE_read_throughput[i] += calc_RT_read(1,1.0 - way_res[j]->total_task_util,config2[j]);
				}
				continue;
			}
			free(way_res);

			way_res = pack_bin_new(way_task_num, 3, way_task, config3, 1, &sched_res);
			if(check_sched(way_res, 3, sched_res)){
				way_sched_res[i] = 0;
				for(int j=0;j<3;j++){
					BE_write_throughput[i] += calc_RT_write(1,1.0 - way_res[j]->total_task_util,config3[j]);
					BE_read_throughput[i] += calc_RT_read(1,1.0 - way_res[j]->total_task_util,config3[j]);
				}
				continue;
			}
			free(way_res);

			way_res = pack_bin_new(way_task_num, 4, way_task, config4, 1, &sched_res);
			if(check_sched(way_res, 4, sched_res)){
				way_sched_res[i] = 0;
				for(int j=0;j<4;j++){
					BE_write_throughput[i] += calc_RT_write(1,1.0 - way_res[j]->total_task_util,config4[j]);
					BE_read_throughput[i] += calc_RT_read(1,1.0 - way_res[j]->total_task_util,config4[j]);
				}
			}
			else{
				way_sched_res[i] = -1;
			}
			free(way_res);
		}
	}

	for(int i=0;i<CHANNEL_NB;i++){
		if(way_sched_res[i] == -1){
			printf("task is unschedulable.\n");
			throughputs[0] = 0.0;
			throughputs[1] = 0.0;
			return -1;
		}
			
	}
	printf("task is schedulable(way)\n");
	for(int i=0;i<CHANNEL_NB;i++){
		throughputs[0] += BE_write_throughput[i];
		throughputs[1] += BE_read_throughput[i];
	}
	return 1;

}

int test_TTC_reverse(int task_num, task_info** task, FILE*fp, float* throughputs){
	int sched_res = 0;
	alloc_set** res;

	sched_res = n_chan_test_TTC(task_num,task,1,throughputs);
	if(sched_res == 0){
		return 0;
	}
	printf("!!!! 1 channel dense packing ended.!!!!\n");
	
	//n-channel dense packing.
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	sched_res = n_chan_test_TTC(task_num,task,2,throughputs);
	if(sched_res == 0){
		return 0;
	}
	printf("!!!! 2 channel dense packing ended.!!!!\n");
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	sched_res = n_chan_test_TTC(task_num,task,3,throughputs);
	if(sched_res == 0){
		return 0;
	}
	printf("!!!! 3 channel dense packing ended.!!!!\n");
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	sched_res = n_chan_test_TTC(task_num,task,4,throughputs);
	if(sched_res == 0){
		return 0;}
	printf("!!!! 4 channel dense packing ended.!!!!\n");
	printf("end of function. task is unschedulable!\n");
	//scheduling impossible. BE task also fails.
	throughputs[0] = 0.0;
	throughputs[1] = 0.0;
	return -1;
}


int n_chan_test_TTC(int task_num, task_info** task, int channum,float* throughputs){
	
	int way_task_num = 0;
	task_info** way_task = NULL;
	int config0[1] = {1};
	int config1[2] = {1,1};
	int config2[2] = {2,1};
	int config3[3] = {1,1,1};
	int config4[2] = {3,1};
	int config5[3] = {2,1,1};
	int config6[4] = {1,1,1,1};
	int config7[1] = {4};
	int config8[2] = {2,2};
	//divide task to each channel
	int safe_chan = 0;
	int sched_res;
	int* chan_config = (int*)malloc(sizeof(int)*channum);
	int* chan_status = (int*)malloc(sizeof(int)*channum);
	for(int i=0;i<channum;i++){
		chan_config[i] = 1;
		chan_status[i] = -1; //init as invalid.
	}
	alloc_set** multi_chan = pack_bin_new(task_num,channum,task,chan_config,0,&sched_res);
	alloc_set** temp;
	//for each allocation set, test packing.

	for(int i=0;i<channum;i++){
		way_task_num = multi_chan[i]->task_num;
		way_task = multi_chan[i]->task_info_ptrs;
		temp = pack_bin_new(way_task_num,1,way_task,config0,1,&sched_res);
		if(check_sched(temp,1,sched_res)){
			chan_status[i] = 1; //validate channel.
			calc_chan_throughput(temp,1,throughputs);
			continue;
		}
		free(temp);

		temp = pack_bin_new(way_task_num,2,way_task,config1,1,&sched_res);
		if(check_sched(temp,2,sched_res)){
			chan_status[i] = 1;
			calc_chan_throughput(temp,2,throughputs);
			continue;
		}
		free(temp);

		temp = pack_bin_new(way_task_num,2,way_task,config2,1,&sched_res);
		if(check_sched(temp,2,sched_res)){
			chan_status[i] = 1;
			calc_chan_throughput(temp,2,throughputs);
			continue;
		}
		free(temp);

		temp = pack_bin_new(way_task_num,3,way_task,config3,1,&sched_res);

		if(check_sched(temp,3,sched_res)){
			chan_status[i] = 1;
			calc_chan_throughput(temp,3,throughputs);
			continue;
		}
		free(temp);

		temp = pack_bin_new(way_task_num,2,way_task,config4,1,&sched_res);
		if(check_sched(temp,2,sched_res)){
			chan_status[i] = 1;
			calc_chan_throughput(temp,2,throughputs);
			continue;
		}
		free(temp);

		temp = pack_bin_new(way_task_num,3,way_task,config5,1,&sched_res);
		if(check_sched(temp,3,sched_res)){
			chan_status[i] = 1;
			calc_chan_throughput(temp,3,throughputs);
			continue;
		}
		free(temp);

		temp = pack_bin_new(way_task_num,4,way_task,config6,1,&sched_res);
		if(check_sched(temp,4,sched_res)){
			chan_status[i] = 1;
			calc_chan_throughput(temp,4,throughputs);
			continue;
		}
		free(temp);
		temp = pack_bin_new(way_task_num,1,way_task,config7,1,&sched_res);
		if(check_sched(temp,1,sched_res)){
			chan_status[i] = 1;
			calc_chan_throughput(temp,1,throughputs);
			continue;
		}
		free(temp);
		temp = pack_bin_new(way_task_num,2,way_task,config8,1,&sched_res);
		if(check_sched(temp,2,sched_res)){
			chan_status[i] = 1;
			calc_chan_throughput(temp,2,throughputs);
			continue;
		}
		free(temp);
	}//!end of RT-channel scheduling.

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


