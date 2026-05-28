#include "dijkstra.h"
#include "graph.h"
#include "gui.h" // Nave's functions
#include "parser.h"
#include "raylib.h" // Graphics library
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // ==========================================
  // 1. Data Parsing and Validation (Bar's Logic)
  // ==========================================
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
    return EXIT_FAILURE;
  }

  Traveler *travelers = NULL;
  int num_travelers = 0;
  Graph *graph = parse_graph_from_file(argv[1], &travelers, &num_travelers);

  if (graph == NULL || travelers == NULL || num_travelers <= 0) {
    fprintf(stderr, "Error: Invalid input or negative weights detected.\n");
    return EXIT_FAILURE;
  }

  // ==========================================
  // 2. Claculate path for all travelers
  // ==========================================
  for (int i = 0; i < num_travelers; i++) {
    dijkstraResult result = find_shortest_path(
        graph->num_nodes, graph->matrix, travelers[i].src, travelers[i].dest);
    if (result.total_weight >= 0 && result.path_length > 0) {
      travelers[i].path = malloc(result.path_length * sizeof(int));
      if (travelers[i].path == NULL) {
        fprintf(stderr,
                "Error: Failed to allocate memory for traveler %d path.\n", i);
        return EXIT_FAILURE;
      }
      for (int j = 0; j < result.path_length; j++) {
        travelers[i].path[j] = result.path[j];
      }
      travelers[i].path_length = result.path_length;
    } else {
      travelers[i].path = NULL;
      travelers[i].path_length = 0;
    }
  }

  // ==========================================
  // 3. Create child processes
  // ==========================================
  for (int i = 0; i < num_travelers; i++) {
    if (travelers[i].path_length == 0)
      continue; // Skip if no path

    pid_t pid = fork();
    if (pid < 0) {
      fprintf(stderr, "Error: Failed to fork process for traveler %d\n", i);
      exit(EXIT_FAILURE);
    } else if (pid == 0) {
      // -------- child ----------
      printf("[%d] started\n", getpid());
      fflush(stdout);
      while (1)
        pause();          // Simulate ongoing process for animation
      exit(EXIT_SUCCESS); // Should never reach here!
    } else {
      // -------- parent ----------
      travelers[i].pid = pid; // Store child PID for future management
    }
  }

  // ==========================================
  // 4. Integration of Nave's GUI (Restored Fixes)
  // ==========================================

  // Set log level to warning to reduce console clutter from raylib
  SetTraceLogLevel(LOG_WARNING);
  // Open graphical window with expanded dimensions for 15 nodes
  InitWindow(1000, 800, "Traffic Simulation 2026 - Milestone 4");
  SetTargetFPS(60);

  VisualNode vNodes[MAX_NODES];
  InitGraphVisuals(graph->num_nodes, vNodes);

  // Create entities for travelers
  Entity *cars = calloc(num_travelers, sizeof(Entity));
  if (cars == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for entities.\n");
    exit(EXIT_FAILURE);
  }
  bool animationRunning = false; // Required for PLAY/STOP functionality

  for (int i = 0; i < num_travelers; i++) {
    if (travelers[i].path_length > 0 && travelers[i].path != NULL) {
      cars[i].currentPos = vNodes[travelers[i].path[0]].pos;
      cars[i].startNode = 0;
      cars[i].endNode = 1;
      cars[i].isWaiting = false;
      cars[i].timer = 0.0f;
      cars[i].color = travelers[i].color; // Assign unique color from traveler
    }
  }

  Rectangle buttonBounds = {20, 100, 120, 40};

  // Main game loop
  while (!WindowShouldClose()) {
    // Update all entities using your new function
    if (animationRunning) {
      UpdateEntities(cars, num_travelers, graph->num_nodes, vNodes,
                     graph->matrix, travelers);
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw background graph
    DrawStaticGraph(graph->num_nodes, vNodes, graph->matrix);

    // UI Information (Restored -1 display logic)
    DrawText("Milestone 4: Traffic Animation", 20, 20, 20, DARKGRAY);

    // Interactive Play/Stop Button
    if (DrawButton(buttonBounds, animationRunning ? "STOP" : "PLAY",
                   animationRunning)) {
      animationRunning = !animationRunning;
    }

    // Draw entities and completion message using your new function
    DrawEntities(
        cars, num_travelers); // Pass the colors array to the drawing function
    EndDrawing();
  }

  // ==========================================
  // 5. Clean Memory
  // ==========================================
  for (int i = 0; i < num_travelers; i++) {
    if (travelers[i].pid != 0) {
      kill(travelers[i].pid, SIGKILL);    // Terminate child process
      waitpid(travelers[i].pid, NULL, 0); // Wait for child
    }

    if (travelers[i].path != NULL) {
      free(travelers[i].path); // Free path memory
    }
  }
  free(travelers); // Free travelers array
  free(cars);      // Free entities array
  free_graph(graph);
  CloseWindow();

  return EXIT_SUCCESS;
}