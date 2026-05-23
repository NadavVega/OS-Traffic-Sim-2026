#ifndef PARSER_H
#define PARSER_H

#include "graph.h"
#include <sys/types.h> // For pid_t type

// Data structure definition for a single traveler
typedef struct {
    int src;   // Source node
    int dest;  // Destination node
    pid_t pid; // Child process ID, managed in milestone 4
} Traveler;

// Updated function signature receiving a double pointer to the travelers array and a pointer to their count
Graph *parse_graph_from_file(const char *filename, Traveler **travelers, int *num_travelers);

#endif // PARSER_H