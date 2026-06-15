#ifndef CHILD_H
#define CHILD_H
#include "graph.h"

void run_child_process(Graph *graph, int src, int dest, int write_fd);
#endif //CHILD_H
