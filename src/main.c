#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "graph.h"
#include "parser.h"
#include "dijkstra.h"
#include "gui.h"     // Nave's functions
#include "raylib.h"  // Graphics library

int main(int argc, char *argv[]) {
    // ==========================================
    // 1. Data Parsing and Validation (Bar's Logic)
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

    // Run Dijkstra
    dijkstraResult result = find_shortest_path(graph->num_nodes, graph->matrix, src, dest);
    bool has_path = (result.total_weight >= 0);

    // ==========================================
    // 2. Integration of Nave's GUI (Restored Fixes)
    // ==========================================
    
    // Open graphical window with expanded dimensions for 15 nodes
    InitWindow(1000, 800, "Traffic Simulation 2026 - Milestone 3");
    SetTargetFPS(60);

    VisualNode vNodes[MAX_NODES];
    InitGraphVisuals(graph->num_nodes, vNodes);

    // Target Stage 2: Array of entities and number of travelers
    Entity cars[1];
    int num_travelers = 1;
    bool animationRunning = false; // Required for PLAY/STOP functionality

    if (has_path && result.path_length > 0) {
        cars[0].currentPos = vNodes[result.path[0]].pos;
        cars[0].startNode = 0;
        cars[0].endNode = 1;
        cars[0].isWaiting = false;
        cars[0].timer = 0.0f;
    }

    Rectangle buttonBounds = { 20, 100, 120, 40 };

    // Main game loop
    while (!WindowShouldClose()) {
        // Update all entities using your new function
        if (animationRunning && has_path && cars[0].endNode < result.path_length) {
            UpdateEntities(cars, num_travelers, graph->num_nodes, vNodes, graph->matrix, result.path, result.path_length);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw background graph
        DrawStaticGraph(graph->num_nodes, vNodes, graph->matrix);

        // UI Information (Restored -1 display logic)
        DrawText("Milestone 3: Traffic Animation", 20, 20, 20, DARKGRAY);
        DrawText(TextFormat("Path: %d -> %d (Weight: %d)", src, dest, result.total_weight), 
                 20, 50, 18, has_path ? MAROON : RED);

        if (!has_path) {
            DrawText("ERROR: NO PATH FOUND (-1)", 20, 80, 18, RED);
        }

        // Interactive Play/Stop Button
        if (DrawButton(buttonBounds, animationRunning ? "STOP" : "PLAY", animationRunning)) {
            animationRunning = !animationRunning;
        }

        // Draw entities and completion message using your new function
        if (has_path) {
            DrawEntities(cars, num_travelers);

            if (cars[0].endNode >= result.path_length) {
                DrawText("DESTINATION REACHED!", 350, 400, 30, LIME);
                animationRunning = false;
            }
        }

        EndDrawing();
    }

    // ==========================================
    // 3. Clean Memory
    // ==========================================
    free_graph(graph);
    CloseWindow();

    return EXIT_SUCCESS;
}