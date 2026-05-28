#ifndef PARSER_H
#define PARSER_H

#include "graph.h"
#include <sys/types.h> // For pid_t type

// Data structure definition for a single traveler
typedef struct {
  int src;         // Source node
  int dest;        // Destination node
  pid_t pid;       // Child process ID, managed in milestone 4
  int *path;       // Shortest path from src to dest, calculated in milestone 3
  int path_length; // Length of the shortest path
  Color color;     // Unique color for visualization, assigned in milestone 4
  int current_path_index; // Current index in the path for animation, used in
                          // milestone 4
} Traveler;

// Updated function signature receiving a double pointer to the travelers array
// and a pointer to their count
Graph *parse_graph_from_file(const char *filename, Traveler **travelers,
                             int *num_travelers);

#endif // PARSER_H