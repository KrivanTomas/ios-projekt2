#include "proj2-shared.h"
#include <stdarg.h>
#include <string.h>

void seq_printf(struct sequence_counter* seq, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    if(sem_wait(&seq->sem) == -1) errExit("sem_wait");

    // 200 is a random large enough number for %d + '\0'
    size_t buffer_size = strlen(fmt) + 200;
    char *buffer = malloc(buffer_size);
    if (buffer == NULL) errExit("malloc");

    sprintf(buffer, "%d: %s", seq->count, fmt);
    vprintf(buffer, args);
    free(buffer);

    seq->count++;

    if(sem_post(&seq->sem) == -1) errExit("sem_post");
}

struct sequence_counter *open_seq() {
    int fd = shm_open(SEQUENCE_COUNTER_NAME, O_RDWR, 0);
    if(fd == -1) errExit("shm_open");

    struct sequence_counter *seq;
    seq = mmap(NULL, sizeof(*seq), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(seq == MAP_FAILED) errExit("mmap");

    return seq;
}

struct docks *open_docks() {
    int fd = shm_open(DOCKS_NAME, O_RDWR, 0);
    if(fd == -1) errExit("shm_open");

    struct docks *docks;
    docks = mmap(NULL, sizeof(*docks) + sizeof(struct dock) * DOCK_COUNT, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(docks == MAP_FAILED) errExit("mmap");

    return docks;
}

struct ferry *open_ferry() {
    int fd = shm_open(FERRY_NAME, O_RDWR, 0);
    if(fd == -1) errExit("shm_open");

    struct ferry *ferry;
    ferry = mmap(NULL, sizeof(*ferry), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ferry == MAP_FAILED) errExit("mmap");

    return ferry;
}

