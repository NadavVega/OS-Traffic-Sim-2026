#include "child.h"
#include "dijkstra.h"
#include "ipc.h"
#include <unistd.h>
#include <stdlib.h>

void run_child_process(Graph *graph, int src, int dest, int write_fd) {
    // 1. הבן מחשב את המסלול בעצמו
    dijkstraResult result = find_shortest_path(graph->num_nodes, graph->matrix, src, dest);

    // טיפול במקרה של שגיאה או שאין מסלול
    if (result.total_weight < 0 || result.path_length == 0) {
        exit(1); 
    }

    pid_t my_pid = getpid();
    ipc_message_t msg;
    msg.child_pid = my_pid;

    // 2. תנועה לאורך המסלול שחושב
    for (int i = 0; i < result.path_length; i++) {
        int current_node = result.path[i];
        msg.current_node = current_node;

        // בדיקה האם הגענו ליעד
        if (i == result.path_length - 1) {
            msg.next_node = -1;
            msg.is_finished = 1;
        } else {
            msg.next_node = result.path[i + 1];
            msg.is_finished = 0;
        }

        // 3. שליחת דיווח לאבא על ההגעה לצומת
        send_message(write_fd, &msg);

        // 4. המתנה של שניה שלמה בצומת (כנדרש בכללי התנועה)
        sleep(1);

        // 5. סימולציית זמן הנסיעה על הקשת (אם טרם הגענו ליעד)
        if (!msg.is_finished) {
            int weight = graph->matrix[current_node][msg.next_node];
            // כל יחידת משקל שווה ל-300 מילישניות, שהן 300,000 מיקרו-שניות
            usleep(weight * 300000);
        }
    }

    // 6. סיום תקין של תהליך הבן
    exit(0);
}