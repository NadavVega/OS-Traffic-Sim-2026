#include "child.h"
#if MILESTONE >= 5
#include "dijkstra.h"
#include "ipc.h"
#endif
#if MILESTONE == 6
#include "node_locks.h"
#endif
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

//=======================================================
// run_child_process for MILESTONE 5:
// Receives graph, src, dest, write_fd.
// Calculates Dijkstra.
// Sends IPC messages to parent.
// Returns nothing because child exits with exit().

// run_child_process for MILESTONE 6:
// Receives graph, src, dest, write_fd, semaphore_id.
// Same as Milestone 5, but locks/unlocks current node.
// The 1-second node wait happens here.

// run_child_process for MILESTONE 7:
// Receives graph, src, dest, write_fd, grant_read_fd.
// Calculates path.
// For each node:
// 1. sends IPC_REQUEST_NODE to parent
// 2. blocks on private grant pipe
// 3. sends IPC_INSIDE_NODE
// 4. sleeps 1 second
// 5. sends IPC_LEFT_NODE
// 6. sends movement update
//========================================================

#if MILESTONE == 5

// ===================================================
// SECTION: Child process route execution and node access
// ===================================================

void run_child_process(Graph *graph, int src, int dest, int write_fd) {

#elif MILESTONE == 6

void run_child_process(Graph *graph, int src, int dest, int write_fd,
                       int semaphore_id) {

#elif MILESTONE >= 7

void run_child_process(Graph *graph, int src, int dest, int write_fd,
                       int grant_read_fd) {
#endif

#if MILESTONE >= 5
  dijkstraResult result =
      find_shortest_path(graph->num_nodes, graph->matrix, src, dest);
  pid_t pid = getpid();

  // Handle case where no path is found
  if (result.total_weight < 0 || result.path_length == 0) {
    IpcMessage no_path = {.pid = pid,
                          .current_node = src,
                          .next_node = -1,
                          .status = IPC_NO_PATH};
    if (ipc_send_message(write_fd, &no_path) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }
    IpcMessage finished = {.pid = pid,
                           .current_node = src,
                           .next_node = -1,
                           .status = IPC_FINISHED};
    int send_result = ipc_send_message(write_fd, &finished);
    close(write_fd);
    exit(send_result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  // Traverse the path
  for (int i = 0; i < result.path_length; i++) {
    bool at_destination = (i == result.path_length - 1);
    int current = result.path[i];
    int next = at_destination ? -1 : result.path[i + 1];

#if MILESTONE == 6
    IpcMessage waiting_msg = {.pid = pid,
                              .current_node = current,
                              .next_node = next,
                              .status = IPC_WAITING_LOCK};
    if (ipc_send_message(write_fd, &waiting_msg) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }

    if (node_locks_lock(semaphore_id, current) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }

    IpcMessage inside_msg = {.pid = pid,
                             .current_node = current,
                             .next_node = next,
                             .status = IPC_INSIDE_NODE};
    if (ipc_send_message(write_fd, &inside_msg) == -1) {
      node_locks_unlock(semaphore_id, current);
      close(write_fd);
      exit(EXIT_FAILURE);
    }

    sleep(1);

    if (node_locks_unlock(semaphore_id, current) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }
#elif MILESTONE >= 7
    // 1. Request permission from the parent scheduler
    IpcMessage req_msg = {.pid = pid,
                          .current_node = current,
                          .next_node = next,
                          .status = IPC_REQUEST_NODE};
    if (ipc_send_message(write_fd, &req_msg) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }

    // 2. Block and wait for the parent to grant permission via the private pipe
    IpcMessage grant_msg;
    if (read(grant_read_fd, &grant_msg, sizeof(grant_msg)) <= 0) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }

    // 3. Permission granted! Enter the node.
    IpcMessage inside_msg = {.pid = pid,
                             .current_node = current,
                             .next_node = next,
                             .status = IPC_INSIDE_NODE};
    if (ipc_send_message(write_fd, &inside_msg) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }

    sleep(1);

    // 4. Leave the node and notify the parent
    IpcMessage left_msg = {.pid = pid,
                           .current_node = current,
                           .next_node = next,
                           .status = IPC_LEFT_NODE};
    if (ipc_send_message(write_fd, &left_msg) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }
#endif

    IpcMessage movement_msg = {.pid = pid,
                               .current_node = current,
                               .next_node = next,
                               .status = at_destination ? IPC_ARRIVED_DEST
                                                        : IPC_EN_ROUTE};
    if (ipc_send_message(write_fd, &movement_msg) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }

    if (!at_destination) {
      int weight = graph->matrix[current][next];
      usleep((useconds_t)weight * 300000);
    }
  }

  // Complete process execution
  IpcMessage finished = {.pid = pid,
                         .current_node = result.path[result.path_length - 1],
                         .next_node = -1,
                         .status = IPC_FINISHED};
  int send_result = ipc_send_message(write_fd, &finished);
  close(write_fd);
  exit(send_result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
#else
  void run_child_process(void) {
    printf("[%d] started\n", getpid());
    fflush(stdout);

    for (;;) {
      pause();
    }
  }
#endif
