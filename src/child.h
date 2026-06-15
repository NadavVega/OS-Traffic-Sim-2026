#ifndef CHILD_H
#define CHILD_H
#include "graph.h"

// The main function that will be executed by each child process
void run_child_process(Graph *graph, int src, int dest, int write_fd);

#endif //CHILD_H
