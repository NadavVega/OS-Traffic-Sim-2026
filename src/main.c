#include "gui.h"
#include <stdio.h>

int main(void) {
  // 1. Initialize variables
  int N = 0, M = 0;
  int graph[15][15];

  // Initialize the matrix with -1 (no connections)
  for (int i = 0; i < 15; i++) {
    for (int j = 0; j < 15; j++) {
      graph[i][j] = -1;
    }
  }

  // 2. Load real data from input.txt
  FILE *file = fopen("input.txt", "r");
  if (!file) {
    printf("Error: Could not open input.txt!\n");
    return 1;
  }

  // Read number of nodes and edges
  fscanf(file, "%d %d", &N, &M);

  // Read each edge and its weight
  for (int i = 0; i < M; i++) {
    int u, v, w;
    fscanf(file, "%d %d %d", &u, &v, &w);
    if (u < 15 && v < 15) {
      graph[u][v] = w;
    }
  }

  // Read the query (start and end nodes)
  int start_node, end_node;
  fscanf(file, "%d %d", &start_node, &end_node);
  fclose(file);

  // 3. Initialize graphics window
  InitWindow(800, 600, "The Schedulers - Milestone 2");
  SetTargetFPS(60);

  // 4. Prepare visuals (Calculating node positions in a circle)
  VisualNode visualNodes[15];
  InitGraphVisuals(N, visualNodes); // Uses N to arrange nodes

  // 5. Main loop
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Milestone 2 Core: Draw the nodes and edges read from the file
    // This function checks if graph[i][j] > 0 before drawing
    DrawStaticGraph(N, visualNodes, graph);

    // UI Text for validation
    DrawText("Milestone 2: Static Graph from File", 20, 20, 20, DARKGRAY);
    DrawText(TextFormat("Nodes: %d, Edges: %d", N, M), 20, 50, 18, MAROON);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}