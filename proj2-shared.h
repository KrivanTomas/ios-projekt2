// proj2-shared.h
// Variables shared in memory between all procesess
#ifndef PROJ2_SHARED_H_
#define PROJ2_SHARED_H_

#include "proj2.h"
#include <semaphore.h>


#define SEQUENCE_COUNTER_NAME "/seqcount"
struct sequence_counter {
    sem_t sem;
    int count;
};

void seq_printf(struct sequence_counter *seq, char *fmt, ...);
struct sequence_counter *open_seq();

#define DOCKS_NAME "/docks"
struct dock {
    struct {
        sem_t sem;
        int cars, trucks;
    } arrivals;
    struct {
        sem_t sem;
        int last_type;
    } boarding;
    struct {
        sem_t sem;
        int vehicles;
    } registered;
};

struct docks {
    size_t dock_count;
    struct dock arr[]; // pirates?
};

struct docks *open_docks();


#define FERRY_NAME "/ferry"
struct ferry {
    sem_t leave_sem;
    int capacity;
    struct {
        sem_t sem;
        int capacity_left;
    } info;
    sem_t disembark_sem;
};

struct ferry *open_ferry();

#endif
