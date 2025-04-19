#include "ferry-impl.h"

void ferry_begin(int capacity, int max_delay) {
    sync_child_death();
    process_type = 1; // ferry
    int destination = 0;
    srand(time(NULL));


    struct sequence_counter *seq = open_seq();
    seq_printf(seq, "P: started\n");

    usleep((int)(rand() / RAND_MAX * max_delay));

    seq_printf(seq, "P: arrived to %d\n", destination);

    capacity++;
    exit(EXIT_SUCCESS);
}
