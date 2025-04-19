// proj2_global.h
// Global variables that all processes have (not shared)
#ifndef PROJ2_GLOBAL_H_
#define PROJ2_GLOBAL_H_

#include "proj2.h"

// 0: main, 1: ferry, 2: car, 4: truck
extern int process_type;

// set by the main process
extern pid_t parent_pid;


void sync_child_death();

#endif
