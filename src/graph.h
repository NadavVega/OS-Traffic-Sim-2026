//
// Created by balah on 09/05/2026.
//
#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

#include "raylib.h"

#define MAX_NODES 15 // Node limit

// Helper structure to store node coordinates on the screen
typedef struct {
  int x;
  int y;
} Point2D;

// Graph data structure definition
typedef struct {
  int num_nodes;
  int num_edges;
  int matrix[MAX_NODES][MAX_NODES];  // Adjacency matrix for edge weights
  Point2D node_positions[MAX_NODES]; // Node positions for graphical display
} Graph;

// Possible animation states of the moving entity
typedef enum {
  ANIM_IDLE,         // Animation hasn't started yet or is completely stopped
  ANIM_WAITING_NODE, // Entity waits for one full second at a node
  ANIM_MOVING_EDGE,  // Entity is currently moving on an edge
  ANIM_FINISHED // Entity reached the destination (to display the message on
                // screen)
} AnimStatus;

// Data structure managing the moving entity on the graph
typedef struct {
  int *path;       // The path (copied from Dijkstra's result)
  int path_length; // Number of nodes in the path

  int current_path_index; // Where the entity is currently located (index in the
                          // path array)
  int current_jump; // Jump counter on the current edge (out of edge weight W)

  float timer; // Internal accumulator timer to track 300ms (jump) or 1 second
               // (wait)

  bool is_playing;   // Flag to control the Play/Stop button
  AnimStatus status; // Current entity state
} AnimationState;

// --- Basic graph management ---
Graph *create_graph(int nodes, int edges);
void add_edge(Graph *g, int src, int dest, int weight);
void free_graph(Graph *g);

// Function to calculate and distribute nodes in space so they don't overlap
void calculate_node_positions(Graph *g, int screen_width, int screen_height);

// Allocates the animation structure and initializes it with the found path
AnimationState *init_animation(int *path, int path_length);

// Frees animation memory at the end to prevent memory leaks
void free_animation(AnimationState *anim);

#endif