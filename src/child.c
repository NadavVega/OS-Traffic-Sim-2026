#include "child.h"
#if MILESTONE >= 5
#include "dijkstra.h"
#include "ipc.h"
#endif
#if MILESTONE >= 6
#include "node_locks.h"
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if MILESTONE == 5
void run_child_process(Graph *graph, int src, int dest, int write_fd) {
#elif MILESTONE >= 6
void run_child_process(Graph *graph, int src, int dest, int write_fd,
                       int semaphore_id) {
#endif
#if MILESTONE >= 5
  dijkstraResult result = find_shortest_path(graph->num_nodes, graph->matrix, src, dest);
  pid_t pid = getpid();

  // Handle case where no path is found
  if (result.total_weight < 0 || result.path_length == 0) {
    IpcMessage no_path = { .pid = pid, .current_node = src, .next_node = -1, .status = IPC_NO_PATH };
    if (ipc_send_message(write_fd, &no_path) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }
    IpcMessage finished = { .pid = pid, .current_node = src, .next_node = -1, .status = IPC_FINISHED };
    int send_result = ipc_send_message(write_fd, &finished);
    close(write_fd);
    exit(send_result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  // Traverse the path
  for (int i = 0; i < result.path_length; i++) {
    bool at_destination = (i == result.path_length - 1);
    int current = result.path[i];
    int next = at_destination ? -1 : result.path[i + 1];

#if MILESTONE >= 6
    IpcMessage waiting_msg = { .pid = pid, .current_node = current, .next_node = next, .status = IPC_WAITING_LOCK };
    if (ipc_send_message(write_fd, &waiting_msg) == -1) { close(write_fd); exit(EXIT_FAILURE); }

    if (node_locks_lock(semaphore_id, current) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }

    IpcMessage inside_msg = { .pid = pid, .current_node = current, .next_node = next, .status = IPC_INSIDE_NODE };
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
#endif

    IpcMessage movement_msg = { .pid = pid, .current_node = current, .next_node = next, .status = at_destination ? IPC_ARRIVED_DEST : IPC_EN_ROUTE };
    if (ipc_send_message(write_fd, &movement_msg) == -1) { close(write_fd); exit(EXIT_FAILURE); }

    if (!at_destination) {
      int weight = graph->matrix[current][next];
      usleep((useconds_t)weight * 300000);
    }
  }

  // Complete process execution
  IpcMessage finished = { .pid = pid, .current_node = result.path[result.path_length - 1], .next_node = -1, .status = IPC_FINISHED };
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
