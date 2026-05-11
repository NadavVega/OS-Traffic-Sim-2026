#ifndef DIJKSTRA_H
#define DIJKSTRA_H
#define MAX_NODES 15
typedef struct {
  int path[MAX_NODES];
  int path_length;
  int total_weight;
} dijkstraResult;

dijkstraResult find_shortest_path(int num_nodes,
                                  int graph[MAX_NODES][MAX_NODES], int start,
                                  int end);

#endif // DIJKSTRA_H