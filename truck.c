#include "truck-impl.h"

void truck_begin() {
    // recieve death signal from parent
    int err = prctl(PR_SET_PDEATHSIG, SIGHUP);
    if(err == -1) exit(EXIT_FAILURE);
    
    // check that the parent is still the same (race condition fix)
    if(getppid() != parent_pid) exit(EXIT_FAILURE);

    process_type = 3; // truck
    printf("Hello from truck!\n");
    exit(EXIT_SUCCESS);
}
