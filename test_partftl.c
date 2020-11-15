#include "sched_simul.h"

//parameters
int ENCOD_LTN = 0;
int DECOD_LTN = 0; //assume ENCOD/DECODE is negligible(handled by hardware)
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

int calc_PART_bandwidth(int hyp, float util, float util2, float* throughputs){
	int hyp_us = hyp*1000*1000;
	int max_w = hyp_us / WRITE_LTN * 3;
	int max_r = hyp_us / READ_LTN;
	int max_wpage = 0;
	int max_rpage = 0;
	int max_readpage = 0;
	float write_util;
	float read_util;
	int temp_w_exec, temp_w_period;
	int temp_r_exec, temp_r_period;
	int temp_e_exec, temp_e_period;
	int temp_gc_exec, temp_gc_period;
	double gc_period_float;
	double gc_period_ratio;
	double temp_gc_threshold;
	for(int i=0;i<max_w;i++){
		write_util = 0.0;
		temp_w_exec = myceil((double)i / (double)W_CHIP) * WRITE_LTN;
    	temp_w_period = hyp_us;
    	write_util += (double)temp_w_exec / (double)temp_w_period;
		//printf("write_exec : %f, write_period : %f, write_util : %f\n",(double)temp_w_exec, (double)temp_w_period , (double)temp_w_exec / (double)temp_w_period);
        
		temp_e_exec = PARITY_CHIP * PAGE_PER_BLOCK * ENCOD_LTN + myceil((double)(PARITY_CHIP * PAGE_PER_BLOCK) / (double)W_CHIP) * WRITE_LTN;
    	temp_e_period = (double)hyp_us * DATA_CHIP * PAGE_PER_BLOCK / (double)i;
    	write_util += (double)temp_e_exec / (double)temp_e_period;
		//printf("encod_exec : %f, encod_period : %f, encod_util : %f\n",(double)temp_e_exec, (double)temp_e_period, (double)temp_e_exec / (double)temp_e_period);
        
		temp_gc_exec = ((DATA_CHIP + PARITY_CHIP) / W_CHIP) * GC_EXEC;
    	temp_gc_threshold = (double)DATA_CHIP*(1.0 - OP_RATE)*PAGE_PER_BLOCK;
		gc_period_ratio = (float)i/(float)temp_gc_threshold; 
    	if(temp_gc_threshold < i){
        gc_period_float = (float)hyp_us / gc_period_ratio;
    	}
    	else{
        gc_period_float = (float)hyp_us * (int)(1.0/gc_period_ratio);
    	}
    	temp_gc_period = (int)gc_period_float;
    	write_util += (float)temp_gc_exec / (float)temp_gc_period;
		if(write_util >= util){
			printf("write util comp : %f, %f\n",write_util,util);
			max_wpage = i-1;
			break;
		}
	}//write bdwith calculated.
	for(int i=0;i<max_r;i++){
		read_util = 0.0;
		temp_r_exec = i * (DATA_TRANS * 2 + READ_LTN + DECOD_LTN);
        temp_r_period = hyp_us;
        read_util += (double)temp_r_exec / (double)temp_r_period;
		if(read_util >= util2){
			printf("Read util comp : %f, %f\n",read_util,util2);
			max_rpage = i-1;
			break;
		}
	}//read bdwith calculated.
	printf("wp : %d, rp : %d\n",max_wpage,max_rpage);
	throughputs[0] = (float)max_wpage * 8192 / 1024.0;
	throughputs[1] = (float)max_rpage * 8192 / 1024.0;
	
}

int test_PARTFTL(int task_num, task_info** task,FILE*fp, float* throughputs){
	
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
	int least_read_period, least_write_period;
	double gc_period_float;
	double gc_period_ratio;
    double temp_gc_threshold;

    for(i=0;i<task_num;i++){
        temp = task[i];
		task[i]->bin_alloc = 0; //since PaRT-FTL does not use bin packing approach, just allocate every task on bin 0.
        temp_r_exec = temp->read_num * (DATA_TRANS * 2 + READ_LTN + DECOD_LTN);
        temp_r_period = temp->read_period;
        read_util += (double)temp_r_exec / (double)temp_r_period;	
		printf("read_exec : %f, read_period : %f, read_util : %f\n",(double)temp_r_exec, (double)temp_r_period, (double)temp_r_exec / (double)temp_r_period );
        
		temp_w_exec = myceil((double)temp->write_num / (double)W_CHIP) * WRITE_LTN;
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
    //calculate the read_blocking and write_blocking.
	least_read_period = __INT_MAX__;
	least_write_period = __INT_MAX__;
	for(i=0;i<task_num;i++){
		if(least_read_period >= task[i]->read_period && (task[i]->read_period > 0)){
			least_read_period = task[i]->read_period;
		}		
		if(least_write_period >= task[i]->write_period && (task[i]->write_period > 0)){
			least_write_period = task[i]->write_period;
		}
	}
	read_util += (float)READ_LTN / (float)least_read_period;
	write_util += (float)ERASE_LTN / (float)least_write_period;
	printf("read_blocking : %f, write_blocking : %f\n",(float)READ_LTN / (float)least_read_period,
													   (float)ERASE_LTN / (float)least_write_period);
	//!calculate blocking.

	if((read_util > 1.0) || (write_util > 1.0)){
		printf("schedule failed, read : %f, write : %f\n",read_util, write_util);
		return -1;
	}
	else{
		printf("schedule successed, read : %f, write : %f\n",read_util, write_util);
		calc_PART_bandwidth(1,1.0 - write_util,1.0 - read_util, throughputs);
    	return 0;
	}

}

