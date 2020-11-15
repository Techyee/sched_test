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

int find_max_double_idx(double* arr, int num){
    double max = -1;
    int res_idx = -1;
    for(int i=0;i<num;i++){
        if(max == -1){
            max = arr[i];
            res_idx = i;
        }
        if(max <= arr[i]){
            max = arr[i];
            res_idx = i;
        }
    }
    if(res_idx == -1)
        abort();
    return res_idx;
}

int find_min_double_idx(double* arr, int num){
    double min = -1;
    int res_idx = -1;
    for (int i=0;i<num;i++){
        if((min == -1) && (arr[i] >= 0.0)){
            min = arr[i];
            res_idx = i;
        }
        if((min >= arr[i]) && (arr[i] >= 0.0)){
            min = arr[i];
            res_idx = i;
        }
    }
    return res_idx;
}
void init_set(alloc_set* set,int chip_num){
    set->task_num = 0;
    set->chip_num = chip_num;
    for(int i=0;i<32;i++){
        set->task_ids[i] = -1;
        set->task_info_ptrs[i] = NULL;
    }
    set->total_task_util = 0.0;
}

int find_least_in_alloc(int task_num, alloc_set* set){
    int res,i;
    res = -1;
    i = 0;
    for(i=0;i<task_num;i++){
        task_info* tptr = set->task_info_ptrs[i];
		int temp;
		//find least period.
		if(tptr->read_period < tptr->write_period &&(tptr->read_period >= 0))
			temp = tptr->read_period;
		else
			temp = tptr->write_period;
		
		if(temp > tptr->gc_period &&(tptr->gc_period >= 0))
			temp = tptr->gc_period;
		else{ /* do nothing */ }
		//found.
		if(res == -1){//init case
			res = temp;
		}
		else{//non-init case : compare
			if (res > temp)
				res = temp;
		}
    }
	return res;	
}

void allocate_task_to_set(alloc_set* set,task_info* task){
    float temp_sum = 0.0;
    int bp = 0;
    set->task_num += 1;
    set->task_ids[set->task_num-1] = task->task_id;
    set->task_info_ptrs[set->task_num-1] = task;
    for(int i=0;i<set->task_num;i++){
        temp_sum += set->task_info_ptrs[i]->task_util;
    }
    bp = find_least_in_alloc(set->task_num,set);
    temp_sum += (float)ERASE_LTN / (float)bp;
    set->total_task_util = temp_sum;
}

void dealloc_task_to_set(alloc_set* set){//omit last task.
    float temp_sum = 0.0;
    int bp = 0;
    set->task_num -= 1;
    set->task_ids[set->task_num] = -1;
    set->task_info_ptrs[set->task_num] = NULL;
    for(int i=0;i<set->task_num;i++){
        temp_sum += set->task_info_ptrs[i]->task_util;
    }
    bp = find_least_in_alloc(set->task_num,set);
    if(bp != -1)
        temp_sum += (float)ERASE_LTN / (float)bp;
    set->total_task_util = temp_sum;
}

void merge_set_new(alloc_set** set, int max_num){
    //try every possible merge set
    int target_A = 0;
    int target_B = 1;
    float cur_lowest_sum = -1.0;
    alloc_set* new_allocset_ptr;
    for(int i=0;i<max_num;i++){
        for(int j=i+1;j<max_num;j++){
            int cur_idx = 0;
            float temp_sum = 0.0;
            alloc_set* temp_merged = (alloc_set*)malloc(sizeof(alloc_set));
            temp_merged->chip_num = set[i]->chip_num + set[j]->chip_num;
            temp_merged->task_num = set[i]->task_num + set[j]->task_num;

            for(int k=0;k<set[i]->task_num;k++){
                temp_merged->task_ids[cur_idx] = set[i]->task_ids[k];
                temp_merged->task_info_ptrs[cur_idx] = set[i]->task_info_ptrs[k];
                cur_idx++;
            }
            for(int k=0;k<set[j]->task_num;k++){
                temp_merged->task_ids[cur_idx] = set[j]->task_ids[k];
                temp_merged->task_info_ptrs[cur_idx] = set[j]->task_info_ptrs[k];
                cur_idx++;
            }
            for(int k=0;k<temp_merged->task_num;k++){
                generate_overhead(temp_merged->task_info_ptrs[k],temp_merged->chip_num);
                temp_sum += temp_merged->task_info_ptrs[k]->task_util;
            }
            int bp = find_least_in_alloc(temp_merged->task_num,temp_merged);
            temp_sum += (float)ERASE_LTN / (float)bp;
            if(cur_lowest_sum == -1.0){//init
                cur_lowest_sum = temp_sum;
                target_A = i;
                target_B = j;
                new_allocset_ptr = temp_merged;
            }
            else if(cur_lowest_sum > temp_sum){//update
                cur_lowest_sum = temp_sum;
                target_A = i;
                target_B = j;
                free(new_allocset_ptr);
                new_allocset_ptr = temp_merged;
            }
            else{//else...
                free(temp_merged);

            }
        }
    }//found merge target A and B.
    printf("merge target is %d and %d, util %f, chip_num %d\n",target_A, target_B,new_allocset_ptr->total_task_util,new_allocset_ptr->chip_num);
    //align the set.
    free(set[target_A]);
    free(set[target_B]);
    set[target_A]=new_allocset_ptr;
    for(int i=target_B;i<max_num-1;i++){
        set[i] = set[i+1];
    }
    //reset the overhead factor correctly
    for(int i=0;i<max_num-1;i++){
        float sum = 0.0;
        for(int j=0;j<set[i]->task_num;j++){
            generate_overhead(set[i]->task_info_ptrs[j],set[i]->chip_num);
            sum += set[i]->task_info_ptrs[j]->task_util;
        }
        int bp = find_least_in_alloc(set[i]->task_num,set[i]);
        sum += (float)ERASE_LTN / (float)bp;
        set[i]->total_task_util = sum;
    }
}
void merge_set(alloc_set** set, int max_num){
    //find the least utilization bin first.
    for(int i=0;i<max_num-1;i++){
        if(set[i]->total_task_util < set[i+1]->total_task_util){
            alloc_set* temp = set[i];
            set[i] = set[i+1];
            set[i+1] = temp;
        }
        //tie-beaker. promote set with large chip num
        if(set[i]->total_task_util == set[i+1]->total_task_util){
            if(set[i]->chip_num > set[i+1]->chip_num){
                alloc_set* temp = set[i];
                set[i] = set[i+1];
                set[i+1] = temp;
            }
        }
    }
    for(int i=0;i<max_num-2;i++){
        if(set[i]->total_task_util < set[i+1]->total_task_util){
            alloc_set* temp = set[i];
            set[i] = set[i+1];
            set[i+1] = temp;
        }
        //tie-beaker. promote set with large chip num
        if(set[i]->total_task_util == set[i+1]->total_task_util){
            if(set[i]->chip_num > set[i+1]->chip_num){
                alloc_set* temp = set[i];
                set[i] = set[i+1];
                set[i+1] = temp;
            }
        }
    }
    //merge the last two allocation set.(smallest 2 bin)
    int cur_idx = 0;
    float temp_sum = 0.0;
    alloc_set* t1 = set[max_num-1];
    alloc_set* t2 = set[max_num-2];
    alloc_set* merged = (alloc_set*)malloc(sizeof(alloc_set));
    merged->chip_num = t1->chip_num + t2->chip_num;
    merged->task_num = t1->task_num + t2->task_num;
    for(int i=0;i<t1->task_num;i++){
        merged->task_ids[cur_idx] = t1->task_ids[i];
        merged->task_info_ptrs[cur_idx] = t1->task_info_ptrs[i];
        cur_idx++;
    }
    for(int i=0;i<t2->task_num;i++){
        merged->task_ids[cur_idx] = t2->task_ids[i];
        merged->task_info_ptrs[cur_idx] = t2->task_info_ptrs[i];
        cur_idx++;
    }
    
    for(int i=0;i<merged->task_num;i++){
        temp_sum += merged->task_info_ptrs[i]->task_util;
    }
    temp_sum += ERASE_LTN / (float)find_least_in_alloc(merged->task_num,merged);
    merged->total_task_util = temp_sum;
    free(set[max_num-1]);
    free(set[max_num-2]);
    set[max_num-2] = merged;
}

void insert_task_to_set(alloc_set* set,task_info* task){
    //old function. used in TTCBFD packing. 
    set->task_num += 1;
    set->task_ids[set->task_num-1] = task->task_id;
    set->task_info_ptrs[set->task_num-1] = task;
    set->total_task_util += task->task_util;
}

void print_set(alloc_set** set, int set_num){
    alloc_set* temp;
    for(int i=0;i<set_num;i++){
        temp = set[i];
        printf("chip_num : %d, alloc task : %d, util : %f\n",temp->chip_num,temp->task_num,temp->total_task_util);
    }
}

int calc_blocking_set(alloc_set* set){
    task_info* cur_ptr;
    int _task_num = set->task_num;
    int minimum_period = -1;
    for(int i=0;i<_task_num;i++){
        cur_ptr = set->task_info_ptrs[i];
        int temp = 0;
		//find least period.
		if(cur_ptr->read_period < cur_ptr->write_period &&(cur_ptr->read_period >= 0))
			temp = cur_ptr->read_period;
		else
			temp = cur_ptr->write_period;
		if(temp > cur_ptr->gc_period &&(cur_ptr->gc_period >= 0))
			temp = cur_ptr->gc_period;
		else{ /* do nothing */ }
		//found.
        if(minimum_period == -1)
            minimum_period = temp;
        else if(minimum_period >= temp)
            minimum_period = temp;
        
    }
    return minimum_period;
}
alloc_set** pack_bin_new(int task_num, int bin_num, task_info** task, int* config, int way, int* sched_res){
    int cur_bn = bin_num;
    double* bins = (double*)malloc(sizeof(double)*cur_bn);
    for(int i=0;i<cur_bn;i++)
        bins[i] = 0.0;
    
    double* expect = (double*)malloc(sizeof(double)*cur_bn);
    for(int i=0;i<cur_bn;i++)
        expect[i] = 0.0;

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
            task[i]->bin_alloc = j;
            blocking_period = find_least_in_bin(task_num,j,task);
            if(config[j] != min_bin){
                if(way == 0)
                    generate_overhead(&temp,CHANNEL_NB*config[j]);
                else
                    generate_overhead(&temp,config[j]);

            expect[j] = 1.0 - bins[j] - temp.task_util - (float)ERASE_LTN / (float)blocking_period;
            }
            else if(config[j] == min_bin){//minimum bin case
                expect[j] = 1.0 - bins[j] - task[i]->task_util - (float)ERASE_LTN / (float)blocking_period;
            }
            task[i]->bin_alloc = -1;
        }//found expectations for all bins.

        cur = find_max_double_idx(expect,cur_bn);

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
        if(res[i]->task_num > 0){
            blocking_period = find_least_in_bin(task_num,i,task);
            bins[i] += (float)ERASE_LTN / (float)blocking_period;
            res[i]->total_task_util += (float)ERASE_LTN / (float)blocking_period;
        }
    }

    *sched_res = 0;
    printf("current bin status : ");
    for(int i=0;i<cur_bn;i++){
        printf("[%f, %d] ",bins[i],res[i]->task_num);
        if(bins[i] >= 1.0)
            *sched_res = -1;
    }
    printf("\n");

    //free temp mems.
    free(bins);
    free(expect);
    return res;
}

alloc_set** pack_bin_new_BFD(int task_num, int bin_num, task_info** task, int* config, int way, int* sched_res){
    int cur_bn = bin_num;

    double* _bins = (double*)malloc(sizeof(double)*cur_bn);
    for(int i=0;i<cur_bn;i++)
        _bins[i] = 0.0;

    double* expect = (double*)malloc(sizeof(double)*cur_bn);
    for(int i=0;i<cur_bn;i++)
        expect[i] = 0.0;
  
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
        //reset bin_alloc
        task[i]->bin_alloc = -1;
        if(way == 0)
            generate_overhead(task[i],CHANNEL_NB*min_bin);
        else
            generate_overhead(task[i],min_bin);
    }
    quick_sort(task,0,task_num-1);

    //do the BFD packing
   
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
            task[i]->bin_alloc = j;
            //check if how blocking factor changes when task[i] belongs to j.
            blocking_period = find_least_in_bin(task_num,j,task);
            if(config[j] != min_bin){
                if(way == 0)
                    generate_overhead(&temp,CHANNEL_NB*config[j]);
                else
                    generate_overhead(&temp,config[j]);
                expect[j] = 1.0 - _bins[j] - temp.task_util - (float)ERASE_LTN / (float)blocking_period;
            }
            else if(config[j] == min_bin){//minimum bin case
                expect[j] = 1.0 - _bins[j] - task[i]->task_util - (float)ERASE_LTN / (float)blocking_period;
            }
            task[i]->bin_alloc = -1;

        }//found expectations for all bins.
        *sched_res = 0;
        

        cur = find_min_double_idx(expect,cur_bn);  
        if(cur == -1){
            printf("BFD failed\n");
            *sched_res = -1;
            return NULL;
        }

        if(config[cur] != min_bin){
            temp = *task[i];
            if(way == 0)
                generate_overhead(&temp,CHANNEL_NB*config[cur]);
            else
                generate_overhead(&temp,config[cur]);
            _bins[cur] += temp.task_util;
            task[i]->task_util = temp.task_util;
            insert_task_to_set(res[cur],task[i]);
            task[i]->bin_alloc = cur;
        }
        else{
            if(way == 0)
                generate_overhead(task[i],CHANNEL_NB*config[cur]);
            else
                generate_overhead(task[i],config[cur]);

            _bins[cur] += task[i]->task_util;
            insert_task_to_set(res[cur],task[i]);
            task[i]->bin_alloc = cur;
        }
    }//BFD bin packing finished.

    //add blocking factor
    for(int i=0;i<cur_bn;i++){
        if(res[i]->task_num > 0){
            blocking_period = find_least_in_bin(task_num,i,task);
            _bins[i] += (float)ERASE_LTN / (float)blocking_period;
            res[i]->total_task_util += (float)ERASE_LTN / (float)blocking_period;
        }
    }
    *sched_res = 0;
    
    printf("current bin status : ");
    for(int i=0;i<cur_bn;i++){
        printf("[%f, %d, %d] ",_bins[i],res[i]->chip_num,res[i]->task_num);
        if(_bins[i] >= 1.0)
            *sched_res = -1;
    }
    printf("\n");
    
    //free temp mems.
    free(_bins);
    free(expect);
    return res;
}

alloc_set** free_allocset(alloc_set** target, int num){
    for(int i=0;i<num;i++)
        free(target[i]);
    return NULL;
}
