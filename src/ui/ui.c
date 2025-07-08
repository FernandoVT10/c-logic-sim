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

    Vector2 mousePos = GetMousePosition();

    // Dragging
    Rectangle draggableCollider = get_rec_from_collider_and_vec(chip->colliders.draggable, chip->pos);
    if(CheckCollisionPointRec(mousePos, draggableCollider)
        && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
    ) {
        ui_drag_chip(chip);
    }

    // Deleting
    Rectangle deletableCollider = get_rec_from_collider_and_vec(chip->colliders.deletable, chip->pos);
    if(CheckCollisionPointRec(mousePos, deletableCollider)
        && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)
        && ui.state == UI_STATE_NONE
    ) {
        ui.chipToDelete = chip;
        ui.state = UI_STATE_DELETING;
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

static void handle_deleting() {
    if(ui.state != UI_STATE_DELETING) return;
    if(ui.chipToDelete == NULL) {
        TraceLog(LOG_ERROR, "Deleting a NULL chip?");
        return;
    }

    ui_delete_chip(ui.chipToDelete);
    ui.chipToDelete = NULL;
    ui.state = UI_STATE_NONE;
}

static void update_wires() {
    // wire deletion
    SetItem *wireItem = ui.wires->head;
    Vector2 mousePos = GetMousePosition();
    while(wireItem != NULL) {
        UIWire *wire = wireItem->data;
        // here we reassing wireItem right away since the wireItem can be deleted below
        wireItem = wireItem->next;

        Vector2 startPos = ui_get_pin_pos(wire->src.pin);
        Vector2 endPos = ui_get_pin_pos(wire->target.pin);
        if(CheckCollisionPointLine(mousePos, startPos, endPos, 5)
            && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            ui_wire_delete(wire);
            continue;
        }
    }

    draw_wires(ui.wires);

    // draw unfinised wire
    if(ui.state == UI_STATE_WIRING) {
        draw_unfinished_wire(ui.currentWire);

        // cancel wiring
        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            ui.state = UI_STATE_NONE;
            ui_wire_free(ui.currentWire);
        }
    }
}

// deletes all the wires that contains any pin in the array in either the src or target fields
static void delete_wires_from_pin_array(UIPinArray pinArr) {
    for(size_t i = 0; i < pinArr.count; i++) {
        UIPin *pin = &pinArr.items[i];

        SetItem *wireItem = ui.wires->head;
        while(wireItem != NULL) {
            UIWire *wire = wireItem->data;
            // wireItem will be freed when ui_wire_delete is called
            wireItem = wireItem->next;
            if(wire->src.pin == pin || wire->target.pin == pin) {
                ui_wire_delete(wire);
            }
        }
    }
}

UI ui = {0};

void ui_init() {
    ui.chips = set_new();
    ui.wires = set_new();
}

void ui_update() {
    update_wires();
    update_chips();
    handle_dragging();
    handle_deleting();

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

void ui_delete_chip(UIChip *chip) {
    delete_wires_from_pin_array(chip->inputs);
    delete_wires_from_pin_array(chip->outputs);
    sim_remove_chip(chip->simChip);
    set_delete(ui.chips, chip);
    ui_chip_free(chip);
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
            chip->colliders.deletable.width = UI_INPUT_DRAGGABLE_WIDTH + UI_INPUT_DRAGGABLE_MARGIN + UI_INPUT_WIDTH;
            chip->colliders.deletable.height = UI_INPUT_HEIGHT;

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

            chip->colliders.deletable.width = UI_NAND_WIDTH;
            chip->colliders.deletable.height = UI_NAND_HEIGHT;

            add_pin_to_chip(chip, true, (Vector2){0, UI_PIN_RADIUS});
            add_pin_to_chip(chip, true, (Vector2){0, UI_NAND_HEIGHT - UI_PIN_RADIUS});
            add_pin_to_chip(chip, false, (Vector2){UI_NAND_WIDTH, UI_NAND_HEIGHT/2});

            break;
        case UI_CHIP_OUTPUT:
            chip->simChip = sim_chip_new(SIM_CHIP_OUTPUT);

            chip->colliders.draggable.width = UI_OUTPUT_WIDTH;
            chip->colliders.draggable.height = UI_OUTPUT_HEIGHT;

            chip->colliders.deletable.width = UI_OUTPUT_WIDTH;
            chip->colliders.deletable.height = UI_OUTPUT_HEIGHT;

            add_pin_to_chip(chip, true, (Vector2){0, UI_OUTPUT_HEIGHT / 2});

            break;
    }

    sim_add_chip(chip->simChip);

    return chip;
}

void ui_chip_free(UIChip *chip) {
    da_free(&chip->inputs);
    da_free(&chip->outputs);
    sim_chip_free(chip->simChip);
    free(chip);
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

void ui_wire_delete(UIWire *wire) {
    set_delete(ui.wires, wire);
    sim_pin_remove_connection(
        wire->src.pin->simPin,
        wire->target.pin->simPin
    );
    ui_wire_free(wire);
}
