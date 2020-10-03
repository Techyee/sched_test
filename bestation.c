#include "sched_simul.h"

//These functions will be added at the test_TTC functions.
//calculate maximum throughput according to chip status.
//if chip is occupied by RT : calc with utilization.
//if chip is empty : calc the maximum thrghouput(saturation).

float calc_RT_write(int hyp, float util, int chip_num){
    /*hyp denotes hyperperiod(metric : second)
     */
    int hyp_us = hyp*1000*1000;
    int max_w = hyp_us / WRITE_LTN;
    int max_wpage = 0;
    task_info* w_profile = (task_info*)malloc(sizeof(task_info));
    //generate task_info profile
    for(int i=0;i<max_w;i++){
        w_profile->task_id = 0;
        w_profile->read_num = 0;
        w_profile->read_period = -1;
        w_profile->write_num = i;
        w_profile->write_period = hyp_us;
        generate_overhead(w_profile, chip_num);
        if(w_profile->task_util >= util){
            printf("util comparison : %f, %f\n",w_profile->task_util, util);
            break;
        }
        max_wpage = i;
    }
    printf("(w)bandwidth is %d kb/s\n",max_wpage * 8);
    free(w_profile);

    return (max_wpage * 8);
}       

float calc_RT_read(int hyp, float util, int chip_num){
    /*hyp denotes hyperperiod(metric : second)
     */
    int hyp_us = hyp*1000*1000;
    int max_r = hyp_us / READ_LTN;
    int max_rpage = 0;
    task_info* r_profile = (task_info*)malloc(sizeof(task_info));
    //generate task_info profile
    for(int i=0;i<max_r;i++){
        r_profile->task_id = 0;
        r_profile->write_num = 0;
        r_profile->write_period = -1;
        r_profile->read_num = i;
        r_profile->read_period = hyp_us;
        generate_overhead(r_profile, chip_num);
        if(r_profile->task_util >= util){
            break;
        }
        max_rpage = i;
    }
    printf("(r)bandwidth is %d kb/s\n", max_rpage * 8);
    free(r_profile);

    return (max_rpage * 8);
}       

float calc_empty_write(int num, int way){
    int page_num;
    int period;
    int pagesize = 8192;
    float throughput;
    if(way == 0){//n empty channels exist.
        //full access to chip will saturate each chip.
        page_num = OP_RATE * PAGE_PER_BLOCK;
        period = page_num * WRITE_LTN + GC_EXEC;
        throughput = (float)page_num * pagesize / (float)period;
        //Kilobyte per second, fully parallel op.
        throughput = throughput * 1000000.0 * (WAY_NB)*num / 1024.0;
    }
    else if(way == 1){//n empty chips exist.
        //since RT task exists on same channel, assume transfer overhead.
        page_num = OP_RATE * PAGE_PER_BLOCK;
        period = page_num * (WRITE_LTN+DATA_TRANS*(WAY_NB - num)) + GC_EXEC;
        
        throughput = (float)page_num * pagesize / (float)period;
        //Kilobyte per second, fully parallel op.
        throughput = throughput * 1000000.0 * num / 1024.0;
    }
 
    return throughput;
}

float calc_empty_read(int num, int way){
    int page_num;
    int period;
    int pagesize = 8192;
    float throughput;

    if(way == 0){//n empty channels exist.
        //full access to chip will saturate each chip.
        page_num = 4;
        period = DATA_TRANS * WAY_NB;
        throughput = (float)page_num * pagesize / (float)period;
        //Kilobyte per second, fully parallel op.
        throughput = throughput * 1000000 * num / 1024.0;
    }
    else if(way == 1){//n empty chips exist.
        //since RT task exists on same channel, assume transfer overhead.
        page_num = num;


        page_num = 1;
        period = DATA_TRANS * WAY_NB ;     

        throughput = (float)page_num * pagesize / (float)period;
        //Kilobyte per second, fully parallel op.
        throughput = throughput * 1000000 * num / 1024.0;
    }
    return throughput;
}
int check_max_throughput(alloc_set** set, int bin_num){
    //params
    int cur_chip_num;
    float util_sum = 0.0;
    int max_w, max_r;
    int total_w,total_r;
    total_w = 0;
    total_r = 0;
    task_info** cur_task_ptr;

    //for each alloc set, give rest of the util to BES.
    for(int i=0;i<bin_num;i++){
        cur_chip_num = set[i]->chip_num;
        cur_task_ptr = set[i]->task_info_ptrs;
        for(int j=0;j<set[i]->task_num;j++){
            //re-calculate overhead factor just in case.
            generate_overhead(cur_task_ptr[j],cur_chip_num);
            util_sum += cur_task_ptr[j]->task_util;
        }
        max_w = calc_RT_write(1,1.0 - util_sum,cur_chip_num);
        max_r = calc_RT_read(1,1.0 - util_sum,cur_chip_num);
        total_w += max_w;
        total_r += max_r;
    }
    printf("max write-only page : %d, max read-only page : %d\n",total_w,total_r);

}
