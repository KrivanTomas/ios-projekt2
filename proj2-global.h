// proj2_global.h
// Global variables that all processes have (not shared)

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
//#include <linux/prctl.h>
#include <sys/prctl.h>
#include <unistd.h>

// 0: main, 1: ferry, 2: car, 4: truck
extern int process_type;

// set by the main process
extern pid_t parent_pid;
