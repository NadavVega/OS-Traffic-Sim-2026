#ifndef CHILD_H
#define CHILD_H

#ifndef MILESTONE
#define MILESTONE 4
#endif

#if MILESTONE == 5
#include "graph.h"

void run_child_process(Graph *graph, int src, int dest, int write_fd);
#elif MILESTONE >= 6
#include "graph.h"

void run_child_process(Graph *graph, int src, int dest, int write_fd,
                       int semaphore_id);
#else
// The main function that will be executed by each child process
void run_child_process(void);
#endif

#endif // CHILD_H
