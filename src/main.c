#include <stdio.h>

#define DEBUG 1

#include "simulation/simulation.h"
#include "simulation/debug.h"
#include "ui/ui.h"
#include "raylib.h"

#define BG_COLOR CLITERAL(Color){ 16, 14, 23, 255 }

int main() {
    InitWindow(1280, 720, "Logic Simulator");
    SetTargetFPS(60);

    sim_init();
    ui_init();

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BG_COLOR);

#ifdef DEBUG
        if(IsKeyPressed(KEY_D) && IsKeyDown(KEY_LEFT_CONTROL)) {
            simulation_print_debug(simulation);
        } else if(IsKeyPressed(KEY_D)) {
            TraceLog(LOG_INFO, "Simulation chips count: %lu", simulation.chips->count);
            TraceLog(LOG_INFO, "UI chips count: %lu", ui.chips->count);
        }
#endif

        if(IsKeyPressed(KEY_N)) {
            ui_add_chip(ui_chip_new(UI_CHIP_NAND, GetMousePosition()));
        }

        if(IsKeyPressed(KEY_I)) {
            ui_add_chip(ui_chip_new(UI_CHIP_INPUT, GetMousePosition()));
        }

        ui_update();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
