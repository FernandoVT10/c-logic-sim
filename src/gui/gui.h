#ifndef GUI_H
#define GUI_H

#include "gui_simulation.h"
#include "../utils.h"

typedef enum {
    GUI_STATE_NONE,
    GUI_STATE_DRAGGING_CHIP,
    GUI_STATE_WIRING,
    GUI_STATE_DELETING_CHIP,
} GUIState;

typedef struct {
    Set *chips;

    GUIState state;
    GUIChip *draggingChip;
    GUIChip *chipToDelete;

    Set *wires;
    // used when we're wiring
    GUIWire *currentWire;
} GUI;

extern GUI gui;

/*
 * Initializes the "gui" variable. Should be called before any "gui" function.
 */
void gui_init();

void gui_update();

#endif // GUI_H
