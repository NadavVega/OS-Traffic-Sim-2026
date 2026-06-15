#include "child.h"
#include "dijkstra.h"
#include "graph.h"
#include "gui.h" // Nave's functions
#include "parser.h"
#include "raylib.h" // Graphics library
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void terminate_and_wait_for_child(pid_t *pid, bool *termination_sent) {
  if (*pid <= 0) {
    return;
  }

  if (!*termination_sent) {
    if (kill(*pid, SIGTERM) == -1 && errno != ESRCH) {
      perror("Error: Failed to terminate child");
    }
    *termination_sent = true;
  }

  while (waitpid(*pid, NULL, 0) == -1) {
    if (errno != EINTR) {
      perror("Error: Failed to wait for child");
      break;
    }
  }
  *pid = 0;
}

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
  // 3. Create one child process per traveler
  // ==========================================
  bool *termination_sent = calloc(num_travelers, sizeof(bool));
  if (termination_sent == NULL) {
    fprintf(stderr, "Error: Failed to allocate child process state.\n");
    return EXIT_FAILURE;
  }

  for (int i = 0; i < num_travelers; i++) {
    pid_t pid = fork();
    if (pid < 0) {
      fprintf(stderr, "Error: Failed to fork process for traveler %d\n", i);
      for (int j = 0; j < i; j++) {
        terminate_and_wait_for_child(&travelers[j].pid,
                                     &termination_sent[j]);
      }
      free(termination_sent);
      for (int j = 0; j < num_travelers; j++) {
        free(travelers[j].path);
      }
      free(travelers);
      free_graph(graph);
      return EXIT_FAILURE;
    } else if (pid == 0) {
      run_child_process();
    } else {
      travelers[i].pid = pid;
    }
  }

  // ==========================================
  // 4. Integration of Nave's GUI (Restored Fixes)
  // ==========================================

  // Set log level to warning to reduce console clutter from raylib
  SetTraceLogLevel(LOG_WARNING);
  // Open graphical window with expanded dimensions for 15 nodes
  InitWindow(1000, 800, "Traffic Simulation 2026 - Milestone 4");
  if (!IsWindowReady()) {
    fprintf(stderr, "Error: Failed to initialize GUI window.\n");
    for (int i = 0; i < num_travelers; i++) {
      terminate_and_wait_for_child(&travelers[i].pid, &termination_sent[i]);
      free(travelers[i].path);
    }
    free(termination_sent);
    free(travelers);
    free_graph(graph);
    return EXIT_FAILURE;
  }
  SetTargetFPS(60);

  VisualNode vNodes[MAX_NODES];
  InitGraphVisuals(graph->num_nodes, vNodes);

  // Create entities for travelers
  Entity *cars = calloc(num_travelers, sizeof(Entity));
  if (cars == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for entities.\n");
    for (int i = 0; i < num_travelers; i++) {
      terminate_and_wait_for_child(&travelers[i].pid, &termination_sent[i]);
      free(travelers[i].path);
    }
    free(termination_sent);
    free(travelers);
    free_graph(graph);
    CloseWindow();
    return EXIT_FAILURE;
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

      for (int i = 0; i < num_travelers; i++) {
        if (!termination_sent[i] &&
            cars[i].endNode >= travelers[i].path_length) {
          terminate_and_wait_for_child(&travelers[i].pid,
                                       &termination_sent[i]);
        }
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw background graph
    DrawStaticGraph(graph->num_nodes, vNodes, graph->matrix);

    // UI Information (Restored -1 display logic)
    DrawText("Milestone 5: Integrated Traffic Animation", 20, 20, 20, DARKGRAY);

    // Interactive Play/Stop Button
    if (DrawButton(buttonBounds, animationRunning ? "STOP" : "PLAY",
                   animationRunning)) {
      animationRunning = !animationRunning;
    }

    // Draw entities and completion message using your new function
    DrawEntities(cars, num_travelers);
    EndDrawing();
  }

  // ==========================================
  // 5. Clean Memory
  // ==========================================
  for (int i = 0; i < num_travelers; i++) {
    terminate_and_wait_for_child(&travelers[i].pid, &termination_sent[i]);

    if (travelers[i].path != NULL) {
      free(travelers[i].path); // Free path memory
    }
  }
  free(termination_sent);
  free(travelers); // Free travelers array
  free(cars);      // Free entities array
  free_graph(graph);
  CloseWindow();

  return EXIT_SUCCESS;
}
