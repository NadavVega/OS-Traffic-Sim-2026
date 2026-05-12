#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "graph.h"
#include "parser.h"
#include "dijkstra.h"
#include "gui.h"     // Nave's functions and structures
#include "raylib.h"  // Graphics library

int main(int argc, char *argv[]) {
    // ==========================================
    // 1. Current logic (reading, validations, Dijkstra)
    // ==========================================
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

    // Early exit if source equals destination (no animation)
    if (src == dest) {
        printf("%d\n0\n", src);
        free_graph(graph);
        return EXIT_SUCCESS;
    }

    // Running Nadav's algorithm
    dijkstraResult result = find_shortest_path(graph->num_nodes, graph->matrix, src, dest);
    bool has_path = (result.path[0] != -1);

    // Print to terminal
    if (!has_path) {
        printf("No path found\n");
    } else {
        for (int i = 0; i < result.path_length; i++) {
            printf("%d", result.path[i]);
            if (i < result.path_length - 1) {
                printf(" -> ");
            }
        }
        printf("\n%d\n", result.total_weight);
    }

    // ==========================================
    // 2. Integration of Nave's GUI
    // ==========================================

    // Open graphical window
    InitWindow(800, 600, "Traffic Simulation 2026");
    SetTargetFPS(60);

    // Initialize graphical nodes (circle distribution)
    VisualNode vNodes[MAX_NODES];
    InitGraphVisuals(graph->num_nodes, vNodes);

    // Initialize entity only if a path is actually found
    Entity car = {0};
    if (has_path && result.path_length > 1) {
        car.currentPos = vNodes[result.path[0]].pos;
        car.startNode = 0; // Starting index in the path
        car.endNode = 1;   // Index of the next node
        car.currentJump = 0;
        car.timer = 0.0f;
        car.isWaiting = true; // Start by waiting at the first node
    }

    // Main game loop (runs every frame until user closes the window)
    while (!WindowShouldClose()) {

        // a. Update logic (Movement)
        if (has_path && result.path_length > 1) {
            // Update movement as long as we haven't finished the entire path
            if (car.endNode < result.path_length) {
                UpdateEntity(&car, graph->num_nodes, vNodes, graph->matrix, result.path, result.path_length);
            }
        }

        // b. Draw screen (Drawing)
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw background graph
        DrawStaticGraph(graph->num_nodes, vNodes, graph->matrix);

        // Draw entity or error message
        if (has_path && result.path_length > 1) {
            DrawEntity(car);
            // Print total weight at the bottom
            DrawText(TextFormat("Total Weight: %d", result.total_weight), 20, 550, 20, DARKGREEN);
        } else if (!has_path) {
            DrawText("No path found!", 300, 20, 20, RED);
        }

        EndDrawing();
    }

    // ==========================================
    // 3. Close and clean memory
    // ==========================================
    CloseWindow();
    free_graph(graph);

    return EXIT_SUCCESS;
}