#include <stdio.h>
#include <stdlib.h>
#include "gui.h"
#include "raymath.h"
#include <math.h>
// Predefined colors for up to 15 travelers (Stage 4 - Unique Colors)
const Color travelerColors[15] = {RED,    ORANGE,  YELLOW, GREEN, BLUE,
                                  PURPLE, PINK,    BROWN,  GRAY,  MAROON,
                                  LIME,   SKYBLUE, VIOLET, GOLD,  DARKGREEN};

// Arrange nodes in a circle to prevent overlap (Stage 2)
void InitGraphVisuals(int num_nodes, VisualNode nodes[]) {
  // New center for 1000x800 window
  float centerX = 500.0f;
  float centerY = 400.0f;
  // Increased radius to utilize the extra space
  float radius = 330.0f;

  for (int i = 0; i < num_nodes; i++) {
    nodes[i].id = i;
    nodes[i].pos.x = centerX + radius * cosf(i * 2 * PI / num_nodes);
    nodes[i].pos.y = centerY + radius * sinf(i * 2 * PI / num_nodes);
  }
}

// Drawing the static graph: edges, arrows, and weights (Stage 2)
void DrawStaticGraph(int num_nodes, VisualNode nodes[], int graph[15][15], const char* scheduler_name) {
  // ... Keep all your existing loop code that draws edges, arrows, and weights exactly the same ...

  // Display the active scheduling algorithm if provided
  if (scheduler_name != NULL) {
      DrawText(TextFormat("Scheduler: %s", scheduler_name), 20, 50, 18, DARKGRAY);
  }

  // ... Keep all your existing loop code that draws the nodes (MAROON circles) exactly the same ...
}

// Interactive Play/Stop button implementation (Stage 3)
bool DrawButton(Rectangle bounds, const char *text, bool active) {
  Vector2 mousePoint = GetMousePosition();
  bool clicked = false;

  if (CheckCollisionPointRec(mousePoint, bounds)) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
      clicked = true;
    DrawRectangleRec(bounds, LIGHTGRAY);
  } else {
    DrawRectangleRec(bounds, active ? LIME : GRAY);
  }

  DrawRectangleLinesEx(bounds, 2, BLACK);
  DrawText(text, bounds.x + 10, bounds.y + 10, 20, BLACK);

  return clicked;
}

void DrawCompletionMessage(const char *text) {
  const int fontSize = 30;
  const int padding = 20;
  int textWidth = MeasureText(text, fontSize);
  Rectangle background = {
      .x = (1000.0f - textWidth) / 2.0f - padding,
      .y = 60.0f,
      .width = textWidth + padding * 2.0f,
      .height = fontSize + padding,
  };

  DrawRectangleRec(background, RAYWHITE);
  DrawRectangleLinesEx(background, 3.0f, DARKGREEN);
  DrawText(text, (1000 - textWidth) / 2, 70, fontSize, DARKGREEN);
}

// Movement and timing logic (Stage 3)
// Inside gui.c -> UpdateEntity()
// Movement and timing logic for multiple travelers (Stage 2 - Multi-traveler)
void UpdateEntities(Entity entities[], int num_travelers, int num_nodes,
                    VisualNode nodes[], int graph[15][15],
                    Traveler travelers[]) {

  // Loop through all travelers in the array
  for (int i = 0; i < num_travelers; i++) {
    // Create a pointer to the current traveler to keep your existing logic
    // intact
    Entity *entity = &entities[i];

    if (entity->endNode >= travelers[i].path_length)
      continue; // Skip if traveler has reached the end of their path

    // Waiting state: Pause for one second at each node
    if (entity->isWaiting) {
      entity->timer += GetFrameTime();
      if (entity->timer >= 1.0f) {
        entity->isWaiting = false;
        entity->timer = 0;
      }
      continue; // Move to the next traveler in the loop
    }

    int u = travelers[i].path[entity->startNode];
    int v = travelers[i].path[entity->endNode];
    int w = graph[u][v];

    entity->timer += GetFrameTime();

    // The total time to traverse this edge is (weight * 0.3) seconds
    float totalTravelTime = w * 0.3f;

    if (entity->timer <= totalTravelTime) {
      // Calculate smooth progression along the edge
      float t = entity->timer / totalTravelTime;
      entity->currentPos = Vector2Lerp(nodes[u].pos, nodes[v].pos, t);
    } else {
      // End of edge traversal
      entity->timer = 0;
      entity->currentPos = nodes[v].pos; // Snap exactly to the node
      entity->startNode++;
      entity->endNode++;

      if (entity->endNode < travelers[i].path_length) {
        entity->isWaiting = true;
      }
    }
  }
}

static Vector2 GetWaitingPosition(Vector2 nodePos, int travelerIndex) {
  float angle = travelerIndex * 2.0f * PI / 15.0f;
  Vector2 offset = {cosf(angle) * 42.0f, sinf(angle) * 42.0f};
  return Vector2Add(nodePos, offset);
}

void UpdateIpcEntities(Entity entities[], int num_travelers, int num_nodes,
                       VisualNode nodes[], int graph[15][15]) {
  for (int i = 0; i < num_travelers; i++) {
    Entity *entity = &entities[i];

    if (entity->currentNode < 0 || entity->currentNode >= num_nodes) {
      continue;
    }

    if (entity->visualState == ENTITY_VISUAL_WAITING) {
      entity->currentPos =
          GetWaitingPosition(nodes[entity->currentNode].pos, i);
      continue;
    }

    if (entity->visualState == ENTITY_VISUAL_INSIDE_NODE) {
      entity->currentPos = nodes[entity->currentNode].pos;
      continue;
    }

    if (entity->visualState != ENTITY_VISUAL_MOVING) {
      continue;
    }

    if (entity->nextNode < 0 || entity->nextNode >= num_nodes) {
      entity->currentPos = nodes[entity->currentNode].pos;
      entity->visualState = ENTITY_VISUAL_IDLE;
      continue;
    }

    int startNode = entity->currentNode;
    int endNode = entity->nextNode;
    entity->timer += GetFrameTime();
    float duration = entity->movementDuration;
    if (duration <= 0.0f) {
      int weight = graph[startNode][endNode];
      duration = weight > 0 ? weight * 0.3f : 0.5f;
    }
    float progress = entity->timer / duration;
    if (progress >= 1.0f) {
      progress = 1.0f;
      entity->visualState = ENTITY_VISUAL_IDLE;
    }

    entity->currentPos =
        Vector2Lerp(nodes[startNode].pos, nodes[endNode].pos, progress);
    if (progress >= 1.0f) {
      entity->currentNode = endNode;
    }
  }
}

// Draw all moving entities from the array (Stage 2 - Multi-traveler)
void DrawEntities(Entity entities[], int num_travelers) {
  // Loop and draw each traveler as a golden circle
  for (int i = 0; i < num_travelers; i++) {
    Color outline = BLACK;
    if (entities[i].visualState == ENTITY_VISUAL_WAITING) {
      outline = ORANGE;
    } else if (entities[i].visualState == ENTITY_VISUAL_INSIDE_NODE) {
      outline = GREEN;
    }

    DrawCircleV(
        entities[i].currentPos, 12,
        entities[i].color); // Use the unique color assigned to each traveler
    DrawCircleLines(entities[i].currentPos.x, entities[i].currentPos.y, 12,
                    outline);
  }
}
