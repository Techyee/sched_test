#include "sched_simul.h"

#define RIO 0
#define WIO 1
#define GCIO 2

#define TBS 1
#define BG 2

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

job* job_dequeue_head(jobhead* h){
    job* res = h->head;
    h->head = res->next;
    h->job_num--;
    res->prev = NULL;
    res->next = NULL;
    return res;
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
    cur_job->prev = NULL;
    cur_job->next = NULL;
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
    return new_job;
}

job* make_job_tbs(int IO_num, int IO_type, int start, int deadline, int target_set){
    job* new_job = (job*)malloc(sizeof(job));
    new_job->task_idx = -1;//TBS job.
    new_job->IO_type = IO_type;
    new_job->target_set_tbs = target_set;
    new_job->start_time_tbs = start;
    if(IO_type == RIO){
        new_job->cur_left_req = IO_num;
        new_job->total_req = IO_num;
        new_job->IO_deadline = deadline;
    }
    if(IO_type == WIO){
        new_job->cur_left_req = IO_num;
        new_job->total_req = IO_num;
        new_job->IO_deadline = deadline;
    }
    if(IO_type == GCIO){
        new_job->cur_left_req = PAGE_PER_BLOCK/2+1;
        new_job->total_req = PAGE_PER_BLOCK/2+1;
        new_job->IO_deadline = deadline;
    }
    return new_job;
}

job* make_job_bg(int IO_num, int IO_type){
    job* new_job = (job*)malloc(sizeof(job));
    new_job->task_idx = -2;//BG job.
    new_job->IO_type = IO_type;
    if(IO_type == RIO){
        new_job->cur_left_req = IO_num;
        new_job->total_req = IO_num;
        new_job->IO_deadline = __INT_MAX__;
    }
    if(IO_type == WIO){
        new_job->cur_left_req = IO_num;
        new_job->total_req = IO_num;
        new_job->IO_deadline = __INT_MAX__;
    }
    if(IO_type == GCIO){
        new_job->cur_left_req = PAGE_PER_BLOCK/2+1;
        new_job->total_req = PAGE_PER_BLOCK/2+1;
        new_job->IO_deadline = __INT_MAX__ - 10;
    }
    return new_job;
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

job* pick_job(jobhead* h){
    job* cur_job = h->head;
    return cur_job;
}

int EDF_simulator(alloc_set** allocsets, int set_num, int BEtype, int* IOnum, int IOtype){
    //params
    int i, j;
    int* cur_stat = (int*)malloc(sizeof(int)*set_num);
    int* cur_exec_left = (int*)malloc(sizeof(int)*set_num);
    job** cur_active_job = (job**)malloc(sizeof(job*)*set_num);
    int* cur_tbs_write = (int*)malloc(sizeof(int)*set_num);
    int* cur_tbs_dl = (int*)malloc(sizeof(int)*set_num);
    task_info** test_task;
    jobhead* jobmanager[set_num];//there must be a single job queue for each cluster.
    jobhead* job_tbs[set_num];
    jobhead* job_bg[set_num];
    int hyper_period = 5000000; // set 10sec as standard.
    int* sp_per_set = (int*)malloc(sizeof(int)*set_num); //monitor the shortest period.
    int* init_sp = (int*)malloc(sizeof(int)*set_num);
    //!params

    //checking allocset
    for(i=0;i<set_num;i++){
        alloc_set* targ = allocsets[i];
        printf("%d, %d, %f\n",targ->chip_num,targ->task_num,targ->total_task_util);
        for(j=0;j<targ->task_num;j++){
            task_info* infos = targ->task_info_ptrs[j];
            printf("task info %d\n",infos->read_num);
        }
    }
    //set sp_per_set.
    for(i=0;i<set_num;i++){
        sp_per_set[i] = __INT_MAX__;
        for(j=0;j<allocsets[i]->task_num;j++){
            task_info* temp = allocsets[i]->task_info_ptrs[j];
            if(temp->read_period < sp_per_set[i])
                sp_per_set[i] = temp->read_period;
            if(temp->write_period < sp_per_set[i])
                sp_per_set[i] = temp->write_period;
            if(temp->gc_period < sp_per_set[i])
                sp_per_set[i] = temp->gc_period;
        }
        //printf("smallest period : %d\n",sp_per_set[i]);
        init_sp[i] = sp_per_set[i];
    }//!set sp_per_set.
    
    //jobmanager init.
    for(i=0;i<set_num;i++){
        jobmanager[i] = queue_init();
        job_tbs[i] = queue_init();
        job_bg[i] = queue_init();
        cur_stat[i] = 0;
        cur_tbs_write[i] = 0;
        cur_tbs_dl[i] = 0;
        cur_active_job[i] = NULL;
    }
    
    printf("inited jobmanager\n");
    
    //worst case simulator. 
    //test the EDF-style sequential I/O execution.
    int* succ_IO = (int*)malloc(sizeof(int)*set_num);
    int* fail_IO = (int*)malloc(sizeof(int)*set_num);
    int* succ_TBS = (int*)malloc(sizeof(int)*set_num);
    int* fail_TBS = (int*)malloc(sizeof(int)*set_num);
    int* succ_BG = (int*)malloc(sizeof(int)*set_num);
    int* succ_TBS_R = (int*)malloc(sizeof(int)*set_num);
    int* succ_BG_R = (int*)malloc(sizeof(int)*set_num);
    for(int i=0;i<set_num;i++){
        succ_IO[i] = 0;
        fail_IO[i] = 0;
        succ_TBS[i] = 0;
        fail_TBS[i] = 0;
        succ_BG[i] = 0;
        succ_TBS_R[i] = 0;
        succ_BG_R[i] = 0;
    }
    int checker = 0;
    int reclaim_page = PAGE_PER_BLOCK / 2; //FIXME::hardcoded reclaimable page.
    int cur_tbs_rsv = 0;
    
    //necessary for BG.
    int* prev_done_gc = (int*)malloc(sizeof(int)*set_num);//marks handled workload.
    for(j=0;j<set_num;j++)
        prev_done_gc[j] = 0;

    ///SIMULATOR STARTS!
    for(i=0;i<hyper_period;i++){//until we reach the hyper_period,
        //initialize jobs.
        for(j=0;j<set_num;j++){//for each allocation set,
            int task_num = allocsets[j]->task_num;
            for(int k=0;k<task_num;k++){//enqueue job.
                job* temp;
                if(i % allocsets[j]->task_info_ptrs[k]->read_period == 0 && (allocsets[j]->task_info_ptrs[k]->read_period > 0)){
                    temp = make_job(allocsets[j]->task_info_ptrs[k],RIO,i);
                    //printf("[ENQ]time : %d, cur_task : %d, type: %d, left : %d, dl : %d\n",i,temp->task_idx,temp->IO_type,temp->cur_left_req,temp->IO_deadline);
                    job_enqueue(jobmanager[j],temp);   
                }
                if(i % allocsets[j]->task_info_ptrs[k]->write_period == 0 && (allocsets[j]->task_info_ptrs[k]->write_period > 0)){
                    temp = make_job(allocsets[j]->task_info_ptrs[k],WIO,i);
                    //printf("[ENQ]time : %d, cur_task : %d, type: %d, left : %d, dl : %d\n",i,temp->task_idx,temp->IO_type,temp->cur_left_req,temp->IO_deadline);
                    job_enqueue(jobmanager[j],temp);
                }
                if(i % allocsets[j]->task_info_ptrs[k]->gc_period == 0 && (allocsets[j]->task_info_ptrs[k]->gc_period > 0)){
                    temp = make_job(allocsets[j]->task_info_ptrs[k],GCIO,i);
                    //printf("[ENQ]time : %d, cur_task : %d, type: %d, left : %d, dl : %d\n",i,temp->task_idx,temp->IO_type,temp->cur_left_req,temp->IO_deadline);
                    job_enqueue(jobmanager[j],temp);
                }
            }
        }
        //[TBS]initialize jobs.(cur TBS workload -> 128pg write. arrives every 1ms.)

        if(BEtype == TBS){
        int tbs_write;
        int tbs_read;
        if(IOtype == WTEST){
            tbs_write = 128;
            tbs_read = 0; //FIXME:: write-only hardcoded
        }else if(IOtype == RTEST){
            tbs_write = 0;
            tbs_read = 128;
        }
        int shortest_dl = -1;
        int target_set;
        int start_time;
        int temp_start;
        int dl;
        int gc_exec;
        int w_exec;
        int r_exec;
        int do_tbs = 0;
        int be_period;
        if(IOtype == WTEST)
            be_period = 10000;
        if(IOtype == RTEST)
            be_period = 1000;
        if(i % be_period == 0) 
            do_tbs = 1;
        else
            do_tbs = 0;
        
        if(do_tbs == 1){
            for(j=0;j<set_num;j++){
                
                //calc job and deadline according to allocset left util.
                float util_left = 1.0 - allocsets[j]->total_task_util;
                int worst_dt = (4 - (allocsets[j]->chip_num % WAY_NB)) % WAY_NB;
                int _w_exec = tbs_write*(WRITE_LTN+worst_dt*DATA_TRANS);
                int _r_exec = tbs_read*(READ_LTN+worst_dt*DATA_TRANS);
                int _gc_exec = ((cur_tbs_write[j] + tbs_write) / (reclaim_page*allocsets[j]->chip_num)) * GC_EXEC;
                float dl_f = (float)(_w_exec+_gc_exec+_r_exec)*(1.0/util_left);
                //printf("util : %f, gen_util : %f, exec : %d, dl : %f\n",util_left,(float)(_w_exec+_gc_exec)/dl_f,_w_exec+_gc_exec,dl_f);
                if(i >= cur_tbs_dl[j]){
                    temp_start = i;
                    dl = (int)dl_f+1+i;
                }
                else{
                    temp_start = cur_tbs_dl[j];
                    dl = (int)dl_f+1+cur_tbs_dl[j];
                }

                //find a set with shortest deadline.
                if(j==0){
                    target_set = j;
                    shortest_dl = dl;
                    start_time = temp_start;
                    r_exec = _r_exec;//export workload.
                    w_exec = _w_exec;
                    gc_exec = _gc_exec;
                }
                else if(shortest_dl >= dl){
                    target_set = j;
                    shortest_dl = dl;
                    start_time = temp_start;
                    r_exec = _r_exec;//export workload.
                    w_exec = _w_exec;
                    gc_exec = _gc_exec;
                }
            }//!found shortest set.
            //printf("[TBS]current : %d, exec : %d, start : %d, dl : %d, util_left : %f, util : %f\n",i,
            //w_exec+gc_exec,start_time, shortest_dl,1.0 - allocsets[target_set]->total_task_util,(float)(w_exec+gc_exec)/(float)(shortest_dl-start_time));
            //4-1. enqueue the job on tbs queue.
            cur_tbs_dl[target_set] = shortest_dl;
            if(IOtype == WTEST){
                job* temp = make_job_tbs(tbs_write,WIO,start_time,shortest_dl,target_set);
                job_enqueue(job_tbs[target_set],temp);
            }else if(IOtype == RTEST){
                job* temp = make_job_tbs(tbs_read,RIO,start_time,shortest_dl,target_set);
                job_enqueue(job_tbs[target_set],temp);
            }
            cur_tbs_write[target_set] = (cur_tbs_write[target_set] + tbs_write) % (reclaim_page*allocsets[target_set]->chip_num);
            //check shortest period.
            if(shortest_dl - start_time < sp_per_set[target_set])
                sp_per_set[target_set] = shortest_dl - start_time;
            if(gc_exec > 0){
                job* temp2 = make_job_tbs(reclaim_page,GCIO,start_time,shortest_dl,target_set);
                job_enqueue(job_tbs[target_set],temp2);
                cur_tbs_write[target_set] = (cur_tbs_write[target_set] + tbs_write) % (reclaim_page*allocsets[target_set]->chip_num);
            }     
        }//tbs enqueued.

        //4-2. move the active job to actual jobqueue.        
        jobhead* tbs_head;
        job* tbs_temp;
        for(j=0;j<set_num;j++){
            tbs_head = job_tbs[j];
            tbs_temp = tbs_head->head;
            if(tbs_temp != NULL){
                while(tbs_temp->start_time_tbs == i){//head is shortest start time.
                    job* res = job_dequeue_head(tbs_head);//dequeue until time does not match.
                    job_enqueue(jobmanager[j],res);
                    tbs_temp = tbs_head->head;
                    if(tbs_temp == NULL)
                        break;
                }
            }
        }//active_tbs enqueued.     
        }


        if(BEtype==BG){
        //[BG] enqueue job BG_queue(dl = __INT_MAX__)
        /*code here*/
        int bg_write = 128;
        int bg_read = 128;
        int do_bg = 0;

        //check if succ_BG[j] reaches gc threshold(if certain set reaches threshold, must do GC.)
        for(j=0;j<set_num;j++){
            if((succ_BG[j] % (reclaim_page*allocsets[j]->chip_num) == 0) && (succ_BG[j] != 0) && (prev_done_gc[j] != succ_BG[j])){
                prev_done_gc[j] = succ_BG[j];//mark how much workload is already handled.
                job* temp = make_job_bg(0,GCIO);
                job_enqueue(job_bg[j],temp);
            }
        }
        int be_period;
        if(IOtype == WTEST)
            be_period = 10000;
        else if(IOtype == RTEST)
            be_period = 1000;
        if(i % be_period == 0) 
            do_bg = 1;
        else
            do_bg = 0;

        if(do_bg == 1){//sched bg workload every 10ms.
            for(j=0;j<set_num;j++){
                if(IOtype == WTEST){
                    job* temp = make_job_bg(bg_write,WIO);
                    job_enqueue(job_bg[j],temp);
                }else if(IOtype == RTEST){
                    job* temp = make_job_bg(bg_read,RIO);
                    job_enqueue(job_bg[j],temp);
                }
            }
        }
        }
       
        //finish job & pick job.
        for(int j=0;j<set_num;j++){//finish req.
            if((cur_active_job[j] != NULL) && (cur_exec_left[j] == 0)){//finish req.
                //printf("[set %d finish]time : %d, cur_task : %d, type: %d, left : %d\n",j,i,cur_active_job[j]->task_idx,
                //cur_active_job[j]->IO_type,cur_active_job[j]->cur_left_req);
                if((cur_active_job[j]->task_idx >= 0) && (cur_active_job[j]->IO_type == WIO))
                    succ_IO[j]++;
        
                cur_stat[j] = 0;
                if((cur_active_job[j]->task_idx == -1) &&(cur_active_job[j]->IO_type == WIO))
                    succ_TBS[j]++;
                if((cur_active_job[j]->task_idx == -1) &&(cur_active_job[j]->IO_type == RIO))
                    succ_TBS_R[j]++;
                if((cur_active_job[j]->task_idx == -2) &&(cur_active_job[j]->IO_type == WIO))
                    succ_BG[j]++;
                if((cur_active_job[j]->task_idx == -2) &&(cur_active_job[j]->IO_type == RIO))
                    succ_BG_R[j]++;
                
                if(cur_active_job[j]->cur_left_req == 0){//finish job.
                    if((cur_active_job[j]->task_idx != -1) && i >= cur_active_job[j]->IO_deadline){
                        printf("task %d miss dl, %d\n",cur_active_job[j]->task_idx,cur_active_job[j]->IO_deadline);
                        fail_IO[j] += 1;
                    }
                    else if((cur_active_job[j]->task_idx == -1) && i >= cur_active_job[j]->IO_deadline){
                        printf("task %d miss dl, %d\n",cur_active_job[j]->task_idx,cur_active_job[j]->IO_deadline);
                        fail_TBS[j] += 1;
                    }
                    if(cur_active_job[j]->task_idx != -2)
                        job_dequeue(jobmanager[j],cur_active_job[j]);
                    else if(cur_active_job[j]->task_idx == -2)
                        job_dequeue(job_bg[j],cur_active_job[j]);
                    else
                        abort();
                }
            }
        }//!finish.

        for(int j=0;j<set_num;j++){//pick job(req).
            if(cur_stat[j] == 0){
                cur_stat[j] = 1; //set as active, and find new job.
                job* now = pick_early_job(jobmanager[j]);
                cur_active_job[j] = now;
                if(now == NULL){//no active RT job.
                    now = pick_early_job(job_bg[j]);
                    cur_active_job[j] = now;
                    if(now == NULL){//no active BG job.
                        cur_stat[j] = 0;
                        continue;
                    }
                }
                int worst_dt = (4 - (allocsets[j]->chip_num % WAY_NB)) % WAY_NB;
                //!done
                if(now->IO_type == RIO)
                    cur_exec_left[j] = READ_LTN + worst_dt*DATA_TRANS;
                if(now->IO_type == WIO)
                    cur_exec_left[j] = WRITE_LTN + worst_dt*DATA_TRANS;
                if(now->IO_type == GCIO){
                    cur_exec_left[j] = 550; //FIXME:: CPB hardcoded
                    if(now->cur_left_req == 0)
                        cur_exec_left[j] = ERASE_LTN;
                }
                //printf("[set %d pick]time : %d, cur_task : %d, type: %d, left : %d,left_exec : %d\n",j,i,now->task_idx,now->IO_type,now->cur_left_req,cur_exec_left[j]);    
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
    int succ_BG_sum = 0;
    int succ_RT_sum = 0;
    int succ_TBS_sum = 0;
    int succ_BGR_sum = 0;
    int succ_TBSR_sum = 0;
    for(int i=0;i<set_num;i++){
        succ_BG_sum += succ_BG[i];
        succ_RT_sum += succ_IO[i];
        succ_TBS_sum += succ_TBS[i];
        succ_BGR_sum += succ_BG_R[i];
        succ_TBSR_sum += succ_TBS_R[i];
       // printf("init_sp : %d, end_sp : %d, success : %d, fail : %d, TBS : %d, fail(TBS) : %d BG : %d\n",init_sp[i],sp_per_set[i],succ_IO[i],fail_IO[i],succ_TBS[i],fail_TBS[i],succ_BG[i]);
    }
    
    if(IOtype == WTEST){
        printf("RT : %d, TBS : %d, BG : %d\n",succ_RT_sum,succ_TBS_sum,succ_BG_sum);
        if(BEtype == TBS)
            *IOnum = succ_TBS_sum;
        else if(BEtype == BG)
            *IOnum = succ_BG_sum;
    }else if(IOtype == RTEST){
        printf("RT : %d, TBS : %d, BG : %d\n",succ_RT_sum,succ_TBSR_sum,succ_BGR_sum);

        if(BEtype == TBS)
            *IOnum = succ_TBSR_sum;
        else if(BEtype == BG)
            *IOnum = succ_BGR_sum;
    }
}