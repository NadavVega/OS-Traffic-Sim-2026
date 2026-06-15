#include "child.h"
#if MILESTONE == 5
#include "dijkstra.h"
#include "ipc.h"
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if MILESTONE == 5
void run_child_process(Graph *graph, int src, int dest, int write_fd) {
  dijkstraResult result =
      find_shortest_path(graph->num_nodes, graph->matrix, src, dest);
  pid_t pid = getpid();

  if (result.total_weight < 0 || result.path_length == 0) {
    IpcMessage no_path = {
        .pid = pid,
        .current_node = src,
        .next_node = -1,
        .status = IPC_NO_PATH,
    };
    if (ipc_send_message(write_fd, &no_path) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }

    IpcMessage finished = {
        .pid = pid,
        .current_node = src,
        .next_node = -1,
        .status = IPC_FINISHED,
    };
    int send_result = ipc_send_message(write_fd, &finished);
    close(write_fd);
    exit(send_result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  for (int i = 0; i < result.path_length; i++) {
    bool at_destination = i == result.path_length - 1;
    IpcMessage arrival = {
        .pid = pid,
        .current_node = result.path[i],
        .next_node = at_destination ? -1 : result.path[i + 1],
        .status = at_destination ? IPC_ARRIVED_DEST : IPC_EN_ROUTE,
    };

    if (ipc_send_message(write_fd, &arrival) == -1) {
      close(write_fd);
      exit(EXIT_FAILURE);
    }

    if (!at_destination) {
      sleep(1);
      int weight = graph->matrix[result.path[i]][result.path[i + 1]];
      usleep((useconds_t)weight * 300000);
    }
  }

  IpcMessage finished = {
      .pid = pid,
      .current_node = result.path[result.path_length - 1],
      .next_node = -1,
      .status = IPC_FINISHED,
  };
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
