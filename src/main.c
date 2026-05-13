#include "dijkstra.h"
#include "graph.h"
#include "gui.h" // Nave's functions
#include "parser.h"
#include "raylib.h" // Graphics library
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  // ==========================================
  // 1. Data Parsing and Validation (Bar's Logic)
  // ==========================================
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int src, dest;
  Graph *graph = parse_graph_from_file(argv[1], &src, &dest);

  if (graph == NULL) {
    fprintf(stderr, "Error: Invalid input or negative weights detected.\n");
    return EXIT_FAILURE;
  }

  if (src < 0 || src >= graph->num_nodes || dest < 0 ||
      dest >= graph->num_nodes) {
    fprintf(stderr, "Error: Source or destination node out of bounds.\n");
    free_graph(graph);
    return EXIT_FAILURE;
  }

  // Run Dijkstra
  dijkstraResult result =
      find_shortest_path(graph->num_nodes, graph->matrix, src, dest);
  bool has_path = (result.total_weight >= 0);

  // ==========================================
  // 2. Integration of Nave's GUI (Restored Fixes)
  // ==========================================

  // Open graphical window with expanded dimensions for 15 nodes
  InitWindow(1000, 800, "Traffic Simulation 2026 - Milestone 3");
  SetTargetFPS(60);

  VisualNode vNodes[MAX_NODES];
  InitGraphVisuals(graph->num_nodes, vNodes);

  Entity car = {0};
  bool animationRunning = false; // Required for PLAY/STOP functionality

  if (has_path && result.path_length > 0) {
    car.currentPos = vNodes[result.path[0]].pos;
    car.startNode = 0;
    car.endNode = 1;
  }

  Rectangle buttonBounds = {20, 100, 120, 40};

  // Main game loop
  while (!WindowShouldClose()) {
    // Update movement logic only if PLAY is active
    if (animationRunning && has_path && car.endNode < result.path_length) {
      UpdateEntity(&car, graph->num_nodes, vNodes, graph->matrix, result.path,
                   result.path_length);
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw background graph
    DrawStaticGraph(graph->num_nodes, vNodes, graph->matrix);

    // UI Information (Restored -1 display logic)
    DrawText("Milestone 3: Traffic Animation", 20, 20, 20, DARKGRAY);
    DrawText(TextFormat("Path: %d -> %d (Weight: %d)", src, dest,
                        result.total_weight),
             20, 50, 18, has_path ? MAROON : RED);

    if (!has_path) {
      DrawText("ERROR: NO PATH FOUND (-1)", 20, 80, 18, RED);
    }

    // Interactive Play/Stop Button
    if (DrawButton(buttonBounds, animationRunning ? "STOP" : "PLAY",
                   animationRunning)) {
      animationRunning = !animationRunning;
    }

    // Draw entity and completion message
    if (has_path) {
      DrawEntity(car);

      if (car.endNode >= result.path_length) {
        DrawText("DESTINATION REACHED!", 350, 400, 30, LIME);
        animationRunning = false;
      }
    }

    EndDrawing();
  }

  // ==========================================
  // 3. Clean Memory
  // ==========================================
  free_graph(graph);
  CloseWindow();

  return EXIT_SUCCESS;
}