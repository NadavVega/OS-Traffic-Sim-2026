#include "raylib.h"
#include "gui.h"

int main(void) {
    // 1. אתחול חלון
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "The Schedulers - OS Traffic Simulation");
    SetTargetFPS(60);

    // 2. הכנת נתוני הגרף (שלב 2)
    int numNodes = 10;
    VisualNode nodes[15];
    InitGraphVisuals(numNodes, nodes);

    // נתוני דמי לבדיקת קשתות ומשקלים
    int testGraph[15][15] = {0};
    testGraph[0][1] = 10; // קשת מ-0 ל-1 עם משקל 10 (יקח 3 שניות)
    testGraph[1][2] = 5;  // קשת מ-1 ל-2 עם משקל 5 (יקח 1.5 שניות)

    // 3. הכנת נתוני האנימציה (שלב 3)
    bool animationRunning = false;
    Rectangle buttonBounds = { 20, 50, 120, 40 };

    // מסלול דמי לבדיקה: מ-0 ל-1 ואז ל-2
    int testPath[] = {0, 1, 2};
    int pathSize = 3;

    // אתחול הישות הזזה
    Entity myEntity = {0};
    myEntity.currentPos = nodes[testPath[0]].pos;
    myEntity.startNode = 0; // האינדקס במסלול (testPath) שממנו יוצאים
    myEntity.endNode = 1;   // האינדקס במסלול שאליו הולכים

    // לולאת המשחק/סימולציה
    while (!WindowShouldClose()) {
        // --- לוגיקה ---

        // בדיקת לחיצה על כפתור הפעלה/עצירה
        if (DrawButton(buttonBounds, animationRunning ? "STOP" : "PLAY", animationRunning)) {
            animationRunning = !animationRunning;
        }

        // עדכון תנועה רק אם האנימציה רצה ועדיין לא הגענו לסוף המסלול
        if (animationRunning && myEntity.endNode < pathSize) {
            UpdateEntity(&myEntity, numNodes, nodes, testGraph, testPath, pathSize);
        }

        // --- ציור ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // כותרת וסטטוס
        DrawText("Milestone 3: Traffic Animation Control", 20, 20, 20, DARKGRAY);
        DrawText(TextFormat("Status: %s", animationRunning ? "RUNNING" : "PAUSED"), 160, 60, 18,
                 animationRunning ? LIME : MAROON);

        // ציור הגרף (החלק הסטטי)
        DrawStaticGraph(numNodes, nodes, testGraph);

        // ציור הישות (החלק הדינמי)
        DrawEntity(myEntity);

        // ציור הכפתור (שיהיה תמיד מקדימה)
        DrawButton(buttonBounds, animationRunning ? "STOP" : "PLAY", animationRunning);

        EndDrawing();
    }

    // סגירה מסודרת
    CloseWindow();
    return 0;
}