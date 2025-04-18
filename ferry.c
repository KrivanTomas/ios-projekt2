#include "ferry-impl.h"

void ferry_begin(int capacity) {
    // recieve death signal from parent
    int err = prctl(PR_SET_PDEATHSIG, SIGHUP);
    if(err == -1) exit(EXIT_FAILURE);
    
    // check that the parent is still the same (race condition fix)
    if(getppid() != parent_pid) exit(EXIT_FAILURE);

    process_type = 1; // ferry
    printf("Hello from ferry, capacity: %d\n", capacity);
    exit(EXIT_SUCCESS);
}
