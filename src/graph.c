//
// Created by balah on 09/05/2026.
//
#include <math.h>
#include <stdlib.h>

#include "graph.h"

//=====================================================
// create_graph:
// Receives nodes and edges.
// Allocates Graph.
// Initializes matrix to -1.
// Returns Graph* or NULL.

// add_edge:
// Receives graph, src, dest, weight.
// Adds directed weighted edge.
// Returns nothing.

// free_graph:
// Receives Graph*.
// Frees memory.
//=====================================================

// Data structures and memory management

Graph *create_graph(int nodes, int edges) {
  // Verify node limit
  if (nodes <= 0 || nodes > MAX_NODES)
    return NULL;

  Graph *g = (Graph *)malloc(sizeof(Graph));
  if (g == NULL)
    return NULL;

  g->num_nodes = nodes;
  g->num_edges = edges;

  // Initialize matrix with -1 (indicates "no edge")
  for (int i = 0; i < MAX_NODES; i++) {
    for (int j = 0; j < MAX_NODES; j++) {
      g->matrix[i][j] = -1;
    }
  }
  return g;
}

void add_edge(Graph *g, int src, int dest, int weight) {
  // Validate input and check for negative weights
  if (g == NULL || src < 0 || src >= g->num_nodes || dest < 0 ||
      dest >= g->num_nodes || weight < 0) {
    return;
  }
  g->matrix[src][dest] = weight;
}

void free_graph(Graph *g) {
  if (g != NULL) {
    free(g); // Free memory as required
  }
}

// Graphical infrastructure

void calculate_node_positions(Graph *g, int screen_width, int screen_height) {
  if (g == NULL || g->num_nodes <= 0)
    return;

  int center_x = screen_width / 2;
  int center_y = screen_height / 2;
  int radius = (screen_height / 2) - 60; // Circle radius

  // Distribute nodes in a circle to prevent overlap and improve readability
  for (int i = 0; i < g->num_nodes; i++) {
    // Using PI constant from raylib.h
    float angle = (2.0f * PI * i) / (float)g->num_nodes;
    g->node_positions[i].x = center_x + (int)(radius * cos(angle));
    g->node_positions[i].y = center_y + (int)(radius * sin(angle));
  }
}

// Animation state management

AnimationState *init_animation(int *path, int path_length) {
  if (path == NULL || path_length <= 0)
    return NULL;

  AnimationState *anim = (AnimationState *)malloc(sizeof(AnimationState));
  if (anim == NULL)
    return NULL;

  // Copy the path into the animation structure
  anim->path = (int *)malloc(path_length * sizeof(int));
  if (anim->path == NULL) {
    free(anim);
    return NULL;
  }
  for (int i = 0; i < path_length; i++)
    anim->path[i] = path[i];

  anim->path_length = path_length;
  anim->current_path_index = 0;
  anim->current_jump = 0;
  anim->timer = 0.0f;
  anim->is_playing = false; // Initial state - Play/Stop
  anim->status = ANIM_IDLE;

  return anim;
}

void free_animation(AnimationState *anim) {
  if (anim != NULL) {
    if (anim->path != NULL)
      free(anim->path);
    free(anim);
  }
}