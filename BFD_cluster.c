#include "sched_simul.h"


int test_TTC_cluster(int task_num, task_info** task, int chip_num, int* set_num){
    
    int max_set = chip_num;
    alloc_set* temp;
    //sort the task according to the worst case bin.
    for(int i=0;i<task_num;i++){
        //reset bin_alloc
        task[i]->bin_alloc = -1;
        generate_overhead(task[i],1);
    }
    quick_sort(task,0,task_num-1);
    
    //make initial allocation set
    alloc_set** res = (alloc_set**)malloc(sizeof(alloc_set*)*chip_num);
    for(int i=0;i<chip_num;i++){
        res[i]=(alloc_set*)malloc(sizeof(alloc_set));
        init_set(res[i],1);
    }
    
    //pack each tasks,making a cluster if necessary.
    for(int i=0;i<task_num;i++){
        printf("====[TASK %d]====\n",i);
        print_taskinfo(task[i]);
        int cur_set = 0;
        int pass = 0;
        int merge_pass = 0;
        int maximum_util = 0.0;
        int target_set = -1;
        while(pass == 0){
            cur_set = 0;
            //search through current alloc_set, trying to find a bin.
            for(int j=0;j<max_set;j++){
                generate_overhead(task[i],res[j]->chip_num);
                allocate_task_to_set(res[j],task[i]);
                if(res[j]->total_task_util < 1.0){
                    if(maximum_util < res[j]->total_task_util){
                        maximum_util = res[j]->total_task_util;
                        target_set = j;
                    }
                }
                dealloc_task_to_set(res[j]);
            }
            //if allocation is failed(no possible bin), try merge
            if(target_set == -1){
                if(max_set == 1){
                    printf("cannot merge cluster anymore...\n");
                    return -1;
                }
                merge_set_new(res,max_set);
                
                max_set--;
                
            }
            else{//if alloc not failed, allocate task.
                generate_overhead(task[i],res[target_set]->chip_num);
                allocate_task_to_set(res[target_set],task[i]);
                print_set(res,max_set);
                pass = 1;
            }
        }    
    }
    
    int bin_check = 0;
    for(int i=0;i<max_set;i++){
        if(res[i]->total_task_util <= 1.0)
            bin_check++;
    }
    if(bin_check == max_set){//for tasks that succeed, do EDF simulator.
        *set_num = max_set;
        EDF_simulator(res,max_set);
        return 0;
    }
        
    else
        return -1;
}