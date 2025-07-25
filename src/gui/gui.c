#include "gui.h"
#include "ui.h"

#define UI_MENU_BG CLITERAL(Color){ 30, 30, 46, 255 }

GUI gui = {
    .isMenuOpen = true,
};

void gui_init() {
    gui.chips = set_new();
    gui.wires = set_new();
    gui.cursorType = MOUSE_CURSOR_DEFAULT;
}

static void draw_menu() {
    static int saveChipId = 0;
    static int addChipId = 0;
    ui_gen_and_set_id(&saveChipId);
    ui_gen_and_set_id(&addChipId);

    int buttonHeight = 30;
    int menuWidth = 300;

    UIWindow(0, 0, GetScreenWidth(), GetScreenHeight()) {
        // UIContainer({
            // .width = LaySizeFixed(menuWidth),
            // .height = LaySizeFitContent(),
            // .bgColor = GUI_MENU_BG,
            // .padding = LayPadding(10),
            // .rowGap = 10,
        // }) {
        UIContainer({
            .width = UISizeFixed(menuWidth),
            .height = UISizeFitContent(),
            .bgColor = UI_MENU_BG,
            .padding = {10, 10, 10, 10},
        }) {
            // LayText("Menu", {
            //     .color = GUI_TEXT_COLOR,
            //     .fontSize = 26,
            // });

            if(UIButton("Save Chip", saveChipId, UISizePercent(1), UISizeFixed(buttonHeight))) {
                TraceLog(LOG_INFO, "Save Chip clicked!");
            }
            //
            if(UIButton("Add Chip", addChipId, UISizePercent(1), UISizeFixed(buttonHeight))) {
                TraceLog(LOG_INFO, "Add Chip clicked!");
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
        if(UIButton("Menu", buttonId, size, size)) {
            gui.isMenuOpen = true;
        }
    };
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
