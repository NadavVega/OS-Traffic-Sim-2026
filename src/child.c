#include "child.h"
#if MILESTONE >= 5
#include "dijkstra.h"
#include "ipc.h"
#include <semaphore.h>
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if MILESTONE >= 5
void run_child_process(Graph *graph, int src, int dest, int write_fd, sem_t *semaphores) {
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

    // --- CRITICAL SECTION: NODE SYNCHRONIZATION ---

    // 1. Send waiting status before attempting to lock the node
    IpcMessage waiting_msg = { .pid = pid, .current_node = current, .next_node = next, .status = IPC_WAITING_LOCK };
    if (ipc_send_message(write_fd, &waiting_msg) == -1) { close(write_fd); exit(EXIT_FAILURE); }

    // 2. Block until the node lock is acquired (Binyamin's lock mechanism)
    sem_wait(&semaphores[current]);

    // 3. Lock acquired successfully. Send inside node status
    IpcMessage inside_msg = { .pid = pid, .current_node = current, .next_node = next, .status = IPC_INSIDE_NODE };
    if (ipc_send_message(write_fd, &inside_msg) == -1) { close(write_fd); exit(EXIT_FAILURE); }

    // 4. Stay inside the node for exactly 1 full second
    sleep(1);

    // 5. Release the node lock
    sem_post(&semaphores[current]);

    // --- END OF CRITICAL SECTION ---

    // 6. Send the regular movement status (En route or Arrived at destination)
    IpcMessage movement_msg = { .pid = pid, .current_node = current, .next_node = next, .status = at_destination ? IPC_ARRIVED_DEST : IPC_EN_ROUTE };
    if (ipc_send_message(write_fd, &movement_msg) == -1) { close(write_fd); exit(EXIT_FAILURE); }

    // 7. Simulate travel time to the next node
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