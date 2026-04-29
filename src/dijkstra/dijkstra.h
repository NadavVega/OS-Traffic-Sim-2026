#ifndef DIJKSTRA_H
#define DIJKSTRA_H

typedef struct {
  int path[15];
  int path_length;
  int total_weight;
} dijkstraResult;

dijkstraResult find_shortest_path(int num_nodes, int graph[15][15], int start,
                                  int end);

#endif // DIJKSTRA_H