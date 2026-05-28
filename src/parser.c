#include "parser.h"
#include "gui.h" // For travelerColors array
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Graph *parse_graph_from_file(const char *filename, Traveler **travelers,
                             int *num_travelers) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    perror("Error opening file");
    return NULL;
  }

  int nodes, edges;
  // Read the first line: number of nodes and edges
  if (fscanf(fp, "%d %d", &nodes, &edges) != 2) {
    fclose(fp);
    return NULL;
  }

  // Create the graph infrastructure in memory
  Graph *g = create_graph(nodes, edges);
  if (g == NULL) {
    fclose(fp);
    return NULL;
  }

  // Read the edges and their weights
  for (int i = 0; i < edges; i++) {
    int u, v, w;
    if (fscanf(fp, "%d %d %d", &u, &v, &w) == 3) {
      // Validate negative weights
      if (w < 0) {
        free_graph(g);
        fclose(fp);
        return NULL;
      }
      add_edge(g, u, v, w);
    }
  }

  // Step A: Scan the file until the #travelers tag is found
  char token[100];
  bool found_travelers = false;
  while (fscanf(fp, "%99s", token) == 1) {
    if (strcmp(token, "#travelers") == 0) {
      found_travelers = true;
      break;
    }
  }

  // If the tag is not found in the file, clean memory and return an error
  if (!found_travelers) {
    free_graph(g);
    fclose(fp);
    return NULL;
  }

  // Step B: Read the total number of travelers
  if (fscanf(fp, "%d", num_travelers) != 1) {
    free_graph(g);
    fclose(fp);
    return NULL;
  }

  // Step C: Dynamic memory allocation for the travelers array based on the read
  // count
  *travelers = (Traveler *)malloc((*num_travelers) * sizeof(Traveler));
  if (*travelers == NULL) {
    free_graph(g);
    fclose(fp);
    return NULL;
  }

  // Step D: Loop to read the source and destination pairs for each traveler
  for (int i = 0; i < *num_travelers; i++) {
    if (fscanf(fp, "%d %d", &((*travelers)[i].src), &((*travelers)[i].dest)) !=
        2) {
      free(*travelers);
      free_graph(g);
      fclose(fp);
      return NULL;
    }
    // Initialize default value for PID to prevent garbage values in memory
    (*travelers)[i].pid = 0;
    (*travelers)[i].path = NULL;            // Initialize path pointer to NULL
    (*travelers)[i].path_length = 0;        // Initialize path length to 0
    (*travelers)[i].current_path_index = 0; // Initialize path index to 0
    (*travelers)[i].color = travelerColors[i % 15]; // Assign unique color
  }

  fclose(fp);
  return g;
}