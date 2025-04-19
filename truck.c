#include "truck-impl.h"

void truck_begin(int truck_id, int max_delay) {
    sync_child_death();
    process_type = 3; // truck
    srand(time(NULL) + truck_id);

    int destination = rand() % 2;

    struct sequence_counter *seq = open_seq();
    seq_printf(seq, "N %d: started %d\n", truck_id, destination);

    usleep((int)(rand() / RAND_MAX * max_delay));

    seq_printf(seq, "O %d: arrived to %d\n", truck_id, destination);

    exit(EXIT_SUCCESS);
}
