#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

    printf("Hello from main!\n");

    // fork ferry
    pid_t pid;
    if((pid = fork()) == 0) ferry_begin(ferry_capacity);
    else if(pid == -1) {
        perror("Failed to fork process (ferry)");
        exit(EXIT_FAILURE);
    }

    //return 0;

    // fork cars
    for(int i = 0; i < car_count; i++) {
        if((pid = fork()) == 0) car_begin();
        else if(pid == -1) {
            perror("Failed to fork process (car)");
            exit(EXIT_FAILURE);
        }
    }

    // fork trucks
    for(int i = 0; i < truck_count; i++) {
        if((pid = fork()) == 0) truck_begin();
        else if(pid == -1) {
            perror("Failed to fork process (car)");
            exit(EXIT_FAILURE);
        }
    }

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

