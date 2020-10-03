#include "sched_simul.h"

typedef struct _job{
    int task_idx;
    int IO_type;
    int cur_IO_exec;
    int IO_deadline;
    job* prev;
    job* next;
}job;

typedef struct _jobhead{
    int job_num;
    job* head;
}jobhead;

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
        return;
    }
    else{
        while(cur_job->next != NULL)
            cur_job = cur_job->next;
    }
    cur_job->next = target;
    target->prev = cur_job;
}

void job_dequeue(jobhead* h, job* target){
    job* cur_job = h->head;
    job* temp;
    if(cur_job == h->head){
        h->head = cur_job->next;
        free(cur_job);
        return;
    }
    else{
        while(cur_job != target)
            cur_job = cur_job->next;
    }
    cur_job->prev->next = cur_job->next;
    cur_job->next->prev = cur_job->prev;
    free(cur_job);
    return;
}
job* make_job(task_info* target, int IO_type){


}
int main(void){
    int i, j;
    int task_num = 4;
    int64_t hyper_period = 1;
    task_info** test_task;
    job* temp_read;
    job* temp_write;
    jobhead* jobmanager;
    test_task = generate_taskset(task_num,1.0,16,1);
    for(i=0;i<task_num-1;i++){
        int temp1 = lcm(test_task[i]->read_period,test_task[i]->write_period);
        int temp2 = lcm(test_task[i+1]->read_period,test_task[i+1]->write_period);
        int lcm_task = lcm(temp1,temp2);
        hyper_period = lcm(hyper_period, lcm_task);
    }
    printf("hyper period is %d",hyper_period);

    //worst case simulator. 
    //Test the EDF-style sequential I/O execution,
    //with event-driven GC.

    int under_GC = 0;
    int GC_counter;
    int GC_status;
    int free_pages = 64;
    int cur_free_pages = free_pages;
    jobmanager = queue_init();
    for(i=0;i<hyper_period;i++){
        //enqueue the released job.
        for(j=0;j<task_num;j++){
            if(i % test_task[j]->read_period == 0){
                temp_read = make_job(test_task[j],0);
                job_enqueue(jobmanager,temp_read);
            }
            else if(i % test_task[j]->write_period == 0){
                temp_write = make_job(test_task[j],1);
                job_enqueue(jobmanager,temp_write);
            }
        }//!enqueue
        
        //dequeue the finished job.
        //!dequeue

        //if free page runs out, mark that we'll run GC.
        if(free_pages == 0 && GC_status == 0){
            under_GC = 1; //mark that we'll just decrement GC counter.
            GC_status = 1; //turn on GC status.
            GC_counter = GC_EXEC;
        }
        //if we're in garbage collection period, just decrement the GC value and exit.
        if(under_GC == 1){
            GC_counter--;
            if(GC_counter == 0){
                under_GC = 0;
                GC_status = 0;
                cur_free_pages = free_pages;
            }
            continue;
            
        }
        //!end of gc passing logic.

        //initiate one of the scheduled I/O.
    }
}