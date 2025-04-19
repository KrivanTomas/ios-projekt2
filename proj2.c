#include <unistd.h>

#include "proj2.h"
#include "proj2-shared.h"
#include "proj2-global.h"
#include "ferry.h"
#include "car.h"
#include "truck.h"

// 0: main, 1: ferry, 2: car, 4: truck
int process_type = 0;

// used in child procesess
pid_t parent_pid = 0;

void main_begin(int truck_count, int car_count, int ferry_capacity, int max_car_time, int max_ferry_time);
void print_usage(FILE *file);

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


    main_begin(truck_count, car_count, ferry_capacity, max_car_time, max_ferry_time);

    return 0;
}

void main_begin(int truck_count, int car_count, int ferry_capacity, int max_car_time, int max_ferry_time) {
    // used in child procesess
    parent_pid = getpid();

    // share sequence counter
    
    int fd;
    struct sequence_counter *seq_counter;

    fd = shm_open(SEQUENCE_COUNTER_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if(fd == -1) errExit("shm_open");
    if(ftruncate(fd, sizeof(struct sequence_counter)) == -1) errExit("ftruncate");

    seq_counter = mmap(NULL, sizeof(*seq_counter), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(seq_counter == MAP_FAILED) errExit("mmap");
    
    if(sem_init(&seq_counter->sem, 1, 0) == -1) errExit("sem_init:");

    seq_counter->count = 1;
    
    if(sem_post(&seq_counter->sem) == -1) errExit("sem_post");

    seq_printf(seq_counter, "Hello from main\n");

    // fork ferry
    pid_t pid;
    if((pid = fork()) == 0) ferry_begin(ferry_capacity, max_ferry_time);
    else if(pid == -1) errExit("fork");

    //return 0;

    // fork cars
    for(int i = 0; i < car_count; i++) {
        if((pid = fork()) == 0) car_begin(i + 1, max_car_time);
        else if(pid == -1) errExit("fork");
    }

    // fork trucks
    for(int i = 0; i < truck_count; i++) {
        if((pid = fork()) == 0) truck_begin(i + 1, max_car_time);
        else if(pid == -1) errExit("fork");
    }


    // unlink shared memory
    sleep(1);
    shm_unlink(SEQUENCE_COUNTER_NAME);

    // temporary unesed warning fix
    max_car_time++;
    max_ferry_time++;
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

