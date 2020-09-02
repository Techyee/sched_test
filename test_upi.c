int calc_UPI(int task_num, task_info* task, int chip){
	/* assuming a microbenchmark, all chips in same allocation will show same traffic..
	 * also, since UPI is basically a runtime algorithm, initialization can be quite naive.
	 */

	float util = 0.0;
	float traffic = 0.0;
	float write_amp = 0.0;
	int read_num_total = 0;
	int read_num_chip = 0;
	int gc_threshold;
	int i;
	float write_num_avg;
	for(i=0;i<task_num;i++){
		read_num_total += task[i]->read_num;
	}

	//calc some parameters.
	read_num_chip = read_num_total / chip;
	gc_threshold = (int)((1 - OP_RATE)*chip*PAGE_PER_BLOCK);
	write_amp = (float)(gc_threshold + (OP_RATE)*PAGE_PER_BLOCK*chip) / (float)gc_threshold; 


	//add read utilization.
	for(i=0;i<task_num;i++){
		traffic += task[i]->read_num / chip * READ_LTN / task[i]->read_period;
	}
	for(i=0;i<task_num;i++){
		traffic += task[i]->write_num * write_amp / chip * WRITE_LTN / task[i]->write_period;
	}

	//since we assume read/write will be evenly distributed to all chip, utilization of each chip is same.
	
	util = read_num_chip / (read_num_chip / (1 - traffic));




	

int test_UPI(task_info** task){
	/* 
     * 
     */ 
}

