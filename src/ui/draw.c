#include "draw.h"

#define UI_NAND_BG_COLOR CLITERAL(Color){ 191, 13, 78, 255 }
#define UI_NAND_FONT_SIZE 26

#define UI_PIN_BG_COLOR CLITERAL(Color){ 58, 62, 74, 255 }

#define UI_INPUT_COLOR CLITERAL(Color){ 58, 62, 74, 255 }
#define UI_INPUT_ACTIVE_COLOR CLITERAL(Color){ 15, 182, 214, 255 }

#define UI_WIRE_COLOR CLITERAL(Color){ 7, 86, 95, 255 }
#define UI_HIGH_WIRE_COLOR CLITERAL(Color){ 15, 182, 214, 255 }

#define UI_OUTPUT_COLOR CLITERAL(Color){ 58, 62, 74, 255 }
#define UI_OUTPUT_DEACTIVE_COLOR CLITERAL(Color){ 7, 86, 95, 255 }
#define UI_OUTPUT_ACTIVE_COLOR CLITERAL(Color){ 15, 182, 214, 255 }

static void draw_nand(UIChip *nand) {
    DrawRectangle(nand->pos.x, nand->pos.y, UI_NAND_WIDTH, UI_NAND_HEIGHT, UI_NAND_BG_COLOR);

    const char *text = "NAND";
    int font_size = UI_NAND_FONT_SIZE;
    int text_width = MeasureText(text, font_size);

    DrawText(
        text,
        nand->pos.x + UI_NAND_WIDTH / 2 - text_width / 2,
        nand->pos.y + UI_NAND_HEIGHT / 2 - font_size / 2,
        font_size,
        WHITE
    );
}

static void draw_pin(UIPin pin) {
    Vector2 pos = ui_get_pin_pos(&pin);
    DrawCircleV(pos, UI_PIN_RADIUS, UI_PIN_BG_COLOR);
}

static void draw_pin_array(UIPinArray pinArr) {
    for(size_t i = 0; i < pinArr.count; i++) {
        draw_pin(pinArr.items[i]);
    }
}

static void draw_input(UIChip *input) {
    SimPin *pin = sim_chip_get_output_pin(input->simChip, 0);
    if(pin == NULL) {
        panic("Pin is NULL");
    }
    bool on = pin->state == PIN_HIGH;

    Color color = on ? UI_INPUT_ACTIVE_COLOR : UI_INPUT_COLOR;

    // this variable will be modified to update the position where next elements will be drawn
    float posX = input->pos.x;

    // draggable rectangle before the switch
    Rectangle draggableRec = {
        .x = posX,
        .y = input->pos.y,
        .width = UI_INPUT_DRAGGABLE_WIDTH,
        .height = UI_INPUT_HEIGHT,
    };
    DrawRectangleRounded(draggableRec, 1, 10, UI_INPUT_COLOR);
    posX += UI_INPUT_DRAGGABLE_WIDTH + UI_INPUT_DRAGGABLE_MARGIN;

    // Rectangle of the switch
    Rectangle rec = {
        .x = posX,
        .y = input->pos.y,
        .width = UI_INPUT_WIDTH,
        .height = UI_INPUT_HEIGHT,
    };
    DrawRectangleLinesEx(rec, 2, color);

    // indicator of the switch state
    if(on) {
        int innerWidth = 20;
        int innerHeight = 20;
        DrawRectangle(
            posX + UI_INPUT_WIDTH / 2 - innerWidth / 2,
            input->pos.y + UI_INPUT_HEIGHT / 2 - innerHeight / 2,
            innerWidth,
            innerHeight,
            UI_INPUT_ACTIVE_COLOR
        );
    }
    posX += UI_INPUT_WIDTH;

    // line after the switch that connects to the pin
    Vector2 lineStart = {posX, input->pos.y + UI_INPUT_HEIGHT / 2};
    Vector2 lineEnd = {lineStart.x + UI_INPUT_LINE_WIDTH, lineStart.y};
    DrawLineEx(lineStart, lineEnd, UI_INPUT_LINE_THICKNESS, UI_INPUT_COLOR);
}

static void draw_output(UIChip *output) {
    SimPin *pin = sim_chip_get_input_pin(output->simChip, 0);
    if(pin == NULL) {
        panic("Pin is NULL");
    }
    bool on = pin->state == PIN_HIGH;

    // DrawRectangle(output->pos.x, output->pos.y, UI_OUTPUT_WIDTH, UI_OUTPUT_HEIGHT, UI_INPUT_COLOR);
    Rectangle rec = {
        .x = output->pos.x,
        .y = output->pos.y,
        .width = UI_OUTPUT_WIDTH,
        .height = UI_OUTPUT_HEIGHT,
    };
    DrawRectangleLinesEx(rec, 5, UI_OUTPUT_COLOR);

    Color color = on ? UI_OUTPUT_ACTIVE_COLOR : UI_OUTPUT_DEACTIVE_COLOR;
    int innerWidth = 20;
    int innerHeight = 20;
    DrawRectangle(
        output->pos.x + UI_OUTPUT_WIDTH / 2 - innerWidth / 2,
        output->pos.y + UI_OUTPUT_HEIGHT / 2 - innerHeight / 2,
        innerWidth, innerHeight, color
    );
}

void draw_chip(UIChip *chip) {
    draw_pin_array(chip->inputs);
    draw_pin_array(chip->outputs);

    switch(chip->type) {
        case UI_CHIP_NAND:
            draw_nand(chip);
            break;
        case UI_CHIP_INPUT:
            draw_input(chip);
            break;
        case UI_CHIP_OUTPUT:
            draw_output(chip);
            break;
    }
}

void draw_wires(Set *wires) {
    SetItem *wireItem = wires->head;
    while(wireItem != NULL) {
        UIWire *wire = wireItem->data;

        Vector2 startPos = ui_get_pin_pos(wire->src.pin);
        Vector2 endPos = ui_get_pin_pos(wire->target.pin);

        Color wireColor = ui_is_pin_high(wire->src.pin) ? UI_HIGH_WIRE_COLOR : UI_WIRE_COLOR;

        DrawLineEx(startPos, endPos, UI_WIRE_THICKNESS, wireColor);

        wireItem = wireItem->next;
    }
}

void draw_unfinished_wire(UIWire *wire) {
    Vector2 mousePos = GetMousePosition();

    Vector2 startPos = wire->src.pin != NULL ? ui_get_pin_pos(wire->src.pin) : mousePos;
    Vector2 endPos = wire->target.pin != NULL ? ui_get_pin_pos(wire->target.pin) : mousePos;

    Color wireColor = UI_WIRE_COLOR;
    if(wire->src.pin != NULL && ui_is_pin_high(wire->src.pin)) {
        wireColor = UI_HIGH_WIRE_COLOR;
    }

    DrawLineEx(startPos, endPos, UI_WIRE_THICKNESS, wireColor);
}
