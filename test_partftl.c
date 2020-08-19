#include "sched_simul.h"

int ENCOD_LTN = 1200;
int DECOD_LTN = 1200;
int PARITY_CHIP = 4;
int DATA_CHIP = 12;
int W_CHIP = 4;
int R_CHIP = 12;
int test_PARTFTL(int task_num, task_info* task){
	
    /* this function initiates schedulability test based on PaRT_FTL.
	 * we must check both of read schedulability test and write schedulability test.
     * 1. get the read and write request information from task_info structure.
     * 2. calculate the read and write utilization according to PaRT_FTL paper. 
     * 3. calculate encoding and GC utilization according to PaRT_FTL paper.
	 */

    task_info* temp;
    double read_util = 0.0;
    double write_util = 0.0;
    
    int i;
    int temp_r_exec, temp_r_period;
    int temp_w_exec, temp_w_period;
    int temp_e_exec, temp_e_period;
    int temp_gc_exec, temp_gc_period;
    double temp_gc_threshold;

    for(i=0;i<task_num;i++)
    {
        temp_task = task_info[i];

        temp_r_exec = temp->read_num * (DATA_TRANS * 2 + READ_LTN + DECOD_LTN);
        temp_r_period = temp->read_period;
        read_util += (double)temp_r_exec / (double)temp_r_period;

        temp_w_exec = ceil((double)temp->write_num / (double)CHANNEL_NB) * WRITE_LTN;
        temp_w_period = temp->write_period;
        write_util += (double)temp_w_exec / (double)temp_w_period;

        temp_e_exec = PARITY_CHIP * PAGE_PER_BLOCK * ENCOD_LTN + ceil((double)(PARITY_CHIP * PAGE_PER_BLOCK) / (double)WRITE_CHIP) * WRITE_LTN;
        temp_e_period = (double)temp->write_period * DATA_CHIP * PAGE_PER_BLOCK / (double)temp->write_num;
        write_util += (double)temp_e_exec / (double)temp_e_period;

        temp_gc_exec = (DATA_CHIP + PARITY_CHIP / W_CHIP) * GC_EXEC;
        temp_gc_threshold = (double)DATA_CHIP*(1.0 - OP_RATE)*PAGE_PER_BLOCK;
        if (gc_threshold < new_task->write_num)
        {
            gc_period_float = (float)temp->write_period / gc_period_ratio;
        }
        else
        {
            gc_period_float = (float)temp->write_period * (int)(1.0/gc_period_ratio);
        }
        temp_gc_period = (int)gc_period_float;
        write_util += temp_gc_exec / temp_gc_period;

    }
    //now, check if the read_util or write_util goes over 1.


        
    
}

