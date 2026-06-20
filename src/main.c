#ifndef MILESTONE
#define MILESTONE 4
#endif

#include "child.h"
#include "dijkstra.h"
#include "graph.h"
#include "gui.h" // Nave's functions
#if MILESTONE >= 5
#include "ipc.h"
#endif
#if MILESTONE >= 6
#include "node_locks.h"
#endif
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

#if MILESTONE >= 5
static int find_traveler_by_pid(const Traveler travelers[], int num_travelers,
                                pid_t pid) {
  for (int i = 0; i < num_travelers; i++) {
    if (travelers[i].pid == pid) {
      return i;
    }
  }
  return -1;
}

static void handle_ipc_message(const IpcMessage *message, Traveler travelers[],
                               Entity entities[], int num_travelers,
                               const VisualNode nodes[], int num_nodes,
                               bool update_display) {
  int traveler_index =
      find_traveler_by_pid(travelers, num_travelers, message->pid);
  if (traveler_index < 0) {
    return;
  }

  Entity *entity = &entities[traveler_index];
  switch (message->status) {
  case IPC_EN_ROUTE:
    if (message->current_node < 0 || message->current_node >= num_nodes ||
        message->next_node < 0 || message->next_node >= num_nodes) {
      return;
    }
    entity->currentNode = message->current_node;
    entity->nextNode = message->next_node;
    if (update_display) {
      entity->currentPos = nodes[message->current_node].pos;
    }
    printf("[PID=%d] arrived at node %d | next node: %d\n", message->pid,
           message->current_node, message->next_node);
    break;
  case IPC_ARRIVED_DEST:
    if (message->current_node < 0 || message->current_node >= num_nodes) {
      return;
    }
    entity->currentNode = message->current_node;
    entity->nextNode = -1;
    if (update_display) {
      entity->currentPos = nodes[message->current_node].pos;
    }
    entity->arrived = true;
    printf("[PID=%d] arrived at node %d | DESTINATION\n", message->pid,
           message->current_node);
    break;
  case IPC_FINISHED:
    entity->finished = true;
    printf("[PID=%d] finished\n", message->pid);
    break;
  case IPC_NO_PATH:
    entity->currentNode = travelers[traveler_index].src;
    entity->nextNode = -1;
    printf("[PID=%d] No path found from %d to %d\n", message->pid,
           travelers[traveler_index].src, travelers[traveler_index].dest);
    break;
  case IPC_WAITING_LOCK:
    printf("[PID=%d] waiting for node %d\n", message->pid,
           message->current_node);
    break;
  case IPC_INSIDE_NODE:
    printf("[PID=%d] entered node %d\n", message->pid,
           message->current_node);
    break;
  }
  fflush(stdout);
}

static bool all_ms5_children_finished(const Entity entities[],
                                      const bool child_reaped[],
                                      int num_travelers) {
  for (int i = 0; i < num_travelers; i++) {
    if (!entities[i].finished || !child_reaped[i]) {
      return false;
    }
  }
  return true;
}

static void terminate_and_wait_for_ms5_children(
    const Traveler travelers[], bool child_reaped[], int num_travelers);

static int start_ms5_children(Graph *graph, Traveler travelers[],
                              int num_travelers, int pipe_fd[2],
                              bool child_reaped[]
#if MILESTONE >= 6
                              ,
                              int semaphore_id
#endif
) {
  int children_started = 0;
  for (int i = 0; i < num_travelers; i++) {
    pid_t pid = fork();
    if (pid < 0) {
      fprintf(stderr, "Error: Failed to fork process for traveler %d\n", i);
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      terminate_and_wait_for_ms5_children(travelers, child_reaped,
                                          children_started);
      return -1;
    }
    if (pid == 0) {
      close(pipe_fd[0]);
#if MILESTONE >= 6
      run_child_process(graph, travelers[i].src, travelers[i].dest, pipe_fd[1],
                        semaphore_id);
#else
      run_child_process(graph, travelers[i].src, travelers[i].dest,
                        pipe_fd[1]);
#endif
    }

    travelers[i].pid = pid;
    children_started++;
  }

  close(pipe_fd[1]);
  return 0;
}

static void reap_exited_children(const Traveler travelers[], bool child_reaped[],
                                 int num_travelers) {
  for (int i = 0; i < num_travelers; i++) {
    if (travelers[i].pid <= 0 || child_reaped[i]) {
      continue;
    }

    pid_t result;
    do {
      result = waitpid(travelers[i].pid, NULL, WNOHANG);
    } while (result == -1 && errno == EINTR);

    if (result == travelers[i].pid || (result == -1 && errno == ECHILD)) {
      child_reaped[i] = true;
    } else if (result == -1) {
      perror("Error: Failed to check child status");
    }
  }
}

static void terminate_and_wait_for_ms5_children(
    const Traveler travelers[], bool child_reaped[], int num_travelers) {
  for (int i = 0; i < num_travelers; i++) {
    if (travelers[i].pid <= 0 || child_reaped[i]) {
      continue;
    }

    pid_t result;
    do {
      result = waitpid(travelers[i].pid, NULL, WNOHANG);
    } while (result == -1 && errno == EINTR);

    if (result == 0) {
      if (kill(travelers[i].pid, SIGTERM) == -1 && errno != ESRCH) {
        perror("Error: Failed to terminate child");
      }
      do {
        result = waitpid(travelers[i].pid, NULL, 0);
      } while (result == -1 && errno == EINTR);
    }
    if (result == travelers[i].pid || (result == -1 && errno == ECHILD)) {
      child_reaped[i] = true;
    } else if (result == -1) {
      perror("Error: Failed to wait for child");
    }
  }
}
#endif

#if MILESTONE == 4
static bool all_ms4_children_finished(const Traveler travelers[],
                                      const bool termination_sent[],
                                      int num_travelers) {
  for (int i = 0; i < num_travelers; i++) {
    if (!termination_sent[i] || travelers[i].pid > 0) {
      return false;
    }
  }
  return true;
}

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
#endif

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

#if MILESTONE >= 5
  bool *child_reaped = calloc(num_travelers, sizeof(bool));
  if (child_reaped == NULL) {
    fprintf(stderr, "Error: Failed to allocate child process state.\n");
    free(travelers);
    free_graph(graph);
    return EXIT_FAILURE;
  }

  int pipe_fd[2];
  if (ipc_create_pipe(pipe_fd) == -1) {
    free(child_reaped);
    free(travelers);
    free_graph(graph);
    return EXIT_FAILURE;
  }
  if (ipc_set_nonblocking(pipe_fd[0]) == -1) {
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    free(child_reaped);
    free(travelers);
    free_graph(graph);
    return EXIT_FAILURE;
  }

#if MILESTONE >= 6
  int semaphore_id = node_locks_create(graph->num_nodes);
  if (semaphore_id == -1) {
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    free(child_reaped);
    free(travelers);
    free_graph(graph);
    return EXIT_FAILURE;
  }
  bool node_locks_destroyed = false;
#endif

  SetTraceLogLevel(LOG_WARNING);
  InitWindow(1000, 800, "Traffic Simulation 2026 - Milestone 5");
  if (!IsWindowReady()) {
    fprintf(stderr, "Error: Failed to initialize GUI window.\n");
    close(pipe_fd[0]);
    close(pipe_fd[1]);
#if MILESTONE >= 6
    node_locks_destroy(semaphore_id);
#endif
    free(child_reaped);
    free(travelers);
    free_graph(graph);
    return EXIT_FAILURE;
  }
  SetTargetFPS(60);

  VisualNode vNodes[MAX_NODES];
  InitGraphVisuals(graph->num_nodes, vNodes);

  Entity *cars = calloc(num_travelers, sizeof(Entity));
  if (cars == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for entities.\n");
    close(pipe_fd[0]);
    close(pipe_fd[1]);
#if MILESTONE >= 6
    node_locks_destroy(semaphore_id);
#endif
    free(child_reaped);
    free(travelers);
    free_graph(graph);
    CloseWindow();
    return EXIT_FAILURE;
  }

  for (int i = 0; i < num_travelers; i++) {
    cars[i].currentPos = vNodes[travelers[i].src].pos;
    cars[i].currentNode = travelers[i].src;
    cars[i].nextNode = -1;
    cars[i].color = travelers[i].color;
  }

  Rectangle buttonBounds = {20, 100, 120, 40};
  bool isPlaying = false;
  bool children_started = false;
  bool pipe_open = true;
  bool simulationCompleted = false;
  bool completionCleanupDone = false;

  while (!WindowShouldClose()) {
    if (!simulationCompleted && children_started && pipe_open) {
      for (;;) {
        IpcMessage message;
        IpcReadResult read_result = ipc_read_message(pipe_fd[0], &message);
        if (read_result == IPC_READ_MESSAGE) {
          handle_ipc_message(&message, travelers, cars, num_travelers, vNodes,
                             graph->num_nodes, isPlaying);
          continue;
        }
        if (read_result == IPC_READ_EOF || read_result == IPC_READ_ERROR) {
          close(pipe_fd[0]);
          pipe_open = false;
        }
        break;
      }
    }
    if (!simulationCompleted && children_started) {
      reap_exited_children(travelers, child_reaped, num_travelers);
    }

    if (!simulationCompleted && isPlaying) {
      for (int i = 0; i < num_travelers; i++) {
        cars[i].currentPos = vNodes[cars[i].currentNode].pos;
      }

      if (children_started &&
          all_ms5_children_finished(cars, child_reaped, num_travelers)) {
        simulationCompleted = true;
        isPlaying = false;
        if (!completionCleanupDone) {
          if (pipe_open) {
            close(pipe_fd[0]);
            pipe_open = false;
          }
          terminate_and_wait_for_ms5_children(travelers, child_reaped,
                                              num_travelers);
#if MILESTONE >= 6
          if (!node_locks_destroyed) {
            node_locks_destroy(semaphore_id);
            node_locks_destroyed = true;
          }
#endif
          completionCleanupDone = true;
        }
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawStaticGraph(graph->num_nodes, vNodes, graph->matrix);
    DrawText("Milestone 5: IPC Traffic Animation", 20, 20, 20, DARKGRAY);
    if (!simulationCompleted) {
      if (DrawButton(buttonBounds, isPlaying ? "STOP" : "PLAY", isPlaying)) {
        isPlaying = !isPlaying;
      }
    }
    DrawEntities(cars, num_travelers);
    if (simulationCompleted) {
      DrawCompletionMessage("Simulation completed");
    }
    EndDrawing();

    if (!simulationCompleted && isPlaying && !children_started) {
      if (start_ms5_children(graph, travelers, num_travelers, pipe_fd,
                             child_reaped
#if MILESTONE >= 6
                             ,
                             semaphore_id
#endif
                             ) == -1) {
        pipe_open = false;
#if MILESTONE >= 6
        node_locks_destroy(semaphore_id);
#endif
        free(child_reaped);
        free(cars);
        free(travelers);
        free_graph(graph);
        CloseWindow();
        return EXIT_FAILURE;
      }
      children_started = true;
    }
  }

  if (pipe_open) {
    close(pipe_fd[0]);
  }
  if (!children_started) {
    close(pipe_fd[1]);
  } else {
    terminate_and_wait_for_ms5_children(travelers, child_reaped,
                                        num_travelers);
  }
#if MILESTONE >= 6
  if (!node_locks_destroyed) {
    node_locks_destroy(semaphore_id);
  }
#endif
  free(child_reaped);
  free(cars);
  free(travelers);
  free_graph(graph);
  CloseWindow();
  return EXIT_SUCCESS;
#else
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
  bool simulationCompleted = false;

  // Main game loop
  while (!WindowShouldClose()) {
    // Update all entities using your new function
    if (!simulationCompleted && animationRunning) {
      UpdateEntities(cars, num_travelers, graph->num_nodes, vNodes,
                     graph->matrix, travelers);

      for (int i = 0; i < num_travelers; i++) {
        if (!termination_sent[i] &&
            cars[i].endNode >= travelers[i].path_length) {
          terminate_and_wait_for_child(&travelers[i].pid,
                                       &termination_sent[i]);
        }
      }

      if (all_ms4_children_finished(travelers, termination_sent,
                                    num_travelers)) {
        simulationCompleted = true;
        animationRunning = false;
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw background graph
    DrawStaticGraph(graph->num_nodes, vNodes, graph->matrix);

    // UI Information (Restored -1 display logic)
    DrawText("Milestone 4: Parent-Controlled Traffic Animation", 20, 20, 20,
             DARKGRAY);

    // Interactive Play/Stop Button
    if (!simulationCompleted) {
      if (DrawButton(buttonBounds, animationRunning ? "STOP" : "PLAY",
                     animationRunning)) {
        animationRunning = !animationRunning;
      }
    }

    // Draw entities and completion message using your new function
    DrawEntities(cars, num_travelers);
    if (simulationCompleted) {
      DrawCompletionMessage("Simulation completed");
    }
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
#endif
}
