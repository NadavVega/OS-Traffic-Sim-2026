// Created by Nave Toren on 09/05/2026.

#ifndef GUI_H
#define GUI_H

#include "raylib.h"

// Structure representing the visual representation of a node on the screen
typedef struct {
    Vector2 pos; // X and Y position in pixels
    int id;      // Node ID (0 to 14)
} VisualNode;

// Function to calculate node positions in a circle to ensure readability
void InitGraphVisuals(int num_nodes, VisualNode nodes[]);

// Function to draw the static graph (nodes, edges, arrows, and weights)
void DrawStaticGraph(int num_nodes, VisualNode nodes[], int graph[15][15]);

// Function to handle interactive button rendering and click detection
bool DrawButton(Rectangle bounds, const char* text, bool active);

// Structure for managing the state of the moving entity
typedef struct {
    Vector2 currentPos;   // Current position in pixels
    int startNode;        // Starting node of the current edge
    int endNode;          // Target node of the current edge
    int currentJump;      // Current jump index out of the total weight (W)
    float timer;          // Internal timer to measure jump duration
    bool isWaiting;       // Indicates if the entity is currently waiting at a node
} Entity;

// Function to update the entity's movement logic
void UpdateEntity(Entity* entity, int num_nodes, VisualNode nodes[], int graph[15][15], int path[], int pathSize);

// Function to render the entity on the screen
void DrawEntity(Entity entity);

#endif // GUI_H