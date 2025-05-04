#include <unistd.h>
#include <sys/wait.h>

#include "proj2.h"
#include "proj2-shared.h"
#include "proj2-global.h"
#include "ferry.h"
#include "car.h"
#include "truck.h"


#define DOCK_COUNT 2

// quick debug fix
#undef O_EXCL
#define O_EXCL 0

#define MASK_SEQUENCE_COUNTER 1
#define MASK_DOCKS 2
#define MASK_FERRY 4
int unlink_mask = 0;

int process_type = TYPE_MAIN;

// used in child procesess
pid_t parent_pid = 0;

void main_begin(int truck_count, int car_count, int ferry_capacity, int max_car_time, int max_ferry_time);
void print_usage(FILE *file);
void exit_unlink_mem();
void close_output();

int main(int argc, char **argv) {
    if(argc != 6) {
        print_usage(stderr);
        return 1;
    }

    // parse the input numbers
    int truck_count = atoi(argv[1]);
    int car_count = atoi(argv[2]);
    int ferry_capacity = atoi(argv[3]);
    int max_car_time = atoi(argv[4]);
    int max_ferry_time = atoi(argv[5]);

    // test input ranges
    // some won't catch parse errors (atoi returns 0 on error)
    if(truck_count < 0 || truck_count >= 10000) {
        fprintf(stderr, "Please enter N as a valid number in between <0;10000)\n");
        return 1;
    }
    if(car_count < 0 || car_count >= 10000) {
        fprintf(stderr, "Please enter O as a valid number in between <0;10000)\n");
        return 1;
    }
    if(ferry_capacity < 3 || ferry_capacity > 100) {
        fprintf(stderr, "Please enter K as a valid number in between <3;100>\n");
        return 1;
    }
    if(max_car_time < 0 || max_car_time > 10000) {
        fprintf(stderr, "Please enter TA as a valid number in between <0;10000>\n");
        return 1;
    }
    if(max_ferry_time < 0 || max_ferry_time > 1000) {
        fprintf(stderr, "Please enter TP as a valid number in between <0;1000>\n");
        return 1;
    }

    if((output_fd = fopen("proj2.out", "w"))== NULL) errExit("fopen");
    atexit(close_output);

    main_begin(truck_count, car_count, ferry_capacity, max_car_time, max_ferry_time);

    return 0;
}

void main_begin(int truck_count, int car_count, int ferry_capacity, int max_car_time, int max_ferry_time) {
    // used in child procesess
    parent_pid = getpid();
    atexit(exit_unlink_mem);

    time_t rand_seed = time(NULL);
    srand(rand_seed);

    // share sequence counter structure
    int fd;
    struct sequence_counter *seq_counter;

    fd = shm_open(SEQUENCE_COUNTER_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if(fd == -1) errExit("shm_open");
    unlink_mask |= MASK_SEQUENCE_COUNTER;
    if(ftruncate(fd, sizeof(struct sequence_counter)) == -1) errExit("ftruncate");

    seq_counter = mmap(NULL, sizeof(*seq_counter), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(seq_counter == MAP_FAILED) errExit("mmap");
    
    if(sem_init(&seq_counter->sem, 1, 0) == -1) errExit("sem_init");
    // set default values
    seq_counter->count = 1;
    
    if(sem_post(&seq_counter->sem) == -1) errExit("sem_post");

    // share docks structure
    struct docks *docks;
    fd = shm_open(DOCKS_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if(fd == -1) errExit("shm_open");
    unlink_mask |= MASK_DOCKS;
    if(ftruncate(fd, sizeof(struct docks) + sizeof(struct dock) * DOCK_COUNT) == -1) errExit("ftruncate");

    docks = mmap(NULL, sizeof(*docks) + sizeof(struct dock) * DOCK_COUNT, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(docks == MAP_FAILED) errExit("mmap");
    
    // count the cars and truck in each dock before forking
    for(int i = 0; i < DOCK_COUNT; i++) {
        docks->arr[i].arrivals.cars = 0;
        docks->arr[i].arrivals.trucks = 0;
    }
    for(int i = 0; i < car_count; i++) {
        docks->arr[rand() % DOCK_COUNT].arrivals.cars++;
    }
    for(int i = 0; i < truck_count; i++) {
        docks->arr[rand() % DOCK_COUNT].arrivals.trucks++;
    }
    srand(rand_seed); // reset rand() so the results are the same
    
    docks->dock_count = DOCK_COUNT;
    for(size_t i = 0; i < docks->dock_count; i++) {
        docks->arr[i].boarding.last_type = TYPE_INIT;
        if(sem_init(&docks->arr[i].arrivals.sem, 1, 1) == -1) errExit("sem_init");
        if(sem_init(&docks->arr[i].boarding.sem, 1, 0) == -1) errExit("sem_init");
    }


    // share ferry structure
    struct ferry *ferry;
    fd = shm_open(FERRY_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if(fd == -1) errExit("shm_open");
    unlink_mask |= MASK_FERRY;
    if(ftruncate(fd, sizeof(struct ferry)) == -1) errExit("ftruncate");

    ferry = mmap(NULL, sizeof(*ferry), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ferry == MAP_FAILED) errExit("mmap");

    ferry->capacity = ferry_capacity; 
    ferry->info.capacity_left = ferry_capacity;

    if(sem_init(&ferry->leave_sem, 1, 0) == -1) errExit("sem_init");
    if(sem_init(&ferry->info.sem, 1, 1) == -1) errExit("sem_init");
    if(sem_init(&ferry->disembark_sem, 1, 0) == -1) errExit("sem_init");
    

    // fork ferry
    pid_t pid;
    if((pid = fork()) == 0) ferry_begin(max_ferry_time);
    else if(pid == -1) errExit("fork");

    //return 0;

    // fork cars
    for(int i = 0; i < car_count; i++) {
        int starting_destination = rand() % 2;
        if((pid = fork()) == 0) car_begin(i + 1, max_car_time, starting_destination);
        else if(pid == -1) errExit("fork");
    }

    // fork trucks
    for(int i = 0; i < truck_count; i++) {
        int starting_destination = rand() % 2;
        if((pid = fork()) == 0) truck_begin(i + 1, max_car_time, starting_destination);
        else if(pid == -1) errExit("fork");
    }


    // wait for all child processes to end
    while(wait(NULL) > 0);

    // unlink shared memory
    sem_destroy(&seq_counter->sem);
    shm_unlink(SEQUENCE_COUNTER_NAME);
    for(size_t i = 0; i < docks->dock_count; i++) {
        sem_destroy(&docks->arr[i].arrivals.sem);
        sem_destroy(&docks->arr[i].boarding.sem);
    }
    shm_unlink(DOCKS_NAME);
    
    sem_destroy(&ferry->leave_sem);
    sem_destroy(&ferry->disembark_sem);
    sem_destroy(&ferry->info.sem);
    shm_unlink(FERRY_NAME);
}

void exit_unlink_mem() {
    if(process_type != TYPE_MAIN) return;

    if(unlink_mask & MASK_SEQUENCE_COUNTER) shm_unlink(SEQUENCE_COUNTER_NAME);
    if(unlink_mask & MASK_DOCKS) shm_unlink(DOCKS_NAME);
    if(unlink_mask & MASK_FERRY) shm_unlink(FERRY_NAME);
}

void close_output() {
    fclose(output_fd);
}

void print_usage(FILE *file) {
    fprintf(file, "Please input the corect number of arguments\n");
    fprintf(file, "\n");
    fprintf(file, "usage:\tproj2 [N] [O] [K] [TA] [TP]\n");
    fprintf(file, "\targ\tfull name\tlimits\n");
    fprintf(file, "\tN:\tNákladní auto\tN<10000\n");
    fprintf(file, "\tO:\tOsobní auto\tO<10000\n");
    fprintf(file, "\tK:\tKapacita přívozu\t3<=K<=100\n");
    fprintf(file, "\tTA:\tMaximální doba jízdy auta(μs)\t0<=TA<=10000\n");
    fprintf(file, "\tTP:\tMaximální doba jízdy přívozu(μs)\t0<=TP<=1000\n");
}

