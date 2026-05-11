#include "raylib.h"
#include "gui.h"

int main(void) {
    // 1. Window Initialization
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "The Schedulers - OS Traffic Simulation");
    SetTargetFPS(60);

    // 2. Prepare Graph Data (Stage 2)
    int numNodes = 10;
    VisualNode nodes[15];
    InitGraphVisuals(numNodes, nodes);

    // Dummy data for testing edges and weights
    int testGraph[15][15] = {0};
    testGraph[0][1] = 10; // Edge from 0 to 1 with weight 10 (will take 3 seconds)
    testGraph[1][2] = 5;  // Edge from 1 to 2 with weight 5 (will take 1.5 seconds)

    // 3. Prepare Animation Data (Stage 3)
    bool animationRunning = false;
    Rectangle buttonBounds = { 20, 50, 120, 40 };

    // Dummy path for testing: from 0 to 1, then to 2
    int testPath[] = {0, 1, 2};
    int pathSize = 3;

    // Initialize the moving entity
    Entity myEntity = {0};
    myEntity.currentPos = nodes[testPath[0]].pos;
    myEntity.startNode = 0; // Current index in path (testPath) from which we depart
    myEntity.endNode = 1;   // Current index in path to which we are traveling

    // Game/Simulation Loop
    while (!WindowShouldClose()) {
        // --- Logic ---

        // Check for Play/Stop button interaction
        if (DrawButton(buttonBounds, animationRunning ? "STOP" : "PLAY", animationRunning)) {
            animationRunning = !animationRunning;
        }

        // Update movement only if animation is running and the end of the path hasn't been reached
        if (animationRunning && myEntity.endNode < pathSize) {
            UpdateEntity(&myEntity, numNodes, nodes, testGraph, testPath, pathSize);
        }

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Title and Status
        DrawText("Milestone 3: Traffic Animation Control", 20, 20, 20, DARKGRAY);
        DrawText(TextFormat("Status: %s", animationRunning ? "RUNNING" : "PAUSED"), 160, 60, 18,
                 animationRunning ? LIME : MAROON);

        // Draw the static portion of the graph
        DrawStaticGraph(numNodes, nodes, testGraph);

        // Draw the dynamic entity
        DrawEntity(myEntity);

        // Draw the button (Ensuring it remains on the top layer)
        DrawButton(buttonBounds, animationRunning ? "STOP" : "PLAY", animationRunning);

        EndDrawing();
    }

    // Clean up and Close
    CloseWindow();
    return 0;
}