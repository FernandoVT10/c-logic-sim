#include "gui.h"

GUI gui = {0};

void gui_init() {
    gui.chips = set_new();
    gui.wires = set_new();
}

void gui_update() {
    gui_sim_update();
}
