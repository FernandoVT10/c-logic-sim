#include <stdio.h>

#define DEBUG 1

#include "simulation.h"
#include "simulation_debug.h"
#include "gui/gui.h"
#include "gui/gui_chip.h"
#include "raylib.h"

#define BG_COLOR CLITERAL(Color){ 16, 14, 23, 255 }

int main() {
    InitWindow(1280, 720, "Logic Simulator");
    SetTargetFPS(60);

    sim_init();
    gui_init();

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BG_COLOR);

#ifdef DEBUG
        if(IsKeyPressed(KEY_D) && IsKeyDown(KEY_LEFT_CONTROL)) {
            sim_debug_print(simulation);
        } else if(IsKeyPressed(KEY_D)) {
            TraceLog(LOG_INFO, "Simulation chips count: %lu", simulation.chips->count);
            TraceLog(LOG_INFO, "GUI chips count: %lu", gui.chips->count);
        }
#endif

        if(IsKeyPressed(KEY_N)) {
            gui_sim_add_chip(gui_chip_new(GUI_CHIP_NAND, GetMousePosition()));
        }

        if(IsKeyPressed(KEY_I)) {
            gui_sim_add_chip(gui_chip_new(GUI_CHIP_INPUT, GetMousePosition()));
        }

        if(IsKeyPressed(KEY_O)) {
            gui_sim_add_chip(gui_chip_new(GUI_CHIP_OUTPUT, GetMousePosition()));
        }

        gui_update();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
