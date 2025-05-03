#include "proj2-global.h"
#include "proj2-shared.h"
#include "car.h"

void car_begin(int car_id, int max_delay, int destination) {
    sync_child_death();
    process_type = TYPE_CAR;
    srand(time(NULL) + car_id);
    
    // open shared memory
    struct sequence_counter *seq = open_seq();
    struct docks *docks = open_docks();
    struct ferry *ferry = open_ferry();
    seq_printf(seq, "O %d: started\n", car_id);

    usleep((int)(rand() / RAND_MAX * max_delay));

    // register car arrival
    seq_printf(seq, "O %d: arrived to %d\n", car_id, destination);
    
    while(1) {
        // (try to) board ferry
        if(sem_wait(&docks->arr[destination].boarding.sem) == -1) errExit("sem_wait");
        if(sem_wait(&docks->arr[destination].arrivals.sem) == -1) errExit("sem_wait");
        if(sem_wait(&ferry->info.sem) == -1) errExit("sem_wait");

        if(docks->arr[destination].arrivals.trucks != 0
          && docks->arr[destination].boarding.last_type == TYPE_CAR
          && ferry->info.capacity_left - TRUCK_SIZE >= 0) { 
            // let the truck pass (and try again)
            if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");
            if(sem_post(&docks->arr[destination].arrivals.sem) == -1) errExit("sem_post");
            if(sem_post(&docks->arr[destination].boarding.sem) == -1) errExit("sem_post");
            continue;
        }
        // board the ferry (for real this time)
        docks->arr[destination].arrivals.cars--;

        docks->arr[destination].boarding.last_type = TYPE_CAR;
        
        seq_printf(seq, "O %d: boarding\n", car_id);
        ferry->info.capacity_left--;

        if(ferry->info.capacity_left == 0
          || (docks->arr[destination].arrivals.cars == 0 && docks->arr[destination].arrivals.trucks == 0)
          || (docks->arr[destination].arrivals.cars == 0 && ferry->info.capacity_left < TRUCK_SIZE)) {
            if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");
            if(sem_post(&docks->arr[destination].arrivals.sem) == -1) errExit("sem_post");

            // tell the ferry to leave
            if(sem_post(&ferry->leave_sem) == -1) errExit("sem_post");
            destination = (destination + 1) % 2;
            break;
        }


        if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");
        if(sem_post(&docks->arr[destination].arrivals.sem) == -1) errExit("sem_post");
        if(sem_post(&docks->arr[destination].boarding.sem) == -1) errExit("sem_post");

        destination = (destination + 1) % 2;
        break;
    }

    // wait for ferry to arrive and leave

    if(sem_wait(&ferry->disembark_sem) == -1) errExit("sem_wait");
    if(sem_wait(&ferry->info.sem) == -1) errExit("sem_wait");

    ferry->info.capacity_left++;
    seq_printf(seq, "O %d: leaving in %d\n", car_id, destination);

    if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");
    if(sem_post(&ferry->disembark_sem) == -1) errExit("sem_post");


    exit(EXIT_SUCCESS);
}
