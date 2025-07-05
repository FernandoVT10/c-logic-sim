typedef struct {
    int offsetX;
    int offsetY;
    int width;
    int height;
} Collider;

// Old Chip
struct Chip {
    ChipType type;
    PinArray inputs;
    PinArray outputs;

    Vector2 pos;

    struct {
        // this collider is relative to this chip
        // the real collider is calculated using the current position
        Collider drag;
        Collider delete;
    } colliders;

    void (*draw)(Chip *chip);
};

void simulator_drag_chip(Chip *chip) {
    if(simulator.state != STATE_NONE) return;

    simulator.state = STATE_DRAGGING;
    simulator.draggingChip = chip;
}

static void handle_dragging() {
    if(simulator.state != STATE_DRAGGING) return;
    if(simulator.draggingChip == NULL) {
        TraceLog(LOG_ERROR, "Dragging state with no chip?");
        return;
    }

    Vector2 delta = GetMouseDelta();
    simulator.draggingChip->pos = Vector2Add(simulator.draggingChip->pos, delta);

    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        simulator.state = STATE_NONE;
        simulator.draggingChip = NULL;
    }
}

static void handle_deletion() {
    if(simulator.chipToDelete != NULL) {
        set_delete(simulator.chips, simulator.chipToDelete);
        chip_free(simulator.chipToDelete);
        simulator.chipToDelete = NULL;
    }
}

static void simulator_draw_chip(Chip *chip) {
    draw_pin_array(chip->inputs);
    draw_pin_array(chip->outputs);

    chip->draw(chip);
}

void simulator_update(void) {
    handle_dragging();

    SetItem *chipItem = simulator.chips->head;
    while(chipItem != NULL) {
        Chip *chip = chipItem->data;

        chip_update(chip);

        BeginMode2D(simulator.camera);
        simulator_draw_chip(chip);
        EndMode2D();

        chipItem = chipItem->next;
    }

    handle_deletion();
}

// calculates the rectangle from a collider and a position(vec)
// NOTE: the collider is relative to the pos
static Rectangle get_rec_from_collider_and_vec(Collider collider, Vector2 vec) {
    return (Rectangle) {
        .x = vec.x + collider.offsetX,
        .y = vec.y + collider.offsetY,
        .width = collider.width,
        .height = collider.height,
    };
}

static void chip_handle_drag(Chip *chip) {
    Vector2 mousePos = GetMousePosition();

    Rectangle colliderRec = get_rec_from_collider_and_vec(chip->colliders.drag, chip->pos);
    if(CheckCollisionPointRec(mousePos, colliderRec)
        && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
    ) {
        simulator_drag_chip(chip);
    }
}

static void chip_handle_delete(Chip *chip) {
    Vector2 mousePos = GetMousePosition();

    Rectangle colliderRec = get_rec_from_collider_and_vec(chip->colliders.delete, chip->pos);
    if(CheckCollisionPointRec(mousePos, colliderRec)
        && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)
    ) {
        simulator_delete_chip(chip);
    }
}

static void chip_debug(Chip *chip) {
    Vector2 mousePos = GetMousePosition();

    // here i'm using the drag collider for simplicity
    Rectangle colliderRec = get_rec_from_collider_and_vec(chip->colliders.drag, chip->pos);
    if(IsKeyPressed(KEY_D) && CheckCollisionPointRec(mousePos, colliderRec)) {
        for(size_t i = 0; i < chip->inputs.count; i++) {
            Pin pin = chip->inputs.items[i];
            char *status = pin.state == PIN_ON ? "ON" : "OFF";
            printf("Input Pin [#%lu] is [%s]\n", i, status);
        }

        for(size_t i = 0; i < chip->outputs.count; i++) {
            Pin pin = chip->outputs.items[i];
            char *status = pin.state == PIN_ON ? "ON" : "OFF";
            printf("Output Pin [#%lu] is [%s]\n", i, status);
        }
    }
}

static void chip_update_input(Chip *input) {
    Rectangle collider = {
        .x = input->pos.x + INPUT_DRAGGABLE_WIDTH + INPUT_DRAGGABLE_MARGIN,
        .y = input->pos.y,
        .width = INPUT_WIDTH,
        .height = INPUT_HEIGHT,
    };
    bool collision = CheckCollisionPointRec(GetMousePosition(), collider);
    if(collision && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Pin *pin = &input->outputs.items[0];
        PinState state = pin->state == PIN_ON ? PIN_OFF : PIN_ON;
        pin_update_state(pin, state);
    }
}

void chip_update(Chip *chip) {
    chip_debug(chip);
    chip_handle_drag(chip);
    chip_handle_delete(chip);

    if(chip->type == INPUT) {
        chip_update_input(chip);
    }
}

Chip *chip_new(ChipType type, Vector2 pos) {
    Chip *chip = alloc(sizeof(Chip));
    chip->type = type;
    chip->pos = pos;

    switch(type) {
        case NAND:
            chip->draw = &draw_nand;
            chip->colliders.drag.width = NAND_WIDTH;
            chip->colliders.drag.height = NAND_HEIGHT;

            chip->colliders.delete.width = NAND_WIDTH;
            chip->colliders.delete.height = NAND_HEIGHT;

            chip_add_pin(&chip->inputs, chip, true, (Vector2){0, PIN_RADIUS});
            chip_add_pin(&chip->inputs, chip, true, (Vector2){0, NAND_HEIGHT - PIN_RADIUS});
            chip_add_pin(&chip->outputs, chip, false, (Vector2){NAND_WIDTH, NAND_HEIGHT/2});

            chip->outputs.items[0].state = PIN_ON;
            break;
        case INPUT:
            chip->draw = &draw_input;

            chip->colliders.drag.width = INPUT_DRAGGABLE_WIDTH;
            chip->colliders.drag.height = INPUT_HEIGHT;

            Vector2 pinPos = {
                .x = INPUT_DRAGGABLE_WIDTH + INPUT_DRAGGABLE_MARGIN + INPUT_WIDTH + INPUT_LINE_WIDTH + PIN_RADIUS,
                .y = INPUT_HEIGHT / 2,
            };
            chip_add_pin(&chip->outputs, chip, false, pinPos);
            break;
    }

    return chip;
}
