#include "sched_simul.h"

#define RIO 0
#define WIO 1
#define GCIO 2



jobhead* queue_init(){
    jobhead* jobqueue_head = (jobhead*)malloc(sizeof(jobhead));
    jobqueue_head->job_num = 0;
    jobqueue_head->head = NULL;
    return jobqueue_head;
}

void job_enqueue(jobhead* h, job* target){
    job* cur_job = h->head;
    if(h->head == NULL){
        h->head = target;
        target->next = NULL;
        h->job_num++;
        return;
    }
    else{
        while(cur_job->next != NULL)
            cur_job = cur_job->next;
    }
    cur_job->next = target;
    target->prev = cur_job;
    target->next = NULL;
    h->job_num++;
    return;
}

void job_dequeue(jobhead* h, job* target){
    job* cur_job = h->head;
    job* temp;
    if(cur_job == target){
        h->head = cur_job->next;
        free(cur_job);
        h->job_num--;
        if(h->job_num == 0)
            h->head = NULL;
        return;
    }
    else{
        while(cur_job != target)
            cur_job = cur_job->next;
    }
    if(cur_job->prev != NULL)
        cur_job->prev->next = cur_job->next;
    if(cur_job->next != NULL)
        cur_job->next->prev = cur_job->prev;
    free(cur_job);
    h->job_num--;
    return;
}

job* job_pop(jobhead* h, job* target){
    job* cur_job = h->head;
    if(cur_job == target){
        h->head = cur_job->next;
        h->job_num--;
        if(h->job_num == 0)
            h->head = NULL;
        return cur_job;
    }
    else{
        while(cur_job != target)
            cur_job = cur_job->next;
    }
    if(cur_job->prev != NULL)
        cur_job->prev->next = cur_job->next;
    if(cur_job->next != NULL)
        cur_job->next->prev = cur_job->prev;
    h->job_num--;
    return cur_job;
}

job* make_job(task_info* target, int IO_type, int current){
    job* new_job = (job*)malloc(sizeof(job));
    new_job->task_idx = target->task_id;
    new_job->IO_type = IO_type;
    if(IO_type == RIO){
        new_job->cur_left_req = target->read_num;
        new_job->total_req = target->read_num;
        new_job->IO_deadline = target->read_period + current;
    }
    if(IO_type == WIO){
        new_job->cur_left_req = target->write_num;
        new_job->total_req = target->write_num;
        new_job->IO_deadline = target->write_period + current;
    }
    if(IO_type == GCIO){
        new_job->cur_left_req = PAGE_PER_BLOCK/2+1;
        new_job->total_req = PAGE_PER_BLOCK/2+1;
        new_job->IO_deadline = target->gc_period + current;
    }
}

job* make_job_tbs(int IO_num, int IO_type, int start, int deadline,int target_set){
    job* new_job = (job*)malloc(sizeof(job));
    new_job->task_idx = -1;//TBS job.
    new_job->IO_type = IO_type;
    new_job->target_set_tbs = target_set;
    new_job->start_time_tbs = start;
    if(IO_type == RIO){
        new_job->cur_left_req = IO_num;
        new_job->total_req = IO_num;
        new_job->IO_deadline = start+deadline;
    }
    if(IO_type == WIO){
        new_job->cur_left_req = IO_num;
        new_job->total_req = IO_num;
        new_job->IO_deadline = start+deadline;
    }
    if(IO_type == GCIO){
        new_job->cur_left_req = PAGE_PER_BLOCK/2+1;
        new_job->total_req = PAGE_PER_BLOCK/2+1;
        new_job->IO_deadline = start+deadline;
    }
}
job* pick_early_job(jobhead* h){
    job* cur_job = h->head;
    job* ret;
    int cur_jnum;
    int cur_earliest_dl;
    cur_jnum = h->job_num;
    if(cur_jnum != 0){
        cur_earliest_dl = cur_job->IO_deadline;
        for(int i=0;i<cur_jnum;i++){
            if(cur_job->IO_deadline <= cur_earliest_dl){
                cur_earliest_dl = cur_job->IO_deadline;
                ret = cur_job;
            }
            cur_job = cur_job->next;
        }
        ret->cur_left_req -= 1; //decrease request number.
        return ret;
    }
    else if(cur_jnum == 0){
        return NULL;
    }
}
int EDF_simulator(alloc_set** allocsets, int set_num){
    //params
    int i, j;
    int* cur_stat = (int*)malloc(sizeof(int)*set_num);
    int* cur_exec_left = (int*)malloc(sizeof(int)*set_num);
    job** cur_active_job = (job**)malloc(sizeof(job*)*set_num);
    int* cur_tbs_write = (int*)malloc(sizeof(int)*set_num);
    int* cur_tbs_dl = (int*)malloc(sizeof(int)*set_num);
    task_info** test_task;
    jobhead* jobmanager[set_num];//there must be a single job queue for each cluster.
    jobhead* job_tbs;
    int hyper_period = 5000000; // set 5sec as standard. 
    //!params

    //jobmanager init.
    for(i=0;i<set_num;i++){
        jobmanager[i] = queue_init();
        cur_stat[i] = 0;
        cur_tbs_write[i] = 0;
    }
    job_tbs = queue_init();
    printf("inited jobmanager\n");
    
    //worst case simulator. 
    //test the EDF-style sequential I/O execution.
    int succ_IO = 0;
    int fail_IO = 0;
    int succ_TBS = 0;
    int reclaim_page = PAGE_PER_BLOCK / 2; //FIXME::hardcoded reclaimable page. 
    for(i=0;i<hyper_period;i++){//until we reach the hyper_period,
        //initialize jobs.
        for(j=0;j<set_num;j++){//for each allocation set,
            int task_num = allocsets[j]->task_num;
            
            for(int k=0;k<task_num;k++){//enqueue job.
                if(i % allocsets[j]->task_info_ptrs[k]->read_period == 0){
                    job* temp = make_job(allocsets[j]->task_info_ptrs[k],RIO,i);
                    printf("[ENQ]time : %d, cur_task : %d, type: %d, left : %d, dl : %d\n",i,temp->task_idx,temp->IO_type,temp->cur_left_req,temp->IO_deadline);
                    job_enqueue(jobmanager[j],temp);   
                }
                if(i % allocsets[j]->task_info_ptrs[k]->write_period == 0){
                    job* temp = make_job(allocsets[j]->task_info_ptrs[k],WIO,i);
                    printf("[ENQ]time : %d, cur_task : %d, type: %d, left : %d, dl : %d\n",i,temp->task_idx,temp->IO_type,temp->cur_left_req,temp->IO_deadline);
                    job_enqueue(jobmanager[j],temp);
                }
                if(i % allocsets[j]->task_info_ptrs[k]->gc_period == 0){
                    job* temp = make_job(allocsets[j]->task_info_ptrs[k],GCIO,i);
                    printf("[ENQ]time : %d, cur_task : %d, type: %d, left : %d, dl : %d\n",i,temp->task_idx,temp->IO_type,temp->cur_left_req,temp->IO_deadline);
                    job_enqueue(jobmanager[j],temp);
                }
            }
        }
        //if TBS, also initialize and alloc TBS.(cur TBS workload -> 128pg write. arrives every 1ms.)
        int tbs_write = 128;
        int tbs_read = 128;
        int shortest_dl = -1;
        int target_set;
        int start_time;
        int temp_start;
        int dl;
        int gc_exec;
        int w_exec;
        int do_tbs = 0;
        if(i % 1000 == 0) 
            do_tbs = 1;
        else
            do_tbs = 0;
        
        if(do_tbs == 1){////watch out for indent.
        for(j=0;j<set_num;j++){
            //1. for each allocset check how much util left.
            //2. calc job and deadline according to allocset left util.
            float util_left = 1.0 - allocsets[j]->total_task_util;
            w_exec = tbs_write*WRITE_LTN;
            gc_exec = ((cur_tbs_write[j] + tbs_write) / (reclaim_page*allocsets[j]->chip_num)) * GC_EXEC;
            float dl_f = (w_exec+gc_exec) * 1.0/util_left;         
            if(i >= cur_tbs_dl[j]){
                temp_start = i;
                dl = (int)dl_f + i;
            }
            else{
                temp_start = cur_tbs_dl[j];
                dl = (int)dl_f + cur_tbs_dl[j];
            }
            //3. find a set with shortest deadline.
            if(j==0){
                target_set = j;
                shortest_dl = dl;
                start_time = temp_start;
            }
            else if(shortest_dl >= dl){
                target_set = j;
                shortest_dl = dl;
                start_time = temp_start;
            }
        }
        //!found shortest set.
        printf("[TBS]target set : %d, chip num : %d, exec : %d, dl : %d\n",target_set,allocsets[target_set]->chip_num,
        w_exec+gc_exec,shortest_dl);
        //4-1. enqueue the job on tbs queue.
        cur_tbs_dl[target_set] = shortest_dl;
        if(gc_exec > 0){
            cur_tbs_write[target_set] = (cur_tbs_write[target_set] + tbs_write) % (reclaim_page*allocsets[target_set]->chip_num);
            printf("cur_tbs_write is %d\n",cur_tbs_write[target_set]);
        }
        job* temp = make_job_tbs(tbs_write,WIO,start_time,shortest_dl,target_set);
        job* temp2 = make_job_tbs(reclaim_page,GCIO,start_time,shortest_dl,target_set);
        job_enqueue(jobmanager[temp->target_set_tbs],temp);
        printf("[TBSENQ]time : %d, cur_task : %d, type: %d, left : %d, dl : %d\n",i,temp->task_idx,temp->IO_type,temp->cur_left_req,temp->IO_deadline);
        job_enqueue(jobmanager[temp->target_set_tbs],temp2);
        printf("[TBSENQ]time : %d, cur_task : %d, type: %d, left : %d, dl : %d\n",i,temp2->task_idx,temp2->IO_type,temp2->cur_left_req,temp2->IO_deadline);
        }//do_tbs end.
        //if BG, just enqueue on BG_queue(no dl. always lowest priority.)
        /*code here*/
        //finish job & pick job.
        for(int j=0;j<set_num;j++){//finish job.
            if((cur_active_job[j] != NULL) && (cur_exec_left[j] == 0)){//finish req.
                printf("[set %d finish]time : %d, cur_task : %d, type: %d, left : %d\n",j,i,cur_active_job[j]->task_idx,
                cur_active_job[j]->IO_type,cur_active_job[j]->cur_left_req);
                cur_stat[j] = 0;
                if((cur_active_job[j]->task_idx == -1) &&(cur_active_job[j]->IO_type == WIO))
                    succ_TBS++;
                if(cur_active_job[j]->cur_left_req == 0){//if there's no more request left,
                    if(i < cur_active_job[j]->IO_deadline)
                        succ_IO += 1;
                    else{
                        if(cur_active_job[j]->task_idx != -1)
                            fail_IO += 1;
                    }
                    job_dequeue(jobmanager[j],cur_active_job[j]);
                }
            }
        }//!finish.

        for(int j=0;j<set_num;j++){//pick job(in detail, req).
            if(cur_stat[j] == 0){
                cur_stat[j] = 1; //set as active, and find new job.
                job* now = pick_early_job(jobmanager[j]);
                cur_active_job[j] = now;
                if(now == NULL){
                    cur_stat[j] = 0;
                    continue;
                }
                
                if(now->IO_type == RIO)
                    cur_exec_left[j] = READ_LTN;
                if(now->IO_type == WIO)
                    cur_exec_left[j] = WRITE_LTN;
                if(now->IO_type == GCIO){
                    cur_exec_left[j] = 550;//FIXME:: CPB hardcoded
                    if(now->cur_left_req == 0)
                        cur_exec_left[j] = ERASE_LTN;
                }
                printf("[set %d pick]time : %d, cur_task : %d, type: %d, left : %d,left_exec : %d\n",j,i,now->task_idx,now->IO_type,now->cur_left_req,cur_exec_left[j]);
                
            }
            //if BG, pick BG job only if RT queue is none.
            //GC of BG occurs in simple way(write -> GC -> write, all in completely BG manner.
            /*code here*/
        }//!pick.

        //progress job.
        for(int j=0;j<set_num;j++){
            if(cur_stat[j] == 1){
                cur_exec_left[j] -= 1;
            }
        } 

    }//!simulation

    printf("success : %d, fail : %d, TBS : %d\n",succ_IO,fail_IO,succ_TBS);
}