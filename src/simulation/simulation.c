#include "simulation.h"

static void update_pin_state(SimPin *pin, SimPinState state);

static SimPin *get_pin_from_arr(SimPinArray pinArr, size_t index) {
    if(pinArr.count == 0 || index > pinArr.count - 1) {
        return NULL;
    }

    return &pinArr.items[index];
}

static void update_chip_state(SimChip *chip) {
    switch(chip->type) {
        case SIM_CHIP_NAND:
            SimPin *output = &chip->outputs.items[0];
            SimPinState state = !(chip->inputs.items[0].state & chip->inputs.items[1].state);
            update_pin_state(output, state);
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

static void chip_add_pin(SimPinArray *pinArr, SimChip *parentChip, bool isInput) {
    da_append(pinArr, ((SimPin){
        .isInput = isInput,
        .parentChip = parentChip,
        .state = PIN_LOW,
        .connectedPins = set_new(),
    }));
}

Simulation simulation = {0};

void sim_init(void) {
    simulation.chips = set_new();
}

void sim_add_chip(SimChip *chip) {
    set_add(simulation.chips, chip);
}

SimChip *sim_chip_new(SimChipType type) {
    SimChip *chip = alloc(sizeof(SimChip));
    chip->type = type;

    switch(type) {
        case SIM_CHIP_NAND:
            chip_add_pin(&chip->inputs, chip, true);
            chip_add_pin(&chip->inputs, chip, true);
            chip_add_pin(&chip->outputs, chip, false);

            chip->outputs.items[0].state = PIN_HIGH;
            break;
        case SIM_CHIP_INPUT:
            chip_add_pin(&chip->outputs, chip, false);
            break;
    }

    return chip;
}

SimPin *sim_chip_get_input_pin(SimChip *chip, size_t index) {
    return get_pin_from_arr(chip->inputs, index);
}

SimPin *sim_chip_get_output_pin(SimChip *chip, size_t index) {
    return get_pin_from_arr(chip->outputs, index);
}

bool sim_chip_toggle_output_pin(SimChip *chip, size_t index) {
    SimPin *pin = sim_chip_get_output_pin(chip, index);
    if(pin == NULL) return false;
    SimPinState newState = pin->state == PIN_HIGH ? PIN_LOW : PIN_HIGH;
    update_pin_state(pin, newState);
    return true;
}

void sim_pin_add_connection(SimPin *src, SimPin *target) {
    if(src->isInput) {
        // TODO: implement a good logger
        printf("[ERROR] Target pin is an Input Pin");
        return;
    }

    update_pin_state(target, src->state);
    set_add(src->connectedPins, target);
}
