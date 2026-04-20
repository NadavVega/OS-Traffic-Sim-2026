#include "raylib.h"

int main(void) {
  InitWindow(800, 450, "OS Project - Test Window");

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Congrats Nadav! Raylib is working!", 190, 200, 20, LIGHTGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}