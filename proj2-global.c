#include "proj2-global.h"

void sync_child_death() {
    // recieve death signal from parent
    int err = prctl(PR_SET_PDEATHSIG, SIGHUP);
    if(err == -1) exit(EXIT_FAILURE);
    
    // check that the parent is still the same (race condition fix)
    if(getppid() != parent_pid) exit(EXIT_FAILURE);
}
