#include "car-impl.h"
#include "math.h"
void car_begin(int car_id, int max_delay) {
    sync_child_death();
    process_type = 2; // car
    srand(time(NULL) + car_id);
    
    int destination = rand() % 2;

    struct sequence_counter *seq = open_seq();
    seq_printf(seq, "O %d: started %d\n", car_id, destination);

    usleep((int)(rand() / RAND_MAX * max_delay));

    seq_printf(seq, "O %d: arrived to %d\n", car_id, destination);

    max_delay++;
    exit(EXIT_SUCCESS);
}
