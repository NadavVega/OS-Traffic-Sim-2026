#include "gui.h"
#include "raymath.h"
#include <math.h>

//========================================================
// InitGraphVisuals:
// Receives num_nodes and VisualNode array.
// Places nodes in circle.
// Returns nothing.

// DrawStaticGraph:
// Receives node count, visual nodes, graph matrix, scheduler name.
// Draws edges, arrows, weights, and nodes.
// Returns nothing.

// UpdateIpcEntities:
// Receives Entity array, node positions, graph matrix.
// Moves travelers visually based on IPC state.
// Returns nothing.

// DrawEntities:
// Receives Entity array.
// Draws traveler circles and outlines according to visual state.
//========================================================

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

// Drawing the static graph: edges, arrows, and weights
void DrawStaticGraph(int num_nodes, VisualNode nodes[], int graph[15][15],
                     const char *scheduler_name) {
  (void)scheduler_name; // main.c draws the scheduler text, not this function

  // Draw edges, arrows, and weights
  for (int i = 0; i < num_nodes; i++) {
    for (int j = 0; j < num_nodes; j++) {
      if (graph[i][j] > 0) {
        DrawLineEx(nodes[i].pos, nodes[j].pos, 2.0f, DARKGRAY);

        Vector2 direction = Vector2Subtract(nodes[j].pos, nodes[i].pos);
        float angle = atan2f(direction.y, direction.x);

        Vector2 arrowPoint = Vector2Subtract(
            nodes[j].pos, Vector2Scale(Vector2Normalize(direction), 35));

        DrawPoly(arrowPoint, 3, 12, angle * RAD2DEG, DARKGRAY);

        Vector2 mid = {(nodes[i].pos.x + nodes[j].pos.x) / 2,
                       (nodes[i].pos.y + nodes[j].pos.y) / 2};

        const char *weightText = TextFormat("%d", graph[i][j]);
        int fontSize = 20;
        int textWidth = MeasureText(weightText, fontSize);

        DrawCircleV(mid, 13, RAYWHITE);
        DrawText(weightText, mid.x - textWidth / 2, mid.y - fontSize / 2,
                 fontSize, RED);
      }
    }
  }

  // Draw nodes
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
