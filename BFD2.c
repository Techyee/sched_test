#include "sched_simul.h"
void calc_chip(int bin_num,float* throughputs,alloc_set** set, int way){
    for(int i=0;i<bin_num;i++){
        if(way == 1){
            if(set[i]->task_num == 0){
                throughputs[0] += calc_empty_write(set[i]->chip_num,1);
                throughputs[1] += calc_empty_read(set[i]->chip_num,1);
            }
            else{
                throughputs[0] += calc_RT_write(1,1.0-set[i]->total_task_util,set[i]->chip_num);
                throughputs[1] += calc_RT_read(1,1.0-set[i]->total_task_util,set[i]->chip_num);
            }
        }
        if(way == 0){
            if(set[i]->task_num == 0){
                throughputs[0] += calc_empty_write(set[i]->chip_num,0);
                throughputs[1] += calc_empty_read(set[i]->chip_num,0);
            }
            else{
                throughputs[0] += calc_RT_write(1,1.0-set[i]->total_task_util,set[i]->chip_num*WAY_NB);
                throughputs[1] += calc_RT_read(1,1.0-set[i]->total_task_util,set[i]->chip_num*WAY_NB);
            }
        }
    }
}

int test_global(int task_num, task_info** task, FILE* fp, float* throughputs){
    int config0_glob[1] = {4};
    int sched_res;
    alloc_set** res;
    res = pack_bin_new_BFD(task_num, 1, task, config0_glob,0, &sched_res);
    if(sched_res == 0){
        calc_chip(1,throughputs,res,0);
        printf("task is sched(glob)\n");
        free(res);
        return 0;
    }
    else
        return -1;

}
int test_TTC_BFD2(int task_num, task_info** task, FILE* fp, float* throughputs){
    int config_num = 8;
    int config_count = 0;
    int config0_chip[16] = {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1};
    int config1_chip[8] = {2,2,2,2, 2,2,2,2};
    int config2_chip[12] = {2,2,2,2, 1,1,1,1, 1,1,1,1};

    int config0_chan[2] = {3,1};
    int config1_chan[2] = {2,2};
    int config2_chan[3] = {2,1,1};
    int config3_chan[4] = {1,1,1,1};

    int config0_glob[1] = {4};
    
    task_info** way_task = NULL;
    alloc_set** res;
    int sched_res = -1;
    int succ_count = 0;
    //level 1. chip-level allocation
    float** throughput_ptr = (float**)malloc(sizeof(float*)*config_num);
    for(int i=0;i<config_num;i++){
        throughput_ptr[i] = (float*)malloc(sizeof(float)*2);
        throughput_ptr[i][0] = 0.0;
        throughput_ptr[i][1] = 0.0;
    }

    res = pack_bin_new_BFD(task_num, 16, task, config0_chip,1,&sched_res);
    
    if(sched_res == 0){
        calc_chip(16,throughput_ptr[config_count],res,1);
        printf("task is sched(16C)\n");
        free(res); succ_count++;
        throughputs[0] = throughput_ptr[config_count][0];
        throughputs[1] = throughput_ptr[config_count][1];
        return 0;
    }
    config_count++;

    res = pack_bin_new_BFD(task_num, 12, task, config2_chip, 1, &sched_res);
    
    if(sched_res == 0){
        calc_chip(12,throughput_ptr[config_count],res,1);
        printf("task is sched(12C)\n");
        free(res); succ_count++;
        throughputs[0] = throughput_ptr[config_count][0];
        throughputs[1] = throughput_ptr[config_count][1];
        return 0;
    }
    config_count++;

    sched_res = -1;
    res = pack_bin_new_BFD(task_num, 8, task, config1_chip, 1, &sched_res);
   
    if(sched_res == 0){
        calc_chip(8,throughput_ptr[config_count],res,1);
        printf("task is sched(8C)\n");
        free(res); succ_count++;
        throughputs[0] = throughput_ptr[config_count][0];
        throughputs[1] = throughput_ptr[config_count][1];
        return 0;
    }
    config_count++;

    
    //level 2. channel-level allocation
    sched_res = -1;
    res = pack_bin_new_BFD(task_num, 2, task, config0_chan,0,&sched_res);
 
    if(sched_res == 0){
        calc_chip(2,throughput_ptr[config_count],res,0);
        printf("task is sched(3,1,Chan)\n");
        free(res); succ_count++;
        throughputs[0] = throughput_ptr[config_count][0];
        throughputs[1] = throughput_ptr[config_count][1];
        return 0;
    }
    config_count++;

    sched_res = -1;
    res = pack_bin_new_BFD(task_num, 2, task, config1_chan,0, &sched_res);
 
    if(sched_res == 0){
        calc_chip(2,throughput_ptr[config_count],res,0);
        printf("task is sched(2,2,Chan)\n");
        free(res); succ_count++;
        throughputs[0] = throughput_ptr[config_count][0];
        throughputs[1] = throughput_ptr[config_count][1];
        return 0;
    }
    config_count++;
    
    sched_res = -1;
    res = pack_bin_new_BFD(task_num, 3, task ,config2_chan,0, &sched_res);
    if(sched_res == 0){
        calc_chip(3,throughput_ptr[config_count],res,0);
        printf("task is sched(2,1,1 chan)\n");
        free(res); succ_count++;
        throughputs[0] = throughput_ptr[config_count][0];
        throughputs[1] = throughput_ptr[config_count][1];
        return 0;
    }
    config_count++;

    
    sched_res = -1;
    res = pack_bin_new_BFD(task_num, 4, task ,config3_chan,0, &sched_res);
    if(sched_res == 0){
        calc_chip(4,throughput_ptr[config_count],res,0);
        printf("task is sched(1,1,1,1 chan)\n");
        free(res); succ_count++;
        throughputs[0] = throughput_ptr[config_count][0];
        throughputs[1] = throughput_ptr[config_count][1];
        return 0;
    }
    config_count++;
    //level 3. global-level allocation
    sched_res = -1;
    res = pack_bin_new_BFD(task_num, 1, task, config0_glob,0, &sched_res);
    if(sched_res == 0){
        calc_chip(1,throughput_ptr[config_count],res,0);
        printf("task is sched(glob)\n");
        free(res); succ_count++;
        throughputs[0] = throughput_ptr[config_count][0];
        throughputs[1] = throughput_ptr[config_count][1];
        return 0;
    }
    config_count++;

    if(succ_count > 0)
        return 0;
    else{
        printf("not sched\n");
        return -1;
    }
}

