#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

Graph* parse_graph_from_file(const char* filename, int* src, int* dest) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return NULL;
    }

    int nodes, edges;
    // Read the first line: number of nodes and edges
    if (fscanf(fp, "%d %d", &nodes, &edges) != 2) {
        fclose(fp);
        return NULL;
    }

    // Create the graph (ensures we don't exceed 15 nodes)
    Graph* g = create_graph(nodes, edges);
    if (g == NULL) {
        fclose(fp);
        return NULL;
    }

    // Read the edges
    for (int i = 0; i < edges; i++) {
        int u, v, w;
        if (fscanf(fp, "%d %d %d", &u, &v, &w) == 3) {
            // Check for negative weights - mandatory requirement from the lecturer
            if (w < 0) {
                free_graph(g);
                fclose(fp);
                return NULL;
            }
            add_edge(g, u, v, w);
        }
    }

    // Read the last line: source and destination for the query
    if (fscanf(fp, "%d %d", src, dest) != 2) {
        // If information is missing, use default values or return an error
    }

    fclose(fp);
    return g;
}