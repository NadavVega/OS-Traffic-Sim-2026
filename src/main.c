#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "graph.h"
#include "parser.h"
#include "dijkstra.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int src, dest;
    Graph* graph = parse_graph_from_file(argv[1], &src, &dest);

    if (graph == NULL) {
        fprintf(stderr, "Error: Invalid input or negative weights detected.\n");
        return EXIT_FAILURE;
    }

    if (src < 0 || src >= graph->num_nodes || dest < 0 || dest >= graph->num_nodes) {
    	fprintf(stderr, "Error: Source or destination node out of bounds.\n");
    	free_graph(graph);
    	return EXIT_FAILURE;
	}

    if (src == dest) {
        printf("%d\n0\n", src);
        free_graph(graph);
        return EXIT_SUCCESS;
    }

    // קריאה לפונקציה של נדב
    dijkstraResult result = find_shortest_path(graph->num_nodes, graph->matrix, src, dest);

    if (result.path[0] == -1) {
        printf("No path found\n");
    } else {
        // הדפסת המסלול עם חיצים
        for (int i = 0; i < result.path_length; i++) {
            printf("%d", result.path[i]);
            if (i < result.path_length - 1) {
                printf(" -> ");
            }
        }
        printf("\n%d\n", result.total_weight);
    }

    // 6. ניקוי זיכרון
    free_graph(graph);

    return EXIT_SUCCESS;
}