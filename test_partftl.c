#include "sched_simul.h"

//parameters
int ENCOD_LTN = 120;
int DECOD_LTN = 120;
int PARITY_CHIP = 4;
int DATA_CHIP = 12;
int W_CHIP = 4;
int R_CHIP = 12;
//!parameters

int myceil(float input){
	if(input - (int)input > 0)
		return (int)input + 1;
	else
		return (int)input;
}

int test_PARTFTL(int task_num, task_info** task,FILE*fp){
	
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
	double gc_period_float;
	double gc_period_ratio;
    double temp_gc_threshold;

    for(i=0;i<task_num;i++)
    {
        temp = task[i];

        temp_r_exec = temp->read_num * (DATA_TRANS * 2 + READ_LTN + DECOD_LTN);
        temp_r_period = temp->read_period;
        read_util += (double)temp_r_exec / (double)temp_r_period;	
		printf("read_exec : %f, read_period : %f, read_util : %f\n",(double)temp_r_exec, (double)temp_r_period, (double)temp_r_exec / (double)temp_r_period );
        
		temp_w_exec = (double)temp->write_num * WRITE_LTN;
        temp_w_period = temp->write_period;
        write_util += (double)temp_w_exec / (double)temp_w_period;
		printf("write_exec : %f, write_period : %f, write_util : %f\n",(double)temp_w_exec, (double)temp_w_period , (double)temp_w_exec / (double)temp_w_period);
        
		temp_e_exec = PARITY_CHIP * PAGE_PER_BLOCK * ENCOD_LTN + myceil((double)(PARITY_CHIP * PAGE_PER_BLOCK) / (double)W_CHIP) * WRITE_LTN;
        temp_e_period = (double)temp->write_period * DATA_CHIP * PAGE_PER_BLOCK / (double)temp->write_num;
        write_util += (double)temp_e_exec / (double)temp_e_period;
		printf("encod_exec : %f, encod_period : %f, encod_util : %f\n",(double)temp_e_exec, (double)temp_e_period, (double)temp_e_exec / (double)temp_e_period);
        
		temp_gc_exec = ((DATA_CHIP + PARITY_CHIP) / W_CHIP) * GC_EXEC;
        temp_gc_threshold = (double)DATA_CHIP*(1.0 - OP_RATE)*PAGE_PER_BLOCK;
		gc_period_ratio = (float)temp->write_num/(float)temp_gc_threshold; 
        if (temp_gc_threshold < temp->write_num)
        {
            gc_period_float = (float)temp->write_period / gc_period_ratio;
        }
        else
        {
            gc_period_float = (float)temp->write_period * (int)(1.0/gc_period_ratio);
        }
        temp_gc_period = (int)gc_period_float;
        write_util += (float)temp_gc_exec / (float)temp_gc_period;
		printf("gc_exec : %d, gc_period : %d, gc_util : %f\n\n",temp_gc_exec, temp_gc_period, (float)temp_gc_exec / (float)temp_gc_period);
    }
    //now, check if the read_util or write_util goes over 1.
	if((read_util > 1.0) || (write_util > 1.0))
	{
		printf("schedule failed, read : %f, write : %f\n",read_util, write_util);
		return 1;
	}
	else{
		printf("schedule successed, read : %f, write : %f\n",read_util, write_util);
    	return 0;
	}

}

