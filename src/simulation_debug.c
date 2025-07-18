#include "simulation_debug.h"

#define ASCII_BOLD_GREEN "\e[1;32m"
#define ASCII_BOLD_RED "\e[1;31m"
#define ASCII_BOLD_BLUE "\e[1;34m"
#define ASCII_CYAN "\e[0;36m"
#define ASCII_YELLOW "\e[0;33m"
#define ASCII_RESET "\e[0m"

static void print_pin_array(SimPinArray pinArr, bool isInput) {
    if(pinArr.count == 0) return;

    const char *type = isInput ? "Input" : "Output";
    printf("  "ASCII_YELLOW"%s Pins"ASCII_RESET"\n", type);
    for(size_t i = 0; i < pinArr.count; i++) {
        SimPin pin = pinArr.items[i];
        const char *state = pin.state == PIN_HIGH
            ? ASCII_BOLD_GREEN"ON"ASCII_RESET
            : ASCII_BOLD_RED"OFF"ASCII_RESET;
        printf("    "ASCII_CYAN"#%lu"ASCII_RESET" is %s\n", i, state);
    }
}

void sim_debug_print(Simulation sim) {
    if(sim.chips->count == 0) return;
    printf("\n");
    SetItem *chipItem = sim.chips->head;
    while(chipItem != NULL) {
        SimChip *chip = chipItem->data;
        char *chipName;
        switch(chip->type) {
            case SIM_CHIP_INPUT:
                chipName = "INPUT";
                break;
            case SIM_CHIP_NAND:
                chipName = "NAND";
                break;
            case SIM_CHIP_OUTPUT:
                chipName = "OUTPUT";
                break;
        }

        printf(ASCII_BOLD_BLUE"%s"ASCII_RESET"\n", chipName);
        print_pin_array(chip->inputs, true);
        print_pin_array(chip->outputs, false);

        chipItem = chipItem->next;
    }
    printf("\n");
}
