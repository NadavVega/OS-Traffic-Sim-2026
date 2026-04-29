#include "dijkstra.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// Function to find the vertex with the minimum distance
int find_min_distance(int num_nodes, int dist[], int visited[]) {
  int min = INT_MAX;
  int min_index = -1;

  for (int i = 0; i < num_nodes; i++) {
    if (!visited[i] && dist[i] < min) {
      min = dist[i];
      min_index = i;
    }
  }
  return min_index;
}

dijkstraResult find_shortest_path(int num_nodes, int graph[15][15], int start,
                                  int end) {
  dijkstraResult result;
  int dist[15];
  int visited[15];
  int prev[15];

  // Initialize distances and visited array
  for (int i = 0; i < num_nodes; i++) {
    dist[i] = INT_MAX;
    visited[i] = 0;
    prev[i] = -1;
  }

  dist[start] = 0;

  // Dijkstra's algorithm
  for (int count = 0; count < num_nodes - 1; count++) {
    int u = find_min_distance(num_nodes, dist, visited);
    if (u == -1)
      break; // No more reachable vertices
    visited[u] = 1;

    // Update distances of adjacent vertices
    for (int v = 0; v < num_nodes; v++) {
      if (!visited[v] && graph[u][v] && dist[u] != INT_MAX &&
          dist[u] + graph[u][v] < dist[v]) {
        dist[v] = dist[u] + graph[u][v];
        prev[v] = u;
      }
    }
  }

  // Reconstruct the shortest path
  result.total_weight = dist[end];
  result.path_length = 0;

  if (dist[end] == INT_MAX) {
    // No path found
    result.path[0] = -1; // Indicate no path
    return result;
  }

  int temp_path[15];
  int index = 0;
  int curr = end;

  while (curr != -1) {
    temp_path[index++] = curr;
    curr = prev[curr];
  }

  // Reverse the path to get the correct order
  result.path_length = index;
  for (int i = 0; i < index; i++) {
    result.path[i] = temp_path[index - i - 1];
  }

  return result;
}