#include "proj2-global.h"
#include "proj2-shared.h"
#include "ferry.h"

void ferry_begin(int capacity, int max_delay) {
    sync_child_death();
    process_type = TYPE_FERRY;
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

        // wait for all vehicles to disembark
        while(1) {
            if(sem_wait(&ferry->disembark_sem) == -1) errExit("sem_wait");
            if(sem_wait(&ferry->info.sem) == -1) errExit("sem_wait");
            if(ferry->info.capacity_left == ferry->capacity) break;
            if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");
            if(sem_post(&ferry->disembark_sem) == -1) errExit("sem_post");
        }
        if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");

        // check if there are any vehicles remaining
        if(sem_wait(&docks->arr[destination].registered.sem) == -1) errExit("sem_wait");
        if(docks->arr[destination].registered.vehicles == 0) {
             
        }
        if(sem_post(&docks->arr[destination].registered.sem) == -1) errExit("sem_post");

        // allow boarding
        if(sem_post(&docks->arr[destination].boarding.sem) == -1) errExit("sem_post");


        if(sem_wait(&ferry->leave_sem) == -1) errExit("sem_post");
        seq_printf(seq, "P: leaving %d\n", destination);

        destination = (destination + 1) % 2;

        if(sem_wait(&ferry->info.sem) == -1) errExit("sem_wait");
        if(ferry->info.capacity_left == ferry->capacity) break;
        if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");

    }

    usleep((int)(rand() / RAND_MAX * max_delay));
    seq_printf(seq, "P: finish");
    capacity++;
    exit(EXIT_SUCCESS);
}
