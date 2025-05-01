#include "ferry-impl.h"

void ferry_begin(int capacity, int max_delay) {
    sync_child_death();
    process_type = 1; // ferry
    int destination = 0;
    srand(time(NULL));


    struct sequence_counter *seq = open_seq();
    struct docks *docks = open_docks();
    struct ferry *ferry = open_ferry();
    seq_printf(seq, "P: started\n");

    while(1) {
        usleep((int)(rand() / RAND_MAX * max_delay));
        seq_printf(seq, "P: arrived to %d\n", destination);

        // allow disembark
        if(sem_post(&ferry->disembark_sem) == -1) errExit("sem_post");
        printf("i dont know how semaphores work :) \n");
        // wait for all vehicles to disembark
        if(sem_wait(&ferry->disembark_sem) == -1) errExit("sem_wait");

        // allow boarding
        if(sem_post(&docks->arr[destination].boarding.sem) == -1) errExit("sem_post");

        
        if(sem_wait(&ferry->leave_sem) == -1) errExit("sem_post");
        seq_printf(seq, "P: leaving %d\n", destination);

        destination = (destination + 1) % 2;
    }

    usleep((int)(rand() / RAND_MAX * max_delay));
    seq_printf(seq, "P: finish");
    capacity++;
    exit(EXIT_SUCCESS);
}
