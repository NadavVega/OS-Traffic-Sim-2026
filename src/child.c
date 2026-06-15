#include "child.h"
#include "dijkstra.h"
#include "ipc.h"
#include <unistd.h>
#include <stdlib.h>

void run_child_process(Graph *graph, int src, int dest, int write_fd) {
    // 1. The child calculates the path independently
    dijkstraResult result = find_shortest_path(graph->num_nodes, graph->matrix, src, dest);

    // Handle cases with no valid path or errors
    if (result.total_weight < 0 || result.path_length == 0) {
        exit(1);
    }

    pid_t my_pid = getpid();
    ipc_message_t msg;
    msg.child_pid = my_pid;

    // 2. Move along the calculated path
    for (int i = 0; i < result.path_length; i++) {
        int current_node = result.path[i];
        msg.current_node = current_node;

        // Check if we have reached the destination
        if (i == result.path_length - 1) {
            msg.next_node = -1;
            msg.is_finished = 1;
        } else {
            msg.next_node = result.path[i + 1];
            msg.is_finished = 0;
        }

        // 3. Send a report to the parent upon arriving at the node
        send_message(write_fd, &msg);

        // 4. Wait for one full second at the node (as required by traffic rules)
        sleep(1);

        // 5. Simulate the travel time on the edge (if not yet at destination)
        if (!msg.is_finished) {
            int weight = graph->matrix[current_node][msg.next_node];
            // Each weight unit equals 300 milliseconds, which is 300,000 microseconds
            usleep(weight * 300000);
        }
    }

    // 6. Graceful exit of the child process
    exit(0);
}