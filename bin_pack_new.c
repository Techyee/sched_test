#include "sched_simul.h"

int find_min(int* arr, int num){
    int min = -1;
    for(int i=0;i<num;i++){
        if(min == -1)
            min = arr[i];
        else if(min >= arr[i])
            min = arr[i];
    }
    return min;
}

int find_min_double_idx(double* arr, int num){
    double min = -1;
    int res_idx = -1;
    for(int i=0;i<num;i++){
        if(min == -1){
            min = arr[i];
            res_idx = i;
        }
        if(min <= arr[i]){
            min = arr[i];
            res_idx = i;
        }
    }
    if(res_idx == -1)
        abort();
    return res_idx;
}

void init_set(alloc_set* set,int chip_num){
    set->task_num = 0;
    set->chip_num = chip_num;
    for(int i=0;i<16;i++){
        set->task_ids[i] = -1;
        set->task_info_ptrs[i] = -1;
    }
    set->total_task_util = 0.0;
}

void insert_task_to_set(alloc_set* set,task_info* task){
    set->task_num += 1;
    set->task_ids[set->task_num-1] = task->task_id;
    set->task_info_ptrs[set->task_num-1] = task;
    set->total_task_util += task->task_util;
}

alloc_set** pack_bin_new(int task_num, int bin_num, task_info** task, int* config, int way, int* sched_res){
    int cur_bn = bin_num;
    double* bins = (double*)malloc(sizeof(double)*cur_bn);
    for(int i=0;i<cur_bn;i++)
        bins[i] = 0.0;
    double* expect = (double*)malloc(sizeof(double)*cur_bn);
    int cur;
    int blocking_period = 0;
    int min_bin = find_min(config,bin_num);
    alloc_set** res =(alloc_set**)malloc(sizeof(alloc_set*) * cur_bn);
    for(int i=0;i<cur_bn;i++){
        res[i] =(alloc_set*)malloc(sizeof(alloc_set));
        init_set(res[i],config[i]);
    }
    //sort the task according to the worst case bin.
    for(int i=0;i<task_num;i++){
        if(way == 0)
            generate_overhead(task[i],CHANNEL_NB*min_bin);
        else
            generate_overhead(task[i],min_bin);
    }
    quick_sort(task,0,task_num-1);

    //do the WFD packing
    
    for(int i=0;i<task_num;i++){
        task_info temp;

        //init
        for(int j=0;j<bin_num;j++)
            expect[j] = 0.0;
            
        //for each task, find suitable bin.    
        for(int j=0;j<bin_num;j++){
            //for each bin, calculate expect value.
            //in a case when bin size is diff, generate temp value. 
            
            temp = *task[i];
            if(config[j] != min_bin){
                if(way == 0)
                    generate_overhead(&temp,CHANNEL_NB*config[j]);
                else
                    generate_overhead(&temp,config[j]);

            expect[j] = 1.0 - bins[j] - temp.task_util;
            }
            else if(config[j] == min_bin){//minimum bin case
                expect[j] = 1.0 - bins[j] - task[i]->task_util;
            }
        }//found expectations for all bins.

        cur = find_min_double_idx(expect,cur_bn);
       
        if(config[cur] != min_bin){
            temp = *task[i];
            if(way == 0)
                    generate_overhead(&temp,CHANNEL_NB*config[cur]);
                else
                    generate_overhead(&temp,config[cur]);
            bins[cur] += temp.task_util;
            task[i]->task_util = temp.task_util;
            insert_task_to_set(res[cur],task[i]);
            task[i]->bin_alloc = cur;
        }
        else{
            bins[cur] += task[i]->task_util;
            insert_task_to_set(res[cur],task[i]);
            task[i]->bin_alloc = cur;
        }
    }//WFD bin packing finished.

    //add blocking factor
    for(int i=0;i<cur_bn;i++){
        blocking_period = find_least_in_bin(task_num,i,task);
        bins[i] += (float)ERASE_LTN / (float)blocking_period;
        res[i]->total_task_util += (float)ERASE_LTN / (float)blocking_period;
    }
    *sched_res = 0;
    printf("current bin status : ");
    for(int i=0;i<cur_bn;i++){
        printf("%f ",bins[i]);
        if(bins[i] >= 1.0)
            *sched_res = -1;
    }
    printf("\n");

    //free temp mems.
    free(bins);
    free(expect);
    return res;
}

alloc_set** free_allocset(alloc_set** target, int num){
    for(int i=0;i<num;i++)
        free(target[i]);
    return NULL;
}
