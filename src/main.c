#ifndef MILESTONE
#define MILESTONE 4
#endif
#include "child.h"
#include "dijkstra.h"
#include "graph.h"
#include "gui.h" // Nave's functions
#include <stdlib.h>
#include <string.h>
#if MILESTONE >= 5
#include "ipc.h"
#endif
#if MILESTONE == 6
#include "node_locks.h"
#endif
#if MILESTONE >= 7
#include "scheduler.h"
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

//====================================================================
// find_traveler_by_pid:
// Receives travelers array, count, pid.
// Returns traveler index or -1.

// grant_next_waiting_traveler:
// Milestone 7 only.
// Receives scheduler, node, grant_write_fds, travelers.
// Chooses next waiting traveler and sends IPC_GRANTED_NODE through private
// pipe. Returns 1 granted, 0 no waiting traveler, -1 error.

// handle_ipc_message:
// Receives IpcMessage and project state.
// Finds traveler by PID.
// Updates Entity state.
// Prints logs.
// For Milestone 7, handles IPC_REQUEST_NODE and IPC_LEFT_NODE.

// start_ms5_children:
// Receives graph, travelers, pipe, child_reaped, plus semaphore or grant pipes.
// Forks one child per traveler.
// In child: closes unused FDs and calls run_child_process.
// In parent: stores PID.
// Returns 0 success, -1 failure.

// reap_exited_children:
// Receives travelers and child_reaped.
// Uses waitpid(..., WNOHANG).
// Returns nothing.

// terminate_and_wait_for_ms5_children:
// Receives travelers and child_reaped.
// Kills unfinished children if needed.
// Waits for them.
// Returns nothing.

// main:
// Receives argc/argv.
// Parses command line.
// Loads graph and travelers.
// Creates IPC pipe.
// Creates semaphore/scheduler resources depending on milestone.
// Starts GUI loop.
// Starts children on PLAY.
// Reads IPC.
// Draws GUI.
// Cleans up everything.
// Returns EXIT_SUCCESS or EXIT_FAILURE.
//==========================================================================

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

// ===================================================
// SECTION: Milestone 7 scheduler grant flow
// ===================================================

#if MILESTONE >= 7
static int grant_next_waiting_traveler(SchedulerState *scheduler, int node,
                                       int grant_write_fds[],
                                       const Traveler travelers[]) {
  SchedulerItem selected;
  int result = scheduler_choose_next(scheduler, node, &selected);
  if (result <= 0) {
    return result;
  }

  if (scheduler_mark_node_busy(scheduler, node) == -1) {
    return -1;
  }

  IpcMessage grant = {.status = IPC_GRANTED_NODE};
  if (ipc_send_message(grant_write_fds[selected.traveler_index], &grant) ==
      -1) {
    scheduler_mark_node_free(scheduler, node);
    return -1;
  }

  printf("  -> Scheduler granted node %d to PID %d\n", node,
         travelers[selected.traveler_index].pid);
  return 1;
}
#endif

// ===================================================
// SECTION: Parent-side IPC handling and GUI state updates
// ===================================================

static void handle_ipc_message(const IpcMessage *message, Traveler travelers[],
                               Entity entities[], int num_travelers,
                               const VisualNode nodes[], int num_nodes,
                               int graph[15][15], bool update_display
#if MILESTONE >= 7
                               ,
                               int grant_write_fds[], SchedulerState *scheduler
#endif
) {
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
      entity->visualState = ENTITY_VISUAL_MOVING;
      entity->timer = 0.0f;
      int weight = graph[message->current_node][message->next_node];
      entity->movementDuration = weight > 0 ? weight * 0.3f : 0.5f;
      if (update_display) {
        entity->currentPos = nodes[message->current_node].pos;
      }
      printf("[PID=%d] arrived at node %d | next node: %d\n", message->pid,
             message->current_node, message->next_node);

#if MILESTONE >= 7
      // --- הוספה למשימת המבחן: שליחת אישור (ACK) מהאב לבן ---
      IpcMessage ack_msg = {.status = IPC_GRANTED_NODE}; // התוכן לא משנה לפי ההוראות
      if (ipc_send_message(grant_write_fds[traveler_index], &ack_msg) == -1) {
        perror("Error: Failed to send ACK to child");
      }else {
        // הוספנו הדפסה כדי לראות שהאישור נשלח
        printf("  [PARENT ACK] Sent movement ACK to PID %d\n", message->pid);
      }
#endif
      break;
  case IPC_ARRIVED_DEST:
    if (message->current_node < 0 || message->current_node >= num_nodes) {
      return;
    }
    entity->currentNode = message->current_node;
    entity->nextNode = -1;
    entity->visualState = ENTITY_VISUAL_IDLE;
    entity->timer = 0.0f;
    entity->movementDuration = 0.0f;
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
    entity->visualState = ENTITY_VISUAL_IDLE;
    entity->timer = 0.0f;
    entity->movementDuration = 0.0f;
    printf("[PID=%d] No path found from %d to %d\n", message->pid,
           travelers[traveler_index].src, travelers[traveler_index].dest);
    break;
  case IPC_WAITING_LOCK:
    if (message->current_node < 0 || message->current_node >= num_nodes) {
      return;
    }
    entity->currentNode = message->current_node;
    entity->nextNode = message->next_node;
    entity->visualState = ENTITY_VISUAL_WAITING;
    entity->timer = 0.0f;
    entity->movementDuration = 0.0f;
    printf("[PID=%d] waiting for node %d\n", message->pid,
           message->current_node);
    break;
  case IPC_INSIDE_NODE:
    if (message->current_node < 0 || message->current_node >= num_nodes) {
      return;
    }
    entity->currentNode = message->current_node;
    entity->nextNode = message->next_node;
    entity->visualState = ENTITY_VISUAL_INSIDE_NODE;
    entity->timer = 0.0f;
    entity->movementDuration = 0.0f;
    if (update_display) {
      entity->currentPos = nodes[message->current_node].pos;
    }
    printf("[PID=%d] entered node %d\n", message->pid, message->current_node);
    break;

#if MILESTONE >= 7
  case IPC_REQUEST_NODE: {
    if (message->current_node < 0 || message->current_node >= num_nodes) {
      return;
    }
    entity->currentNode = message->current_node;
    entity->nextNode = message->next_node;
    entity->visualState = ENTITY_VISUAL_WAITING;
    entity->timer = 0.0f;
    entity->movementDuration = 0.0f;

    int node = message->current_node;
    int job_length = 0;
    if (message->next_node >= 0 && message->next_node < num_nodes) {
      job_length = graph[node][message->next_node];
    }
    if (job_length < 0) {
      job_length = 0;
    }

    if (scheduler_add_waiting(scheduler, node, traveler_index, message->pid,
                              message->next_node, job_length) == -1) {
      fprintf(stderr, "Error: Failed to queue PID %d for node %d\n",
              message->pid, node);
      return;
    }

    printf("[PID=%d] requested node %d (next edge weight: %d)\n", message->pid,
           node, job_length);

    if (!scheduler_is_node_busy(scheduler, node)) {
      grant_next_waiting_traveler(scheduler, node, grant_write_fds, travelers);
    }
    break;
  }
  case IPC_LEFT_NODE: {
    if (message->current_node < 0 || message->current_node >= num_nodes) {
      return;
    }
    int left_node = message->current_node;
    scheduler_mark_node_free(scheduler, left_node);
    printf("[PID=%d] left node %d\n", message->pid, left_node);
    grant_next_waiting_traveler(scheduler, left_node, grant_write_fds,
                                travelers);
    break;
  }
#endif
#if MILESTONE < 7
  case IPC_REQUEST_NODE:
  case IPC_LEFT_NODE:
    break;
#endif
  case IPC_GRANTED_NODE:
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

static void terminate_and_wait_for_ms5_children(const Traveler travelers[],
                                                bool child_reaped[],
                                                int num_travelers);

// ===================================================
// SECTION: Child process creation and cleanup
// ===================================================

static int start_ms5_children(Graph *graph, Traveler travelers[],
                              int num_travelers, int pipe_fd[2],
                              bool child_reaped[]
#if MILESTONE == 6
                              ,
                              int semaphore_id
#elif MILESTONE >= 7
                              ,
                              int grant_pipes[][2]
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
#if MILESTONE >= 7
      for (int j = 0; j < num_travelers; j++) {
        close(grant_pipes[j][1]); // Child doesn't write to grant pipes
        if (j != i) {
          close(grant_pipes[j][0]); // Child only needs its own read pipe
        }
      }
      run_child_process(graph, travelers[i].src, travelers[i].dest, pipe_fd[1],
                        grant_pipes[i][0]);
#elif MILESTONE == 6
      run_child_process(graph, travelers[i].src, travelers[i].dest, pipe_fd[1],
                        semaphore_id);
#else
      run_child_process(graph, travelers[i].src, travelers[i].dest, pipe_fd[1]);
#endif
    }

    travelers[i].pid = pid;
    children_started++;
  }

  close(pipe_fd[1]);
  return 0;
}

static void reap_exited_children(const Traveler travelers[],
                                 bool child_reaped[], int num_travelers) {
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

static void terminate_and_wait_for_ms5_children(const Traveler travelers[],
                                                bool child_reaped[],
                                                int num_travelers) {
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

// ===================================================
// SECTION: Program setup, GUI loop, and final cleanup
// ===================================================

// ===================================================
// ------------------ MAIN ---------------------------
// ===================================================

int main(int argc, char *argv[]) {
  const char *input_filename = NULL;
  const char *scheduler_name = "fcfs";
#if MILESTONE >= 7
  SchedulerAlgorithm scheduler_algorithm = SCHEDULER_FCFS;
#endif

  if (argc == 2) {
    input_filename = argv[1];
  } else if (argc == 4 && strcmp(argv[1], "-schd") == 0) {
    input_filename = argv[3];
#if MILESTONE >= 7
    if (scheduler_parse_algorithm(argv[2], &scheduler_algorithm) == -1) {
      fprintf(stderr, "Usage: %s [-schd fcfs|sjf] <file_name>\n", argv[0]);
      return EXIT_FAILURE;
    }
    scheduler_name = scheduler_algorithm_name(scheduler_algorithm);
#else
    scheduler_name = argv[2];
#endif
  } else {
    fprintf(stderr, "Usage: %s [-schd fcfs|sjf] <file_name>\n", argv[0]);
    return EXIT_FAILURE;
  }

  Traveler *travelers = NULL;
  int num_travelers = 0;
  Graph *graph =
      parse_graph_from_file(input_filename, &travelers, &num_travelers);

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

#if MILESTONE >= 7
  SchedulerState *scheduler =
      scheduler_create(graph->num_nodes, scheduler_algorithm);
  if (scheduler == NULL) {
    free(child_reaped);
    free(travelers);
    free_graph(graph);
    return EXIT_FAILURE;
  }
#endif

  int pipe_fd[2];
  if (ipc_create_pipe(pipe_fd) == -1) {
#if MILESTONE >= 7
    scheduler_destroy(scheduler);
#endif
    free(child_reaped);
    free(travelers);
    free_graph(graph);
    return EXIT_FAILURE;
  }
  if (ipc_set_nonblocking(pipe_fd[0]) == -1) {
    close(pipe_fd[0]);
    close(pipe_fd[1]);
#if MILESTONE >= 7
    scheduler_destroy(scheduler);
#endif
    free(child_reaped);
    free(travelers);
    free_graph(graph);
    return EXIT_FAILURE;
  }

#if MILESTONE == 6
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

#if MILESTONE >= 7
  int grant_pipes[num_travelers][2];
  int grant_write_fds[num_travelers];
  for (int i = 0; i < num_travelers; i++) {
    if (pipe(grant_pipes[i]) == -1) {
      perror("Error: Failed to create grant pipes");
      for (int j = 0; j < i; j++) {
        close(grant_pipes[j][0]);
        close(grant_pipes[j][1]);
      }
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      scheduler_destroy(scheduler);
      free(child_reaped);
      free(travelers);
      free_graph(graph);
      return EXIT_FAILURE;
    }
    grant_write_fds[i] = grant_pipes[i][1];
  }
#endif

  SetTraceLogLevel(LOG_WARNING);
  InitWindow(1000, 800, "Traffic Simulation 2026 - Milestone 5/6/7");
  if (!IsWindowReady()) {
    fprintf(stderr, "Error: Failed to initialize GUI window.\n");
    close(pipe_fd[0]);
    close(pipe_fd[1]);
#if MILESTONE == 6
    node_locks_destroy(semaphore_id);
#endif
#if MILESTONE >= 7
    for (int i = 0; i < num_travelers; i++) {
      close(grant_pipes[i][0]);
      close(grant_pipes[i][1]);
    }
    scheduler_destroy(scheduler);
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
#if MILESTONE == 6
    node_locks_destroy(semaphore_id);
#endif
#if MILESTONE >= 7
    for (int i = 0; i < num_travelers; i++) {
      close(grant_pipes[i][0]);
      close(grant_pipes[i][1]);
    }
    scheduler_destroy(scheduler);
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
                             graph->num_nodes, graph->matrix, isPlaying
#if MILESTONE >= 7
                             ,
                             grant_write_fds, scheduler
#endif
          );
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
      UpdateIpcEntities(cars, num_travelers, graph->num_nodes, vNodes,
                        graph->matrix);

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
#if MILESTONE == 6
          if (!node_locks_destroyed) {
            node_locks_destroy(semaphore_id);
            node_locks_destroyed = true;
          }
#endif
#if MILESTONE >= 7
          for (int i = 0; i < num_travelers; i++) {
            close(grant_write_fds[i]);
          }
          scheduler_destroy(scheduler);
          scheduler = NULL;
#endif
          completionCleanupDone = true;
        }
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawStaticGraph(graph->num_nodes, vNodes, graph->matrix, scheduler_name);
    DrawText("Milestone Simulation", 20, 20, 20, DARKGRAY);

#if MILESTONE >= 7
    DrawText(TextFormat("Scheduler: %s", scheduler_name), 20, 50, 20, BLUE);
#endif

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
#if MILESTONE == 6
                             ,
                             semaphore_id
#elif MILESTONE >= 7
                             ,
                             grant_pipes
#endif
                             ) == -1) {
        pipe_open = false;
#if MILESTONE == 6
        node_locks_destroy(semaphore_id);
#endif
#if MILESTONE >= 7
        for (int i = 0; i < num_travelers; i++) {
          close(grant_pipes[i][0]);
          close(grant_pipes[i][1]);
        }
        scheduler_destroy(scheduler);
#endif
        free(child_reaped);
        free(cars);
        free(travelers);
        free_graph(graph);
        CloseWindow();
        return EXIT_FAILURE;
      }
      children_started = true;
#if MILESTONE >= 7
      for (int i = 0; i < num_travelers; i++) {
        close(grant_pipes[i][0]); // Parent closes read ends of grant pipes
      }
#endif
    }
  }

  if (pipe_open) {
    close(pipe_fd[0]);
  }
  if (!children_started) {
    close(pipe_fd[1]);
  } else {
    terminate_and_wait_for_ms5_children(travelers, child_reaped, num_travelers);
  }
#if MILESTONE == 6
  if (!node_locks_destroyed) {
    node_locks_destroy(semaphore_id);
  }
#endif
#if MILESTONE >= 7
  if (!completionCleanupDone && children_started) {
    for (int i = 0; i < num_travelers; i++) {
      close(grant_write_fds[i]);
    }
  } else if (!children_started) {
    for (int i = 0; i < num_travelers; i++) {
      close(grant_pipes[i][0]);
      close(grant_pipes[i][1]);
    }
  }
  scheduler_destroy(scheduler);
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
        terminate_and_wait_for_child(&travelers[j].pid, &termination_sent[j]);
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
          terminate_and_wait_for_child(&travelers[i].pid, &termination_sent[i]);
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

    // Pass the scheduler name to the drawing function for Milestone 4 fallback
    DrawStaticGraph(graph->num_nodes, vNodes, graph->matrix, scheduler_name);

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
