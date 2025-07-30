#include "gui.h"
#include "ui.h"

#define UI_MENU_BG CLITERAL(Color){ 30, 30, 46, 255 }
#define UI_TEXT_COLOR_WHITE CLITERAL(Color){ 205, 214, 244, 255 }

GUI gui = {
    .isMenuOpen = true,
};

void gui_init() {
    gui.chips = set_new();
    gui.wires = set_new();
    gui.cursorType = MOUSE_CURSOR_DEFAULT;
}

static void toggle_menu() {
    gui.isMenuOpen = !gui.isMenuOpen;
}

static void draw_menu() {
    static int saveChipId = 0;
    static int addChipId = 0;
    ui_gen_and_set_id(&saveChipId);
    ui_gen_and_set_id(&addChipId);

    int buttonHeight = 30;
    int menuWidth = 300;

    UIWindow(0, 0, GetScreenWidth(), GetScreenHeight()) {
        UIContainer({
           .width = UISizePercent(1),
           .height = UISizePercent(1),
           .align = {
               .x = UI_ALIGN_CENTER,
               .y = UI_ALIGN_CENTER,
           },
           .bgColor = RED,
        }) {
            UIContainer({
                .width = UISizeFixed(menuWidth),
                .height = UISizeFitContent(),
                .bgColor = UI_MENU_BG,
                .padding = {10, 10, 10, 10},
                .gap = 10,
            }) {
                UIText("Menu", UI_TEXT_COLOR_WHITE, 26);

                UIButton({
                    .text = "Save Chip",
                    .id = saveChipId,
                    .width = UISizePercent(1),
                    .height = UISizeFixed(buttonHeight),
                    .onClick = &toggle_menu,
                });

                UIButton({
                    .text = "Add Chip",
                    .id = addChipId,
                    .width = UISizePercent(1),
                    .height = UISizeFixed(buttonHeight),
                    .onClick = &toggle_menu,
                });
            }
        }
    }
}

static void draw_menu_button() {
    static int buttonId = 0;
    ui_gen_and_set_id(&buttonId);

    int buttonWidth = 70;
    int buttonHeight = 30;
    int padding = 10;

    int windowX = padding;
    int windowY = GetScreenHeight() - buttonHeight - padding;

    UIWindow(windowX, windowY, buttonWidth, buttonHeight){
        UISize size = UISizePercent(1);
        UIButton({
            .text = "Menu",
            .id = buttonId,
            .width = size,
            .height = size,
            .onClick = &toggle_menu,
        });
    }
}

void gui_update() {
    gui.cursorType = MOUSE_CURSOR_DEFAULT;

    gui_sim_update();

    if(gui.isMenuOpen) {
        draw_menu();
    } else {
        draw_menu_button();
    }

    SetMouseCursor(gui.cursorType);
}

void gui_set_mouse_cursor(MouseCursor cursor) {
    if(gui.cursorType == MOUSE_CURSOR_DEFAULT)
        gui.cursorType = cursor;
}
