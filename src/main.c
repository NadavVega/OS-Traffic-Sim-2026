#include "dijkstra.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  // Attempt to open the input file as specified in the project requirements
  FILE *file = fopen("input.txt", "r");
  if (!file) {
    perror("Error: Could not open input.txt");
    return 1;
  }

  int N, M;
  // Read the number of nodes (N) and edges (M) from the first line
  if (fscanf(file, "%d %d", &N, &M) != 2) {
    fclose(file);
    return 1;
  }

  // Initialize the adjacency matrix with -1 to represent no existing edge
  int graph[MAX_NODES][MAX_NODES];
  for (int i = 0; i < MAX_NODES; i++) {
    for (int j = 0; j < MAX_NODES; j++) {
      graph[i][j] = -1;
    }
  }

  // Read M edges from the file (src, dst, weight)
  for (int i = 0; i < M; i++) {
    int u, v, w;
    if (fscanf(file, "%d %d %d", &u, &v, &w) == 3) {
      graph[u][v] = w;
    }
  }

  // Read the final line containing the source and destination query
  int start_node, end_node;
  if (fscanf(file, "%d %d", &start_node, &end_node) != 2) {
    fclose(file);
    return 1;
  }
  fclose(file);

  // Execute the algorithm logic
  dijkstraResult result = find_shortest_path(N, graph, start_node, end_node);

  // Output handling based on project formatting rules
  // Output handling based on precise project formatting rules
  if (result.total_weight == -2) {
    // Error case: Negative weights (already handled inside dijkstra.c)
    return 1;
  }

  if (result.total_weight == -1) {
    // Requirement: Print specific message if no path exists
    printf("No path found\n");
  } else if (start_node == end_node) {
    // Requirement: If source equals destination, print 0 and 0 on separate
    // lines
    printf("0\n0\n");
  } else {
    // Requirement: Print path wrapped in $ with arrows (e.g., $0->2->1$)
    printf("$");
    for (int i = 0; i < result.path_length; i++) {
      printf("%d%s", result.path[i], (i == result.path_length - 1) ? "" : "->");
    }
    printf("$\n");

    // Requirement: Print the total accumulated weight on a new line
    printf("%d\n", result.total_weight);
  }

  return 0;
}