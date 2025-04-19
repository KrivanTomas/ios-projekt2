// impl2-shared.h
// Variables shared in memory between all procesess
#ifndef IMPL2_SHARED_H_
#define IMPL2_SHARED_H_

#include "proj2.h"
#include <semaphore.h>


#define SEQUENCE_COUNTER_NAME "/seqcount"
struct sequence_counter {
    sem_t sem;
    int count;
};

void seq_printf(struct sequence_counter *seq, char *fmt, ...);

struct sequence_counter *open_seq();

#endif
