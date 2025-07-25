#include "ui.h"
#include "gui.h"

#include <string.h>

#define UI_TEXT_COLOR CLITERAL(Color){ 205, 214, 244, 255 }

#define UI_BUTTON_BG CLITERAL(Color){ 69, 71, 90, 255 }
#define UI_BUTTON_HOVER_BG CLITERAL(Color){ 49, 50, 68, 255 }
#define UI_BUTTON_FONT_COLOR CLITERAL(Color){ 205, 214, 244, 255 }
#define UI_BUTTON_FONT_SIZE 20

#define ArraySize(arr) (sizeof(arr)/(*arr))

static UIState ui;

void ui_window(int x, int y, int width, int height) {
    ui.layout.x = x;
    ui.layout.y = y;

    da_append(&ui.containers, ((UIContainer) {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    }));
}

void ui_window_close(void) {
    if(ui.containers.count == 0) {
        TraceLog(LOG_ERROR, "There's no containers, but ui_window_close was called");
        return;
    }

    ui.containers.count--;

    // draw commands!
    for(size_t i = 0; i < ui.cmds.count; i++) {
        UICmd cmd = ui.cmds.items[i];

        switch(cmd.type) {
            case UI_CMD_RECT:
                DrawRectangleRec(cmd.rect.rect, cmd.rect.color);
                break;
            case UI_CMD_TEXT:
                Vector2 pos = cmd.text.pos;
                DrawText(cmd.text.text, pos.x, pos.y, cmd.text.fontSize, cmd.text.color);
                break;
        }
    }

    // reset stack
    ui.cmds.count = 0;
}

// static Rectangle get_next_rec() {
//     Rectangle res = {
//         .x = ui.container.x,
//         .y = ui.last_rec.height,
//         .width = ui.container.width,
//     };
//
//     if(ui.container.rows != NULL) {
//         res.height = ui.container.rows[0];
//     } else {
//         res.height = ui.container.height;
//     }
//
//     return (ui.last_rec = res);
// }

static bool is_hovered(int id) {
    return ui.hover == id;
}

static bool is_focused(int id) {
    return ui.focus == id;
}

static void update_control(int id, Rectangle rec) {
    bool isMouseOver = CheckCollisionPointRec(GetMousePosition(), rec);

    if(is_hovered(id) && !isMouseOver) {
        ui.hover = 0;
    } else if(isMouseOver) {
        ui.hover = id;
    }

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if(is_hovered(id)) {
            ui.focus = id;
        } else if(is_focused(id)) {
            ui.focus = 0;
        }
    }
}

static int parse_size(UISize size, int containerSize, int fitSize) {
    switch(size.type) {
        case UI_SIZE_FIT_CONTENT:
            return fitSize;
        case UI_SIZE_FIXED:
            return size.value;
        case UI_SIZE_PERCENT:
            // if the container size is -1, it means that it was set to fit the content
            // so we default to fit our own size
            if(containerSize == -1) {
                return fitSize;
            }

            return size.value * containerSize;
    }

    panic("UNREACHABLE");
}

static UIContainer *get_last_container() {
    if(ui.containers.count == 0) {
        panic("There's no containers");
    }

    return &ui.containers.items[ui.containers.count - 1];
}

static Rectangle layout_next(UISize width, UISize height, Vector2 fitSize) {
    UIContainer *container = get_last_container();

    int padWidth = container->padding.left + container->padding.right;

    Rectangle rec = {
        .x = ui.layout.x + container->padding.left,
        .y = ui.layout.y + container->padding.top,
        .width = parse_size(width, container->width, fitSize.x) - padWidth,
        .height = parse_size(height, container->height, fitSize.y),
    };

    return (ui.last_rec = rec);
}

static void update_layout(Rectangle rec) {
    // TODO: it only works with columns not with rows
    ui.layout.y = rec.x + rec.height;
}

// creates, adds, and returns an empty cmd to the stack
static UICmd *create_cmd() {
    da_append(&ui.cmds, ((UICmd){}));
    return &ui.cmds.items[ui.cmds.count - 1];
}

bool ui_button(const char *text, int id, UISize width, UISize height) {
    int textWidth = MeasureText(text, UI_BUTTON_FONT_SIZE);

    Vector2 fitSize = {textWidth, textWidth};
    Rectangle rec = layout_next(width, height, fitSize);

    bool res = false;

    update_control(id, rec);

    bool hovered = is_hovered(id);

    if(hovered) {
        gui_set_mouse_cursor(MOUSE_CURSOR_POINTING_HAND);

        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && is_focused(id)) {
            res = true;
        }
    }

    {
        UICmd *cmd = create_cmd();
        cmd->type = UI_CMD_RECT;
        cmd->rect.rect = rec;
        cmd->rect.color = hovered ? UI_BUTTON_HOVER_BG : UI_BUTTON_BG;
    }

    {
        int textX = rec.x + rec.width / 2 - textWidth / 2;
        int textY = rec.y + rec.height / 2 - UI_BUTTON_FONT_SIZE / 2;

        UICmd *cmd = create_cmd();
        cmd->type = UI_CMD_TEXT;
        cmd->text = (UICmdText) {
            // TODO: bad
            .text = strdup(text),
            .pos = (Vector2){textX, textY},
            .color = UI_BUTTON_FONT_COLOR,
            .fontSize = UI_BUTTON_FONT_SIZE,
        };
    }

    update_layout(rec);

    return res;
}

void ui_container(UIContainerOpts opts) {
    Rectangle containerRec = layout_next(opts.width, opts.height, (Vector2){-1, -1});

    UICmd *cmd = create_cmd();
    cmd->type = UI_CMD_RECT;
    cmd->rect.rect = containerRec;
    cmd->rect.color = opts.bgColor;

    da_append(&ui.containers, ((UIContainer) {
        .x = containerRec.x,
        .y = containerRec.y,
        .width = containerRec.width,
        .height = containerRec.height,
        .padding = opts.padding,
        .cmd = cmd,
    }));

    ui.last_rec = (Rectangle){0};
}

void ui_container_close(void) {
    if(ui.containers.count == 0) {
        TraceLog(LOG_ERROR, "There's no containers, but ui_container_close was called");
        return;
    }

    UIContainer *container = get_last_container();

    if(container->height == -1) {
        int height = ui.last_rec.y + ui.last_rec.height - container->y + container->padding.bottom;
        container->cmd->rect.rect.height = height;
    }

    ui.containers.count--;
    // TODO: The UISizeFitContent only works with the height
    // TODO: handle 2 containers
}

void ui_gen_and_set_id(int *res) {
    static int currentId = 0;
    if(*res == 0) *res = ++currentId;
}
