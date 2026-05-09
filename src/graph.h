//
// Created by balah on 09/05/2026.
//
#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

#define MAX_NODES 15 // מגבלת קודקודים

// מבנה עזר לשמירת קואורדינטות של צומת על המסך
typedef struct {
    int x;
    int y;
} Point2D;

// הגדרת מבנה הנתונים של הגרף
typedef struct {
    int num_nodes;
    int num_edges;
    int matrix[MAX_NODES][MAX_NODES];  // מטריצת שכנויות למשקלי הקשתות
    Point2D node_positions[MAX_NODES]; // מיקומי הצמתים לתצוגה הגרפית
} Graph;

// מצבי האנימציה האפשריים של הישות הזזה
typedef enum {
    ANIM_IDLE,          // האנימציה טרם התחילה או נעצרה לחלוטין
    ANIM_WAITING_NODE,  // הישות ממתינה שנייה אחת שלמה בצומת
    ANIM_MOVING_EDGE,   // הישות נמצאת במהלך תנועה על קשת
    ANIM_FINISHED       // הישות הגיעה ליעד (להצגת ההודעה על המסך)
} AnimStatus;

// מבנה הנתונים שמנהל את הישות הזזה על הגרף
typedef struct {
    int* path;              // המסלול (מועתק מהתוצאה של דייקסטרה)
    int path_length;        // כמות הקודקודים במסלול

    int current_path_index; // היכן הישות נמצאת כרגע (אינדקס במערך path)
    int current_jump;       // מונה קפיצות על הקשת הנוכחית (מתוך משקל הקשת W)

    float timer;            // טיימר פנימי מצטבר למעקב אחרי 300ms (קפיצה) או 1 שניה (המתנה)

    bool is_playing;        // דגל לשליטה על כפתור התחל/עצור (Play/Stop)
    AnimStatus status;      // מצב הישות הנוכחי
} AnimationState;

// --- ניהול גרף בסיסי ---
Graph* create_graph(int nodes, int edges);
void add_edge(Graph* g, int src, int dest, int weight);
void free_graph(Graph* g);

// פונקציה שתחשב ותפזר את הצמתים במרחב כך שלא יחפפו
void calculate_node_positions(Graph* g, int screen_width, int screen_height);

// מקצה את מבנה האנימציה ומאתחלת אותו עם המסלול שנמצא
AnimationState* init_animation(int* path, int path_length);

// משחררת את הזיכרון של האנימציה בסיום למניעת דליפות זיכרון
void free_animation(AnimationState* anim);

#endif