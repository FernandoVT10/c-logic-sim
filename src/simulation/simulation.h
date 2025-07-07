#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdbool.h>
#include "../utils.h"

typedef struct SimChip SimChip;

typedef enum {
    PIN_LOW = 0,
    PIN_HIGH,
} SimPinState;

typedef struct {
    bool isInput;
    SimChip *parentChip;
    Set *connectedPins;
    SimPinState state;
} SimPin;

typedef struct {
    SimPin *items;
    size_t count;
    size_t capacity;
} SimPinArray;

typedef enum {
    SIM_CHIP_NAND,
    SIM_CHIP_INPUT,
} SimChipType;

struct SimChip {
    SimChipType type;
    SimPinArray inputs;
    SimPinArray outputs;
};

typedef struct {
    Set *chips;
} Simulation;

extern Simulation simulation;

void sim_init(void);
void sim_add_chip(SimChip *chip);
// NOTE: it doesn't free chip, and neither removes the connections between pins
// this is like that because this function is used for the ui_delete_chip (for now)
// and that function removes the wires and the connections that are made for them
void sim_remove_chip(SimChip *chip);

// CHIP
SimChip *sim_chip_new(SimChipType type);
void sim_chip_free(SimChip *chip);
SimPin *sim_chip_get_input_pin(SimChip *chip, size_t index);
SimPin *sim_chip_get_output_pin(SimChip *chip, size_t index);
// toggles pin state between PIN_OFF and PIN_ON
// NOTE: returns false when the pin was not found
bool sim_chip_toggle_output_pin(SimChip *chip, size_t index);

// PIN
void sim_pin_add_connection(SimPin *src, SimPin *target);
// removes "target" connection from "src"
void sim_pin_remove_connection(SimPin *src, SimPin *target);

#endif // SIMULATION_H
