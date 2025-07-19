#ifndef DRAW_H
#define DRAW_H

#include "gui.h"

void gui_draw_chip(GUIChip *chip);
void gui_draw_wires(Set *wires);
void gui_draw_unfinished_wire(GUIWire *wire);

#endif // DRAW_H
