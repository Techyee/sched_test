#include <sched_simul.h>

//parameters
PAGE_PER_BLOCK = 128;
OP_RATE = 25;
CHANNEL_NB = 4;
WAY_NB = 4;
READ_LTN = 50;
WRITE_LTN = 500;
ERASE_LTN = 5000;
DATA_TRANS = 40;

//!parameters

int taskinfo_generation(task_info* res, double util){
	/* making a task_info structure based on random.
	   utilization is given, and read/write amount is selected as random.
	   garbage collection overhead is accounted, and task_info.task_util is
	   summation of read util, write util and garbage collection util.
	 */
	 
}	

int taskset_generation(int task_num, double util, task_info* info){
		

	

