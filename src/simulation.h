#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdbool.h>
#include "utils.h"

typedef struct SimChip SimChip;

// NOTE: should it be a boolean instead of an enum?
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
    SIM_CHIP_OUTPUT,
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

/*
 * Initializes necessary data into the variable "simulation."
 * Should be called before any other function from this file.
 */
void sim_init(void);

void sim_add_chip(SimChip *chip);

/*
 * Removes the chip from the Simulation "chips" set.
 * It doesn't free the chip or remove the connections between pins.
 */
void sim_remove_chip(SimChip *chip);

// ------------------------- //
// SimChip related functions //
// ------------------------- //

SimChip *sim_chip_new(SimChipType type);

/*
 * Frees the chip and its fields. It doesn't remove the connections between pins.
 */
void sim_chip_free(SimChip *chip);

/*
 * @return NULL when there's no a pin with that index
 */
SimPin *sim_chip_get_input_pin(SimChip *chip, size_t index);

/*
 * @return NULL when there's no a pin with that index
 */
SimPin *sim_chip_get_output_pin(SimChip *chip, size_t index);

/*
 * Toggles pin state between PIN_HIGH and PIN_LOW
 *
 * @return false if the pin was not found
 */
bool sim_chip_toggle_output_pin(SimChip *chip, size_t index);

// ------------------------ //
// SimPin related functions //
// ------------------------ //

/*
 * It's important to know that the connection between pins, consists of
 * a Set called "connectedPins."
 *
 * So when an output pin is updated, it iterates and updates all the pins
 * inside the Set.
 *
 * An input pin has this Set, too but it's never used. So it's important to
 * differentiate between an input and output pin.
 */

/*
 * Adds "target" pin to "src" pin. "src" pin should be an output pin.
 *
 * @return false when "src" is an input pin
 * */
bool sim_pin_add_connection(SimPin *src, SimPin *target);


/*
 * Removes "target" pin from "src" pin
 *
 * @return true if pin is found and removed
 * */
bool sim_pin_remove_connection(SimPin *src, SimPin *target);

bool sim_pin_is_high(SimPin *pin);

#endif // SIMULATION_H
