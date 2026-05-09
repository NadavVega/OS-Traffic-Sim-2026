
// Created by נוה תורן on 09/05/2026.

#ifndef GUI_H
#define GUI_H

#include "raylib.h"

// מבנה לייצוג ויזואלי של צומת על המסך
typedef struct {
    Vector2 pos; // מיקום X ו-Y בפיקסלים
    int id;      // מספר הצומת (0 עד 14) [cite: 80]
} VisualNode;

// פונקציה לחישוב מיקומי הצמתים במעגל כדי להבטיח קריאות [cite: 80]
void InitGraphVisuals(int num_nodes, VisualNode nodes[]);

// פונקציה לציור הגרף הסטטי (צמתים, קשתות, חצים ומשקלים) [cite: 76, 79]
void DrawStaticGraph(int num_nodes, VisualNode nodes[], int graph[15][15]);

bool DrawButton(Rectangle bounds, const char* text, bool active);

// מבנה לניהול מצב הישות הזזה
typedef struct {
    Vector2 currentPos;   // מיקום נוכחי בפיקסלים
    int startNode;        // צומת מוצא של הקשת הנוכחית
    int endNode;          // צומת יעד של הקשת הנוכחית
    int currentJump;      // באיזו קפיצה אנחנו מתוך W
    float timer;          // טיימר פנימי למדידת זמן הקפיצה
    bool isWaiting;       // האם הישות בהמתנה של שנייה בצומת
} Entity;

// פונקציה לעדכון תנועת הישות
void UpdateEntity(Entity* entity, int num_nodes, VisualNode nodes[], int graph[15][15], int path[], int pathSize);

// פונקציה לציור הישות
void DrawEntity(Entity entity);


#endif // GUI_H