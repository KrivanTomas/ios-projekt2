#include "truck-impl.h"

void truck_begin(int truck_id, int max_delay, int destination) {
    sync_child_death();
    process_type = TYPE_TRUCK;
    srand(time(NULL) + truck_id);

    struct sequence_counter *seq = open_seq();
    seq_printf(seq, "N %d: started %d\n", truck_id, destination);

    usleep((int)(rand() / RAND_MAX * max_delay));

    seq_printf(seq, "N %d: arrived to %d\n", truck_id, destination);

    exit(EXIT_SUCCESS);
}
