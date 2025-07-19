#include "simulation.h"

Simulation simulation = {0};

void sim_init(void) {
    simulation.chips = set_new();
}

void sim_add_chip(SimChip *chip) {
    set_add(simulation.chips, chip);
}

void sim_remove_chip(SimChip *chip) {
    set_delete(simulation.chips, chip);
}

static void chip_add_input_pin(SimChip *chip) {
    da_append(&chip->inputs, ((SimPin){
        .isInput = true,
        .parentChip = chip,
        .state = PIN_LOW,
        .connectedPins = set_new(),
    }));
}

static void chip_add_output_pin(SimChip *chip) {
    da_append(&chip->outputs, ((SimPin){
        .isInput = false,
        .parentChip = chip,
        .state = PIN_LOW,
        .connectedPins = set_new(),
    }));
}

SimChip *sim_chip_new(SimChipType type) {
    SimChip *chip = alloc(sizeof(SimChip));
    chip->type = type;

    switch(type) {
        case SIM_CHIP_NAND:
            chip_add_input_pin(chip);
            chip_add_input_pin(chip);
            chip_add_output_pin(chip);

            chip->outputs.items[0].state = PIN_HIGH;
            break;
        case SIM_CHIP_INPUT:
            chip_add_output_pin(chip);
            break;
        case SIM_CHIP_OUTPUT:
            chip_add_input_pin(chip);
            break;
    }

    return chip;
}

static void free_pin_array(SimPinArray *pinArr) {
    for(size_t i = 0; i < pinArr->count; i++) {
        SimPin pin = pinArr->items[i];
        set_clear_and_destroy(pin.connectedPins);
    }

    da_free(pinArr);
}

void sim_chip_free(SimChip *chip) {
    free_pin_array(&chip->inputs);
    free_pin_array(&chip->outputs);
    free(chip);
}

static SimPin *get_pin_from_arr(SimPinArray pinArr, size_t index) {
    if(pinArr.count == 0 || index > pinArr.count - 1) {
        return NULL;
    }

    return &pinArr.items[index];
}

SimPin *sim_chip_get_input_pin(SimChip *chip, size_t index) {
    return get_pin_from_arr(chip->inputs, index);
}

SimPin *sim_chip_get_output_pin(SimChip *chip, size_t index) {
    return get_pin_from_arr(chip->outputs, index);
}

static void update_pin_state(SimPin *pin, SimPinState state);

static void update_chip_state(SimChip *chip) {
    switch(chip->type) {
        case SIM_CHIP_NAND:
            SimPin *output = &chip->outputs.items[0];
            SimPinState state = !(chip->inputs.items[0].state & chip->inputs.items[1].state);
            if(state != output->state) {
                update_pin_state(output, state);
            }
            break;
        default: break;
    }
}

static void update_pin_state(SimPin *pin, SimPinState state) {
    pin->state = state;

    if(pin->isInput) {
        update_chip_state(pin->parentChip);
    } else {
        SetItem *item = pin->connectedPins->head;
        while(item != NULL) {
            SimPin *pin = item->data;
            update_pin_state(pin, state);
            item = item->next;
        }
    }
}

bool sim_chip_toggle_output_pin(SimChip *chip, size_t index) {
    SimPin *pin = sim_chip_get_output_pin(chip, index);
    if(pin == NULL) return false;
    SimPinState newState = pin->state == PIN_HIGH ? PIN_LOW : PIN_HIGH;
    update_pin_state(pin, newState);
    return true;
}

bool sim_pin_add_connection(SimPin *src, SimPin *target) {
    if(src->isInput) {
        // TODO: implement a good logger
        printf("[ERROR] Target pin is an Input Pin");
        return false;
    }

    update_pin_state(target, src->state);
    set_add(src->connectedPins, target);
    return true;
}

bool sim_pin_remove_connection(SimPin *src, SimPin *target) {
    update_pin_state(target, PIN_LOW);
    return set_delete(src->connectedPins, target);
}

bool sim_pin_is_high(SimPin *pin) {
    return pin->state == PIN_HIGH;
}
