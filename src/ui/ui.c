#include <assert.h>
#include "ui.h"
#include "draw.h"
#include "raymath.h"

static bool can_finish_wiring(UIPin *pin) {
    if(pin->isInput) {
        // if the pin is an input, the wire target's pin should be null
        return ui.currentWire->target.pin == NULL;
    } else {
        // else the wire source's pin should be null
        return ui.currentWire->src.pin == NULL;
    }
}

static void handle_wiring(UIPin *pin) {
    if(ui.state == UI_STATE_WIRING) {
        // finish wiring
        if(!can_finish_wiring(pin)) return;
        ui.state = UI_STATE_NONE;

        if(pin->isInput) {
            ui.currentWire->target.pin = pin;
        } else {
            ui.currentWire->src.pin = pin;
        }

        sim_pin_add_connection(
            ui.currentWire->src.pin->simPin,
            ui.currentWire->target.pin->simPin
        );

        set_add(ui.wires, ui.currentWire);
        ui.currentWire = NULL;
    } else {
        // start wiring
        if(ui.state != UI_STATE_NONE) return;
        ui.state = UI_STATE_WIRING;
        ui.currentWire = ui_wire_new();

        if(pin->isInput) {
            ui.currentWire->target.pin = pin;
        } else {
            ui.currentWire->src.pin = pin;
        }
    }
}

// calculates the rectangle from a collider and a position(vec)
// NOTE: the collider is relative to the pos
static Rectangle get_rec_from_collider_and_vec(UICollider collider, Vector2 vec) {
    return (Rectangle) {
        .x = vec.x + collider.offsetX,
        .y = vec.y + collider.offsetY,
        .width = collider.width,
        .height = collider.height,
    };
}

static void update_pin(UIPin *pin) {
    Vector2 pos = ui_get_pin_pos(pin);
    Vector2 mousePos = GetMousePosition();

    if(CheckCollisionPointCircle(mousePos, pos, UI_PIN_RADIUS)
        && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
    ) {
        handle_wiring(pin);
    }
}

static void update_pin_array(UIPinArray arr) {
    for(size_t i = 0; i < arr.count; i++) {
        update_pin(&arr.items[i]);
    }
}

static void update_chip(UIChip *chip) {
    update_pin_array(chip->inputs);
    update_pin_array(chip->outputs);

    // Chip specific logic
    switch(chip->type) {
        case UI_CHIP_INPUT: {
            Rectangle collider = {
                .x = chip->pos.x + UI_INPUT_DRAGGABLE_WIDTH + UI_INPUT_DRAGGABLE_MARGIN,
                .y = chip->pos.y,
                .width = UI_INPUT_WIDTH,
                .height = UI_INPUT_HEIGHT,
            };
            bool collision = CheckCollisionPointRec(GetMousePosition(), collider);
            if(collision && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if(!sim_chip_toggle_output_pin(chip->simChip, 0)) {
                    panic("Output Pin not found");
                }
            }
        } break;
        default: break;
    }

    // Dragging
    Vector2 mousePos = GetMousePosition();

    Rectangle draggableCollider = get_rec_from_collider_and_vec(chip->colliders.draggable, chip->pos);
    if(CheckCollisionPointRec(mousePos, draggableCollider)
        && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
    ) {
        ui_drag_chip(chip);
    }
}

static void add_pin_to_chip(UIChip *chip, bool isInput, Vector2 initialPos) {
    UIPin pin = {
        .id = ++ui.pinLastId,
        .isInput = isInput,
        .parentChip = chip,
        .pos = initialPos,
    };

    if(isInput) {
        size_t index = chip->inputs.count;
        SimPin *simPin = sim_chip_get_input_pin(chip->simChip, index);
        if(simPin == NULL) {
            // this message is not useful, but I don't know what to write :)
            panic("Pin is NULL");
        }
        pin.simPin = simPin;
        da_append(&chip->inputs, pin);
    } else {
        size_t index = chip->outputs.count;
        SimPin *simPin = sim_chip_get_output_pin(chip->simChip, index);
        if(simPin == NULL) {
            // this message is not useful, but I don't know what to write :)
            panic("Pin is NULL");
        }
        pin.simPin = simPin;
        da_append(&chip->outputs, pin);
    }
}

static void update_chips() {
    SetItem *chipItem = ui.chips->head;
    while(chipItem != NULL) {
        UIChip *chip = chipItem->data;
        update_chip(chip);
        chipItem = chipItem->next;
    }

    chipItem = ui.chips->head;
    while(chipItem != NULL) {
        UIChip *chip = chipItem->data;
        draw_chip(chip);
        chipItem = chipItem->next;
    }
}

static void handle_dragging() {
    if(ui.state != UI_STATE_DRAGGING) return;
    if(ui.draggingChip == NULL) {
        TraceLog(LOG_ERROR, "Dragging with no draggingChip?");
        return;
    }

    Vector2 delta = GetMouseDelta();
    ui.draggingChip->pos = Vector2Add(ui.draggingChip->pos, delta);

    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        ui.state = UI_STATE_NONE;
        ui.draggingChip = NULL;
    }
}

UI ui = {0};

void ui_init() {
    ui.chips = set_new();
    ui.wires = set_new();
}

void ui_update() {
    draw_wires(ui.wires);
    if(ui.state == UI_STATE_WIRING) {
        draw_unfinished_wire(ui.currentWire);

        // cancel wiring
        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            ui.state = UI_STATE_NONE;
            ui_wire_free(ui.currentWire);
        }
    }
    update_chips();
    handle_dragging();

    if(IsKeyPressed(KEY_D)) {
        TraceLog(LOG_INFO, "Wires Count: %lu", ui.wires->count);
        const char *state = ui.state == UI_STATE_WIRING ? "true" : "false";
        TraceLog(LOG_INFO, "Wiring: [%s]", state);
    }
}

void ui_add_chip(UIChip *chip) {
    set_add(ui.chips, chip);
}

void ui_drag_chip(UIChip *chip) {
    if(ui.state != UI_STATE_NONE) return;

    ui.state = UI_STATE_DRAGGING;
    ui.draggingChip = chip;
}

UIChip *ui_chip_new(UIChipType type, Vector2 initialPos) {
    UIChip *chip = alloc(sizeof(UIChip));
    chip->type = type;
    chip->pos = initialPos;

    switch(type) {
        case UI_CHIP_INPUT:
            chip->simChip = sim_chip_new(SIM_CHIP_INPUT);

            chip->colliders.draggable.width = UI_INPUT_DRAGGABLE_WIDTH;
            chip->colliders.draggable.height = UI_INPUT_HEIGHT;

            // chip->colliders.delete.width = INPUT_DRAGGABLE_WIDTH + INPUT_DRAGGABLE_MARGIN + INPUT_WIDTH;
            // chip->colliders.delete.height = INPUT_HEIGHT;

            Vector2 pos = {
                .x = UI_INPUT_DRAGGABLE_WIDTH + UI_INPUT_DRAGGABLE_MARGIN + UI_INPUT_WIDTH + UI_INPUT_LINE_WIDTH + UI_PIN_RADIUS,
                .y = UI_INPUT_HEIGHT / 2,
            };
            add_pin_to_chip(chip, false, pos);
            break;
        case UI_CHIP_NAND:
            chip->simChip = sim_chip_new(SIM_CHIP_NAND);

            chip->colliders.draggable.width = UI_NAND_WIDTH;
            chip->colliders.draggable.height = UI_NAND_HEIGHT;

            // chip->colliders.delete.width = NAND_WIDTH;
            // chip->colliders.delete.height = NAND_HEIGHT;

            add_pin_to_chip(chip, true, (Vector2){0, UI_PIN_RADIUS});
            add_pin_to_chip(chip, true, (Vector2){0, UI_NAND_HEIGHT - UI_PIN_RADIUS});
            add_pin_to_chip(chip, false, (Vector2){UI_NAND_WIDTH, UI_NAND_HEIGHT/2});

            break;
    }

    sim_add_chip(chip->simChip);

    return chip;
}

Vector2 ui_get_pin_pos(UIPin *pin) {
    return Vector2Add(pin->parentChip->pos, pin->pos);
}

bool ui_is_pin_high(UIPin *pin) {
    return pin->simPin->state == PIN_HIGH;
}

UIWire *ui_wire_new() {
    return alloc(sizeof(UIWire));
}

void ui_wire_free(UIWire *wire) {
    free(wire);
}
