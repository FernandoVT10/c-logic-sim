#ifndef GUI_SIMULATION_H
#define GUI_SIMULATION_H

// ----------------------- //
// Appearence of the chips //
// ----------------------- //
#define GUI_NAND_WIDTH 100
#define GUI_NAND_HEIGHT 40

#define GUI_PIN_RADIUS 8

#define GUI_INPUT_WIDTH 30
#define GUI_INPUT_HEIGHT 30
#define GUI_INPUT_LINE_WIDTH 5 // line width that is drawn after the square
#define GUI_INPUT_LINE_THICKNESS 5 // line thickness that is drawn after the square
#define GUI_INPUT_DRAGGABLE_WIDTH 8 // Little square before the square of the switch used to drag it
#define GUI_INPUT_DRAGGABLE_MARGIN 5 // margin between the draggable rectangle and the switch

#define GUI_WIRE_THICKNESS 5

#define GUI_OUTPUT_WIDTH 30
#define GUI_OUTPUT_HEIGHT 30

#include "raylib.h"
#include "../simulation.h"

typedef struct GUIChip GUIChip;

typedef struct {
    size_t id;
    bool isInput;
    GUIChip *parentChip;
    SimPin *simPin;
    // "global" position is calculated adding the parent position and this position
    Vector2 pos;
} GUIPin;

typedef struct {
    GUIPin *items;
    size_t count;
    size_t capacity;
} GUIPinArray;

typedef enum {
    GUI_CHIP_INPUT,
    GUI_CHIP_NAND,
    GUI_CHIP_OUTPUT,
} GUIChipType;

typedef struct {
    float offsetX;
    float offsetY;
    float width;
    float height;
} GUIChipCollider;

struct GUIChip {
    GUIChipType type;
    Vector2 pos;
    SimChip *simChip;
    GUIPinArray inputs;
    GUIPinArray outputs;

    struct {
        // these colliders are calculated relative to the chip position
        GUIChipCollider draggable;
        GUIChipCollider deletable;
    } colliders;
};

typedef struct {
    GUIPin *src;
    GUIPin *target;
} GUIWire;

void gui_sim_update(void);

void gui_sim_add_chip(GUIChip *chip);

void gui_sim_remove_chip(GUIChip *chip);

// ----------------- //
// GUIPin functions //
// ----------------- //
/*
 * It adds the pin's pos to the parentChip's pos and returns it.
 */
Vector2 gui_pin_get_pos(GUIPin *pin);

// ----------------- //
// GUIWire functions //
// ----------------- //
GUIWire *gui_wire_new();

void gui_wire_free(GUIWire *wire);

/*
 * Deletes all wires that have the pin in either "target" or "src" fields.
 */
void gui_wire_delete_by_pin(GUIPin *pin);

#endif // GUI_SIMULATION_H
