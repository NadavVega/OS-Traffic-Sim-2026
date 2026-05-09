#include "gui.h"
#include "raymath.h"
#include <math.h>

// סידור הצמתים במעגל למניעת חפיפה (שלב 2)
void InitGraphVisuals(int num_nodes, VisualNode nodes[]) {
    float centerX = 400.0f;
    float centerY = 300.0f;
    float radius = 220.0f;

    for (int i = 0; i < num_nodes; i++) {
        nodes[i].id = i;
        nodes[i].pos.x = centerX + radius * cosf(i * 2 * PI / num_nodes);
        nodes[i].pos.y = centerY + radius * sinf(i * 2 * PI / num_nodes);
    }
}

// ציור הגרף הסטטי: קשתות, חצים ומשקלים (שלב 2)
void DrawStaticGraph(int num_nodes, VisualNode nodes[], int graph[15][15]) {
    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < num_nodes; j++) {
            if (graph[i][j] > 0) {
                // 1. ציור קשת
                DrawLineEx(nodes[i].pos, nodes[j].pos, 2.0f, DARKGRAY);

                // 2. חישוב וציור ראש חץ בנקודת המפגש עם הצומת
                Vector2 direction = Vector2Subtract(nodes[j].pos, nodes[i].pos);
                float angle = atan2f(direction.y, direction.x);
                Vector2 arrowPoint = Vector2Subtract(nodes[j].pos, Vector2Scale(Vector2Normalize(direction), 35));
                DrawPoly(arrowPoint, 3, 12, angle * RAD2DEG, DARKGRAY);

                // 3. ציור משקל הקשת במרכז
                Vector2 mid = { (nodes[i].pos.x + nodes[j].pos.x) / 2, (nodes[i].pos.y + nodes[j].pos.y) / 2 };
                DrawText(TextFormat("%d", graph[i][j]), mid.x + 10, mid.y + 10, 20, RED);
            }
        }
    }

    // ציור צמתים (עיגול + מספר)
    for (int i = 0; i < num_nodes; i++) {
        DrawCircleV(nodes[i].pos, 25, MAROON);
        DrawCircleLines(nodes[i].pos.x, nodes[i].pos.y, 25, BLACK);
        DrawText(TextFormat("%d", i), nodes[i].pos.x - 5, nodes[i].pos.y - 8, 20, WHITE);
    }
}

// מימוש כפתור Play/Stop אינטראקטיבי (שלב 3)
bool DrawButton(Rectangle bounds, const char* text, bool active) {
    Vector2 mousePoint = GetMousePosition();
    bool clicked = false;

    if (CheckCollisionPointRec(mousePoint, bounds)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) clicked = true;
        DrawRectangleRec(bounds, LIGHTGRAY);
    } else {
        DrawRectangleRec(bounds, active ? LIME : GRAY);
    }

    DrawRectangleLinesEx(bounds, 2, BLACK);
    DrawText(text, bounds.x + 10, bounds.y + 10, 20, BLACK);

    return clicked;
}

// לוגיקת תנועה ותזמון (שלב 3)
void UpdateEntity(Entity* entity, int num_nodes, VisualNode nodes[], int graph[15][15], int path[], int pathSize) {

    // מצב המתנה: עוצרים לשנייה אחת בכל צומת (דרישת פרויקט)
    if (entity->isWaiting) {
        entity->timer += GetFrameTime();
        if (entity->timer >= 1.0f) {
            entity->isWaiting = false;
            entity->timer = 0;
        }
        return;
    }

    // זיהוי צמתי הקשת הנוכחית במסלול
    int u = path[entity->startNode];
    int v = path[entity->endNode];
    int w = graph[u][v]; // משקל הקשת הקובע את מספר הקפיצות

    entity->timer += GetFrameTime();

    // כל קפיצה לוקחת בדיוק 300 מילישניות (דרישת פרויקט)
    if (entity->timer >= 0.3f) {
        entity->timer = 0;
        entity->currentJump++;

        if (entity->currentJump <= w) {
            // חישוב התקדמות ליניארית (Interpolation) בין הצמתים
            float t = (float)entity->currentJump / w;
            entity->currentPos = Vector2Lerp(nodes[u].pos, nodes[v].pos, t);
        } else {
            // סיום מעבר קשת: מעבר לצמתים הבאים במסלול
            entity->currentJump = 0;
            entity->startNode++;
            entity->endNode++;

            // אם הגענו לצומת שאינו היעד הסופי - נכנסים למצב המתנה
            if (entity->endNode < pathSize) {
                entity->isWaiting = true;
            }
        }
    }
}

// ציור הישות הזזה (שלב 3)
void DrawEntity(Entity entity) {
    DrawCircleV(entity.currentPos, 12, GOLD); // עיגול מוזהב מייצג את הרכב/נוסע
    DrawCircleLines(entity.currentPos.x, entity.currentPos.y, 12, BLACK);
}