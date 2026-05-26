#include "gui.h"
#include "raymath.h"
#include <math.h>

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
void DrawStaticGraph(int num_nodes, VisualNode nodes[], int graph[15][15]) {
  for (int i = 0; i < num_nodes; i++) {
    for (int j = 0; j < num_nodes; j++) {
      if (graph[i][j] > 0) {
        // 1. Draw edge
        DrawLineEx(nodes[i].pos, nodes[j].pos, 2.0f, DARKGRAY);

        // 2. Calculate and draw arrowhead at the intersection with the node
        Vector2 direction = Vector2Subtract(nodes[j].pos, nodes[i].pos);
        float angle = atan2f(direction.y, direction.x);
        Vector2 arrowPoint = Vector2Subtract(
            nodes[j].pos, Vector2Scale(Vector2Normalize(direction), 35));
        DrawPoly(arrowPoint, 3, 12, angle * RAD2DEG, DARKGRAY);

        // 3. Draw edge weight at the center with a clean background
        Vector2 mid = {(nodes[i].pos.x + nodes[j].pos.x) / 2,
                       (nodes[i].pos.y + nodes[j].pos.y) / 2};

        // Format the weight text and measure its width for centering
        const char *weightText = TextFormat("%d", graph[i][j]);
        int fontSize = 20;
        int textWidth = MeasureText(weightText, fontSize);

        // Draw a small white circle to hide the line behind the text
        DrawCircleV(mid, 13, RAYWHITE);

        // Draw the text exactly in the center of the midpoint
        DrawText(weightText, mid.x - textWidth / 2, mid.y - fontSize / 2,
                 fontSize, RED);
      }
    }
  }

  // Draw nodes (circle + ID)
  for (int i = 0; i < num_nodes; i++) {
    DrawCircleV(nodes[i].pos, 25, MAROON);
    DrawCircleLines(nodes[i].pos.x, nodes[i].pos.y, 25, BLACK);
    DrawText(TextFormat("%d", i), nodes[i].pos.x - 5, nodes[i].pos.y - 8, 20,
             WHITE);
  }
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

// Movement and timing logic (Stage 3)
// Inside gui.c -> UpdateEntity()
// Movement and timing logic for multiple travelers (Stage 2 - Multi-traveler)
void UpdateEntities(Entity entities[], int num_travelers, int num_nodes, VisualNode nodes[],
                    int graph[15][15], int path[], int pathSize) {

  // Loop through all travelers in the array
  for (int i = 0; i < num_travelers; i++) {
    // Create a pointer to the current traveler to keep your existing logic intact
    Entity *entity = &entities[i];

    // Waiting state: Pause for one second at each node
    if (entity->isWaiting) {
      entity->timer += GetFrameTime();
      if (entity->timer >= 1.0f) {
        entity->isWaiting = false;
        entity->timer = 0;
      }
      continue; // Move to the next traveler in the loop
    }

    int u = path[entity->startNode];
    int v = path[entity->endNode];
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

      if (entity->endNode < pathSize) {
        entity->isWaiting = true;
      }
    }
  }
}
// Draw all moving entities from the array (Stage 2 - Multi-traveler)
void DrawEntities(Entity entities[], int num_travelers) {
  // Loop and draw each traveler as a golden circle
  for (int i = 0; i < num_travelers; i++) {
    DrawCircleV(entities[i].currentPos, 12, GOLD); // Golden circle representing the vehicle/passenger
    DrawCircleLines(entities[i].currentPos.x, entities[i].currentPos.y, 12, BLACK);
  }
}