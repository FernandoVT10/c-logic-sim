#ifndef GUI_CHIP_H
#define GUI_CHIP_H

#include "gui_simulation.h"

GUIChip *gui_chip_new(GUIChipType type, Vector2 initialPos);

void gui_chip_free(GUIChip *chip);

void gui_chip_update(GUIChip *chip);

#endif // GUI_CHIP_H
