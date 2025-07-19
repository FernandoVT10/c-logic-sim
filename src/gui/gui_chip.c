#include "gui.h"
#include "gui_simulation.h"

/*
 * These functions connect the graphical pin with the simulated one.
 * Both functions "panic" when you try to add more graphical pins
 * that simulated ones.
*/
const char *excessOfGraphicalPinsError = "You adding more \"graphical\" pins than there are simulated pins";
static void chip_add_input_pin(GUIChip *chip, Vector2 initialPos) {
    GUIPin pin = {
        .isInput = true,
        .parentChip = chip,
        .pos = initialPos,
    };

    size_t index = chip->inputs.count;
    SimPin *simPin = sim_chip_get_input_pin(chip->simChip, index);
    if(simPin == NULL) {
        panic(excessOfGraphicalPinsError);
    }
    pin.simPin = simPin;
    da_append(&chip->inputs, pin);
}

static void chip_add_output_pin(GUIChip *chip, Vector2 initialPos) {
    GUIPin pin = {
        .isInput = false,
        .parentChip = chip,
        .pos = initialPos,
    };

    size_t index = chip->outputs.count;
    SimPin *simPin = sim_chip_get_output_pin(chip->simChip, index);
    if(simPin == NULL) {
        panic(excessOfGraphicalPinsError);
    }
    pin.simPin = simPin;
    da_append(&chip->outputs, pin);
}

GUIChip *gui_chip_new(GUIChipType type, Vector2 initialPos) {
    GUIChip *chip = alloc(sizeof(GUIChip));
    chip->type = type;
    chip->pos = initialPos;

    switch(type) {
        case GUI_CHIP_INPUT:
            chip->simChip = sim_chip_new(SIM_CHIP_INPUT);

            chip->colliders.draggable.width = GUI_INPUT_DRAGGABLE_WIDTH;
            chip->colliders.draggable.height = GUI_INPUT_HEIGHT;
            chip->colliders.deletable.width = GUI_INPUT_DRAGGABLE_WIDTH + GUI_INPUT_DRAGGABLE_MARGIN + GUI_INPUT_WIDTH;
            chip->colliders.deletable.height = GUI_INPUT_HEIGHT;

            Vector2 pos = {
                .x = GUI_INPUT_DRAGGABLE_WIDTH + GUI_INPUT_DRAGGABLE_MARGIN + GUI_INPUT_WIDTH + GUI_INPUT_LINE_WIDTH + GUI_PIN_RADIUS,
                .y = GUI_INPUT_HEIGHT / 2,
            };
            chip_add_output_pin(chip, pos);
            break;
        case GUI_CHIP_NAND:
            chip->simChip = sim_chip_new(SIM_CHIP_NAND);

            chip->colliders.draggable.width = GUI_NAND_WIDTH;
            chip->colliders.draggable.height = GUI_NAND_HEIGHT;

            chip->colliders.deletable.width = GUI_NAND_WIDTH;
            chip->colliders.deletable.height = GUI_NAND_HEIGHT;

            chip_add_input_pin(chip, (Vector2){0, GUI_PIN_RADIUS});
            chip_add_input_pin(chip, (Vector2){0, GUI_NAND_HEIGHT - GUI_PIN_RADIUS});
            chip_add_output_pin(chip, (Vector2){GUI_NAND_WIDTH, GUI_NAND_HEIGHT/2});

            break;
        case GUI_CHIP_OUTPUT:
            chip->simChip = sim_chip_new(SIM_CHIP_OUTPUT);

            chip->colliders.draggable.width = GUI_OUTPUT_WIDTH;
            chip->colliders.draggable.height = GUI_OUTPUT_HEIGHT;

            chip->colliders.deletable.width = GUI_OUTPUT_WIDTH;
            chip->colliders.deletable.height = GUI_OUTPUT_HEIGHT;

            chip_add_input_pin(chip, (Vector2){0, GUI_OUTPUT_HEIGHT / 2});

            break;
    }

    sim_add_chip(chip->simChip);

    return chip;
}

void gui_chip_free(GUIChip *chip) {
    da_free(&chip->inputs);
    da_free(&chip->outputs);
    sim_chip_free(chip->simChip);
    free(chip);
}

static bool can_finish_wiring(GUIPin *pin) {
    if(pin->isInput) {
        // if the pin is an input, the wire target's pin should be null
        return gui.currentWire->target == NULL;
    } else {
        // else the wire source's pin should be null
        return gui.currentWire->src == NULL;
    }
}

static void handle_wiring(GUIPin *pin) {
    if(gui.state == GUI_STATE_WIRING) {
        // finish wiring
        if(!can_finish_wiring(pin)) return;
        gui.state = GUI_STATE_NONE;

        if(pin->isInput) {
            gui.currentWire->target = pin;
        } else {
            gui.currentWire->src = pin;
        }

        sim_pin_add_connection(
            gui.currentWire->src->simPin,
            gui.currentWire->target->simPin
        );

        set_add(gui.wires, gui.currentWire);
        gui.currentWire = NULL;
    } else {
        // start wiring
        if(gui.state != GUI_STATE_NONE) return;
        gui.state = GUI_STATE_WIRING;
        gui.currentWire = gui_wire_new();

        if(pin->isInput) {
            gui.currentWire->target = pin;
        } else {
            gui.currentWire->src = pin;
        }
    }
}

static void update_pin_array(GUIPinArray arr) {
    Vector2 mousePos = GetMousePosition();

    for(size_t i = 0; i < arr.count; i++) {
        GUIPin *pin = &arr.items[i];

        Vector2 pos = gui_pin_get_pos(pin);
        if(CheckCollisionPointCircle(mousePos, pos, GUI_PIN_RADIUS)
            && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        ) {
            handle_wiring(pin);
        }
    }
}

// calculates the rectangle from a collider and a position(vec)
// NOTE: the collider is relative to the pos
static Rectangle get_rec_from_collider_and_vec(GUIChipCollider collider, Vector2 vec) {
    return (Rectangle) {
        .x = vec.x + collider.offsetX,
        .y = vec.y + collider.offsetY,
        .width = collider.width,
        .height = collider.height,
    };
}

// deletes all the wires that contains any pin in the array in either the src or target fields
static void delete_wires_from_pin_array(GUIPinArray pinArr) {
    for(size_t i = 0; i < pinArr.count; i++) {
        GUIPin *pin = &pinArr.items[i];

        gui_wire_delete_by_pin(pin);
    }
}

static void delete_chip(GUIChip *chip) {
    delete_wires_from_pin_array(chip->inputs);
    delete_wires_from_pin_array(chip->outputs);
    sim_remove_chip(chip->simChip);
    gui_sim_remove_chip(chip);
    gui_chip_free(chip);
}

void gui_chip_update(GUIChip *chip) {
    update_pin_array(chip->inputs);
    update_pin_array(chip->outputs);

    // Chip specific logic
    switch(chip->type) {
        case GUI_CHIP_INPUT: {
            Rectangle collider = {
                .x = chip->pos.x + GUI_INPUT_DRAGGABLE_WIDTH + GUI_INPUT_DRAGGABLE_MARGIN,
                .y = chip->pos.y,
                .width = GUI_INPUT_WIDTH,
                .height = GUI_INPUT_HEIGHT,
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
        && gui.state == GUI_STATE_NONE
    ) {
        gui.state = GUI_STATE_DRAGGING_CHIP;
        gui.draggingChip = chip;
    }

    // Deleting
    Rectangle deletableCollider = get_rec_from_collider_and_vec(chip->colliders.deletable, chip->pos);
    if(CheckCollisionPointRec(mousePos, deletableCollider)
        && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)
        && gui.state == GUI_STATE_NONE
    ) {
        delete_chip(chip);
    }
}
