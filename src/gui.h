// Created by Nave Toren on 09/05/2026.

#ifndef GUI_H
#define GUI_H

#include "parser.h" // For Traveler struct and related data
#include "raylib.h"

// Structure representing the visual representation of a node on the screen
typedef struct {
  Vector2 pos; // X and Y position in pixels
  int id;      // Node ID (0 to 14)
} VisualNode;

// Predefined colors for up to 15 travelers (Stage 4 - Unique Colors)
extern const Color travelerColors[15];

// Function to calculate node positions in a circle to ensure readability
void InitGraphVisuals(int num_nodes, VisualNode nodes[]);

// Function to draw the static graph (nodes, edges, arrows, and weights)
void DrawStaticGraph(int num_nodes, VisualNode nodes[], int graph[15][15]);

// Function to handle interactive button rendering and click detection
bool DrawButton(Rectangle bounds, const char *text, bool active);

// Structure for managing the state of the moving entity
typedef struct {
  Vector2 currentPos; // Current position in pixels
  int startNode;      // Starting node of the current edge
  int endNode;        // Target node of the current edge
  int currentJump;    // Current jump index out of the total weight (W)
  float timer;        // Internal timer to measure jump duration
  bool isWaiting;     // Indicates if the entity is currently waiting at a node
  Color color;        // Unique color for the entity
} Entity;

// Function to update the movement logic for all entities in the array
void UpdateEntities(Entity entities[], int num_travelers, int num_nodes,
                    VisualNode nodes[], int graph[15][15],
                    Traveler travelers[]);

// Function to render all entities from the array on the screen
void DrawEntities(Entity entities[], int num_travelers);

#endif // GUI_H