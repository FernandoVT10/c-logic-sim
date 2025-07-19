#include "gui.h"
#include "gui_simulation.h"
#include "gui_chip.h"
#include "draw.h"

#include "raymath.h"

static void update_chips(void) {
    SetItem *chipItem = gui.chips->head;
    while(chipItem != NULL) {
        GUIChip *chip = chipItem->data;
        // here we change the item right away since the chip can be removed
        // by the "update_chip" function
        chipItem = chipItem->next;
        gui_chip_update(chip);
    }
}

static void draw_chips(void) {
    SetItem *chipItem = gui.chips->head;
    while(chipItem != NULL) {
        GUIChip *chip = chipItem->data;
        gui_draw_chip(chip);
        chipItem = chipItem->next;
    }
}

static void handle_dragging_chip(void) {
    if(gui.draggingChip == NULL) {
        TraceLog(LOG_ERROR, "Dragging with no draggingChip?");
        return;
    }

    Vector2 delta = GetMouseDelta();
    gui.draggingChip->pos = Vector2Add(gui.draggingChip->pos, delta);

    // cancel dragging
    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        gui.state = GUI_STATE_NONE;
        gui.draggingChip = NULL;
    }
}

static void handle_wiring(void) {
    gui_draw_unfinished_wire(gui.currentWire);

    // cancel wiring
    if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        gui.state = GUI_STATE_NONE;
        gui_wire_free(gui.currentWire);
    }
}

void gui_sim_update(void) {
    gui_draw_wires(gui.wires);
    update_chips();

    switch(gui.state) {
        case GUI_STATE_DRAGGING_CHIP:
            handle_dragging_chip();
            break;
        case GUI_STATE_WIRING:
            handle_wiring();
            break;
        default: break;
    }

    // remember to draw the chips below the wires!
    draw_chips();
}

void gui_sim_add_chip(GUIChip *chip) {
    set_add(gui.chips, chip);
}

void gui_sim_remove_chip(GUIChip *chip) {
    set_delete(gui.chips, chip);
}

Vector2 gui_pin_get_pos(GUIPin *pin) {
    return Vector2Add(pin->parentChip->pos, pin->pos);
}

GUIWire *gui_wire_new() {
    return alloc(sizeof(GUIWire));
}

void gui_wire_free(GUIWire *wire) {
    free(wire);
}

void gui_wire_delete_by_pin(GUIPin *pin) {
    SetItem *wireItem = gui.wires->head;
    while(wireItem != NULL) {
        GUIWire *wire = wireItem->data;
        // wireItem will be freed when ui_wire_delete is called
        wireItem = wireItem->next;
        if(wire->src == pin || wire->target == pin) {
            // delete wire
            sim_pin_remove_connection(
                wire->src->simPin,
                wire->target->simPin
            );
            gui_wire_free(wire);
            set_delete(gui.wires, wire);
        }
    }
}
