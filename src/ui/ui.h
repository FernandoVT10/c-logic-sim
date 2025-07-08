#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "../utils.h"
#include "../simulation/simulation.h"

#define UI_NAND_WIDTH 100
#define UI_NAND_HEIGHT 40

#define UI_PIN_RADIUS 8

#define UI_INPUT_WIDTH 30
#define UI_INPUT_HEIGHT 30
#define UI_INPUT_LINE_WIDTH 5 // line width that is drawn after the square
#define UI_INPUT_LINE_THICKNESS 5 // line thickness that is drawn after the square
#define UI_INPUT_DRAGGABLE_WIDTH 8 // Little square before the square of the switch used to drag it
#define UI_INPUT_DRAGGABLE_MARGIN 5 // margin between the draggable rectangle and the switch

#define UI_WIRE_THICKNESS 5

#define UI_OUTPUT_WIDTH 30
#define UI_OUTPUT_HEIGHT 30

typedef struct UIChip UIChip;

typedef struct {
    size_t id;
    bool isInput;
    UIChip *parentChip;
    SimPin *simPin;
    // "global" position is calculated adding the parent position and this position
    Vector2 pos;
} UIPin;

typedef struct {
    UIPin *items;
    size_t count;
    size_t capacity;
} UIPinArray;

typedef enum {
    UI_CHIP_INPUT,
    UI_CHIP_NAND,
    UI_CHIP_OUTPUT,
} UIChipType;

typedef struct {
    float offsetX;
    float offsetY;
    float width;
    float height;
} UICollider;

struct UIChip {
    UIChipType type;
    Vector2 pos;
    SimChip *simChip;
    UIPinArray inputs;
    UIPinArray outputs;

    struct {
        // these colliders are calculated relative to the chip position
        UICollider draggable;
        UICollider deletable;
    } colliders;
};

typedef struct {
    UIPin *pin;
} ConnectionInfo;

typedef struct {
    ConnectionInfo src;
    ConnectionInfo target;
} UIWire;

typedef enum {
    UI_STATE_NONE,
    UI_STATE_DRAGGING,
    UI_STATE_WIRING,
    UI_STATE_DELETING,
} UIState;

typedef struct {
    Set *chips;

    UIState state;
    UIChip *draggingChip;
    UIChip *chipToDelete;

    Set *wires;
    // used when we're wiring
    UIWire *currentWire;

    // keeps track of the pin id previously created created
    size_t pinLastId;
} UI;

extern UI ui;

void ui_init();
void ui_update();
void ui_add_chip(UIChip *chip);
void ui_drag_chip(UIChip *chip);
void ui_delete_chip(UIChip *chip);

UIChip *ui_chip_new(UIChipType type, Vector2 initialPos);
void ui_chip_free(UIChip *chip);

// returns the actual pin pos in the world (adding its "pos" and its parent "pos")
Vector2 ui_get_pin_pos(UIPin *pin);
bool ui_is_pin_high(UIPin *pin);

UIWire *ui_wire_new();
void ui_wire_free(UIWire *wire);
// removes and frees a wire
void ui_wire_delete(UIWire *wire);

#endif // UI_H
