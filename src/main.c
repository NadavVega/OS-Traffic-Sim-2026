#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "graph.h"
#include "parser.h"
#include "dijkstra.h"
#include "gui.h"     // הפונקציות והמבנים של נווה
#include "raylib.h"  // ספריית הגרפיקה

int main(int argc, char *argv[]) {
    // ==========================================
    // 1. לוגיקה נוכחית (קריאה, בדיקות, דייקסטרה)
    // ==========================================
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int src, dest;
    Graph* graph = parse_graph_from_file(argv[1], &src, &dest);

    if (graph == NULL) {
        fprintf(stderr, "Error: Invalid input or negative weights detected.\n");
        return EXIT_FAILURE;
    }

    if (src < 0 || src >= graph->num_nodes || dest < 0 || dest >= graph->num_nodes) {
        fprintf(stderr, "Error: Source or destination node out of bounds.\n");
        free_graph(graph);
        return EXIT_FAILURE;
    }

    // יציאה מוקדמת אם מקור שווה ליעד (ללא אנימציה)
    if (src == dest) {
        printf("%d\n0\n", src);
        free_graph(graph);
        return EXIT_SUCCESS;
    }

    // הרצת האלגוריתם של נדב
    dijkstraResult result = find_shortest_path(graph->num_nodes, graph->matrix, src, dest);
    bool has_path = (result.path[0] != -1);

    // הדפסה לטרמינל
    if (!has_path) {
        printf("No path found\n");
    } else {
        for (int i = 0; i < result.path_length; i++) {
            printf("%d", result.path[i]);
            if (i < result.path_length - 1) {
                printf(" -> ");
            }
        }
        printf("\n%d\n", result.total_weight);
    }

    // ==========================================
    // 2. שילוב ה-GUI של נווה
    // ==========================================

    // פתיחת חלון גרפי
    InitWindow(800, 600, "Traffic Simulation 2026");
    SetTargetFPS(60);

    // אתחול הצמתים הגרפיים (פיזור במעגל)
    VisualNode vNodes[MAX_NODES];
    InitGraphVisuals(graph->num_nodes, vNodes);

    // אתחול הישות רק אם באמת נמצא מסלול
    Entity car = {0};
    if (has_path && result.path_length > 1) {
        car.currentPos = vNodes[result.path[0]].pos;
        car.startNode = 0; // האינדקס במסלול בו אנו מתחילים
        car.endNode = 1;   // האינדקס של הצומת הבא
        car.currentJump = 0;
        car.timer = 0.0f;
        car.isWaiting = true; // מתחילים בהמתנה בצומת הראשון
    }

    // לולאת המשחק המרכזית (רצה כל פרייד עד שהמשתמש סוגר את החלון)
    while (!WindowShouldClose()) {

        // א. עדכון הלוגיקה (Movement)
        if (has_path && result.path_length > 1) {
            // מעדכנים תנועה כל עוד לא סיימנו את כל המסלול
            if (car.endNode < result.path_length) {
                UpdateEntity(&car, graph->num_nodes, vNodes, graph->matrix, result.path, result.path_length);
            }
        }

        // ב. ציור המסך (Drawing)
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // ציור הגרף ברקע
        DrawStaticGraph(graph->num_nodes, vNodes, graph->matrix);

        // ציור הישות או הודעת שגיאה
        if (has_path && result.path_length > 1) {
            DrawEntity(car);
            // הדפסת המשקל הכולל למטה
            DrawText(TextFormat("Total Weight: %d", result.total_weight), 20, 550, 20, DARKGREEN);
        } else if (!has_path) {
            DrawText("No path found!", 300, 20, 20, RED);
        }

        EndDrawing();
    }

    // ==========================================
    // 3. סגירה וניקוי זיכרון
    // ==========================================
    CloseWindow();
    free_graph(graph);

    return EXIT_SUCCESS;
}