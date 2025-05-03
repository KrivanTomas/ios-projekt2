#include "proj2-global.h"
#include "proj2-shared.h"
#include "truck.h"

void truck_begin(int truck_id, int max_delay, int destination) {
    sync_child_death();
    process_type = TYPE_TRUCK;
    srand(time(NULL) + truck_id);

    struct sequence_counter *seq = open_seq();
    struct docks *docks = open_docks();
    struct ferry *ferry = open_ferry();

    seq_printf(seq, "N %d: started\n", truck_id);

    usleep((int)(rand() / RAND_MAX * max_delay));

    // register truck arrival
    if(sem_wait(&docks->arr[destination].arrivals.sem) == -1) errExit("sem_wait");
    
    docks->arr[destination].arrivals.trucks++;
    seq_printf(seq, "N %d: arrived to %d\n", truck_id, destination);

    if(sem_post(&docks->arr[destination].arrivals.sem) == -1) errExit("sem_post");

    while(1) {
        // (try to) board ferry
        if(sem_wait(&docks->arr[destination].boarding.sem) == -1) errExit("sem_wait");
        if(sem_wait(&docks->arr[destination].arrivals.sem) == -1) errExit("sem_wait");
        if(sem_wait(&ferry->info.sem) == -1) errExit("sem_wait");
        if(sem_wait(&docks->arr[destination].registered.sem) == -1) errExit("sem_wait");

        if(ferry->info.capacity_left < TRUCK_SIZE
          || (docks->arr[destination].boarding.last_type == TYPE_TRUCK
             && docks->arr[destination].arrivals.cars != 0)) {
            // let the car pass (and try again)
            if(sem_post(&docks->arr[destination].registered.sem) == -1) errExit("sem_post");
            if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");
            if(sem_post(&docks->arr[destination].arrivals.sem) == -1) errExit("sem_post");
            if(sem_post(&docks->arr[destination].boarding.sem) == -1) errExit("sem_post");
            continue;
        }
        // board the ferry (for real this time)
        docks->arr[destination].arrivals.trucks--;
        docks->arr[destination].boarding.last_type = TYPE_TRUCK;
        docks->arr[destination].registered.vehicles--;

        seq_printf(seq, "N %d: boarding\n", truck_id);
        ferry->info.capacity_left -= TRUCK_SIZE;

        if(docks->arr[destination].registered.vehicles == 0
          || ferry->info.capacity_left == 0
          || (docks->arr[destination].arrivals.cars == 0 && docks->arr[destination].arrivals.trucks == 0)
          || (docks->arr[destination].arrivals.cars == 0 && ferry->info.capacity_left < TRUCK_SIZE)) {
            if(sem_post(&docks->arr[destination].registered.sem) == -1) errExit("sem_post");
            if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");
            if(sem_post(&docks->arr[destination].arrivals.sem) == -1) errExit("sem_post");

            // tell the ferry to leave
            if(sem_post(&ferry->leave_sem) == -1) errExit("sem_post");
            destination = (destination + 1) % 2;
            break;
        }

        if(sem_post(&docks->arr[destination].registered.sem) == -1) errExit("sem_post");
        if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");
        if(sem_post(&docks->arr[destination].arrivals.sem) == -1) errExit("sem_post");
        if(sem_post(&docks->arr[destination].boarding.sem) == -1) errExit("sem_post");

        destination = (destination + 1) % 2;
        break;
    }

    // wait for ferry to arrive and leave

    if(sem_wait(&ferry->disembark_sem) == -1) errExit("sem_wait");
    if(sem_wait(&ferry->info.sem) == -1) errExit("sem_wait");

    ferry->info.capacity_left += 3;
    seq_printf(seq, "N %d: leaving in %d\n", truck_id, destination);

    if(sem_post(&ferry->info.sem) == -1) errExit("sem_post");
    if(sem_post(&ferry->disembark_sem) == -1) errExit("sem_post");
    exit(EXIT_SUCCESS);
}
