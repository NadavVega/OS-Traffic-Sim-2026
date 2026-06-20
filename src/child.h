#ifndef CHILD_H
#define CHILD_H

#ifndef MILESTONE
#define MILESTONE 4
#endif

#if MILESTONE >= 5
#include "graph.h"
#include <semaphore.h>

// Updated to receive the sem_t array from Binyamin's API
void run_child_process(Graph *graph, int src, int dest, int write_fd, sem_t *semaphores);
#else
// The main function that will be executed by each child process
void run_child_process(void);
#endif

#endif // CHILD_H