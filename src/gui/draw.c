#include "draw.h"

#define GUI_NAND_BG_COLOR CLITERAL(Color){ 191, 13, 78, 255 }
#define GUI_NAND_FONT_SIZE 26

#define GUI_PIN_BG_COLOR CLITERAL(Color){ 58, 62, 74, 255 }

#define GUI_INPUT_COLOR CLITERAL(Color){ 58, 62, 74, 255 }
#define GUI_INPUT_ACTIVE_COLOR CLITERAL(Color){ 15, 182, 214, 255 }

#define GUI_WIRE_COLOR CLITERAL(Color){ 7, 86, 95, 255 }
#define GUI_HIGH_WIRE_COLOR CLITERAL(Color){ 15, 182, 214, 255 }

#define GUI_OUTPUT_COLOR CLITERAL(Color){ 58, 62, 74, 255 }
#define GUI_OUTPUT_DEACTIVE_COLOR CLITERAL(Color){ 7, 86, 95, 255 }
#define GUI_OUTPUT_ACTIVE_COLOR CLITERAL(Color){ 15, 182, 214, 255 }

static void draw_nand(GUIChip *nand) {
    DrawRectangle(nand->pos.x, nand->pos.y, GUI_NAND_WIDTH, GUI_NAND_HEIGHT, GUI_NAND_BG_COLOR);

    const char *text = "NAND";
    int font_size = GUI_NAND_FONT_SIZE;
    int text_width = MeasureText(text, font_size);

    DrawText(
        text,
        nand->pos.x + GUI_NAND_WIDTH / 2 - text_width / 2,
        nand->pos.y + GUI_NAND_HEIGHT / 2 - font_size / 2,
        font_size,
        WHITE
    );
}

static void draw_pin(GUIPin pin) {
    Vector2 pos = gui_pin_get_pos(&pin);
    DrawCircleV(pos, GUI_PIN_RADIUS, GUI_PIN_BG_COLOR);
}

static void draw_pin_array(GUIPinArray pinArr) {
    for(size_t i = 0; i < pinArr.count; i++) {
        draw_pin(pinArr.items[i]);
    }
}

static void draw_input(GUIChip *input) {
    SimPin *pin = sim_chip_get_output_pin(input->simChip, 0);
    if(pin == NULL) {
        panic("Pin is NULL");
    }
    bool on = pin->state == PIN_HIGH;

    Color color = on ? GUI_INPUT_ACTIVE_COLOR : GUI_INPUT_COLOR;

    // this variable will be modified to update the position where next elements will be drawn
    float posX = input->pos.x;

    // draggable rectangle before the switch
    Rectangle draggableRec = {
        .x = posX,
        .y = input->pos.y,
        .width = GUI_INPUT_DRAGGABLE_WIDTH,
        .height = GUI_INPUT_HEIGHT,
    };
    DrawRectangleRounded(draggableRec, 1, 10, GUI_INPUT_COLOR);
    posX += GUI_INPUT_DRAGGABLE_WIDTH + GUI_INPUT_DRAGGABLE_MARGIN;

    // Rectangle of the switch
    Rectangle rec = {
        .x = posX,
        .y = input->pos.y,
        .width = GUI_INPUT_WIDTH,
        .height = GUI_INPUT_HEIGHT,
    };
    DrawRectangleLinesEx(rec, 2, color);

    // indicator of the switch state
    if(on) {
        int innerWidth = 20;
        int innerHeight = 20;
        DrawRectangle(
            posX + GUI_INPUT_WIDTH / 2 - innerWidth / 2,
            input->pos.y + GUI_INPUT_HEIGHT / 2 - innerHeight / 2,
            innerWidth,
            innerHeight,
            GUI_INPUT_ACTIVE_COLOR
        );
    }
    posX += GUI_INPUT_WIDTH;

    // line after the switch that connects to the pin
    Vector2 lineStart = {posX, input->pos.y + GUI_INPUT_HEIGHT / 2};
    Vector2 lineEnd = {lineStart.x + GUI_INPUT_LINE_WIDTH, lineStart.y};
    DrawLineEx(lineStart, lineEnd, GUI_INPUT_LINE_THICKNESS, GUI_INPUT_COLOR);
}

static void draw_output(GUIChip *output) {
    SimPin *pin = sim_chip_get_input_pin(output->simChip, 0);
    if(pin == NULL) {
        panic("Pin is NULL");
    }
    bool on = pin->state == PIN_HIGH;

    Rectangle rec = {
        .x = output->pos.x,
        .y = output->pos.y,
        .width = GUI_OUTPUT_WIDTH,
        .height = GUI_OUTPUT_HEIGHT,
    };
    DrawRectangleLinesEx(rec, 5, GUI_OUTPUT_COLOR);

    Color color = on ? GUI_OUTPUT_ACTIVE_COLOR : GUI_OUTPUT_DEACTIVE_COLOR;
    int innerWidth = 20;
    int innerHeight = 20;
    DrawRectangle(
        output->pos.x + GUI_OUTPUT_WIDTH / 2 - innerWidth / 2,
        output->pos.y + GUI_OUTPUT_HEIGHT / 2 - innerHeight / 2,
        innerWidth, innerHeight, color
    );
}

void gui_draw_chip(GUIChip *chip) {
    draw_pin_array(chip->inputs);
    draw_pin_array(chip->outputs);

    switch(chip->type) {
        case GUI_CHIP_NAND:
            draw_nand(chip);
            break;
        case GUI_CHIP_INPUT:
            draw_input(chip);
            break;
        case GUI_CHIP_OUTPUT:
            draw_output(chip);
            break;
    }
}

static bool is_pin_high(GUIPin *pin) {
    return sim_pin_is_high(pin->simPin);
}

void gui_draw_wires(Set *wires) {
    SetItem *wireItem = wires->head;
    while(wireItem != NULL) {
        GUIWire *wire = wireItem->data;

        Vector2 startPos = gui_pin_get_pos(wire->src);
        Vector2 endPos = gui_pin_get_pos(wire->target);

        Color wireColor = is_pin_high(wire->src) ? GUI_HIGH_WIRE_COLOR : GUI_WIRE_COLOR;

        DrawLineEx(startPos, endPos, GUI_WIRE_THICKNESS, wireColor);

        wireItem = wireItem->next;
    }
}

void gui_draw_unfinished_wire(GUIWire *wire) {
    Vector2 mousePos = GetMousePosition();

    Vector2 startPos = wire->src != NULL ? gui_pin_get_pos(wire->src) : mousePos;
    Vector2 endPos = wire->target != NULL ? gui_pin_get_pos(wire->target) : mousePos;

    Color wireColor = GUI_WIRE_COLOR;
    if(wire->src != NULL && is_pin_high(wire->src)) {
        wireColor = GUI_HIGH_WIRE_COLOR;
    }

    DrawLineEx(startPos, endPos, GUI_WIRE_THICKNESS, wireColor);
}
