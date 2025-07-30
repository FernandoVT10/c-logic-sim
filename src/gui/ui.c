#include "ui.h"
#include "gui.h"

#include <string.h>
#include <stdint.h>

#define UI_TEXT_COLOR CLITERAL(Color){ 205, 214, 244, 255 }

#define UI_BUTTON_BG CLITERAL(Color){ 69, 71, 90, 255 }
#define UI_BUTTON_HOVER_BG CLITERAL(Color){ 49, 50, 68, 255 }
#define UI_BUTTON_FONT_COLOR CLITERAL(Color){ 205, 214, 244, 255 }
#define UI_BUTTON_FONT_SIZE 20

#define ArraySize(arr) (sizeof(arr)/(*arr))

static UIState ui;

static UINode *create_node() {
    // TODO: make use of the arena
    return alloc(sizeof(UINode));
}

void ui_window(float x, float y, float width, float height) {
    UINode *node = create_node();
    node->type = UI_CONTAINER_NODE;
    node->rect = (Rectangle) {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    };
    node->container = (UIContainerNode) {
        .width = UISizeFixed(width),
        .height = UISizeFixed(height),
    };

    ui.tree = node;
    da_append(&ui.containers, node);
}

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

// static void handle_container(UINode *container) {
//     UINode *node = container->children.head;
//     UIPadding padding = container->container.padding;
//
//     float containerH = padding.top;
//
//     float containerWidth = container->rect.width - padding.left - padding.right;
//     // TODO: what happens when the container is fitSize and height is -1?
//     float containerHeight = container->rect.height - padding.top - padding.bottom;
//
//     int index = 0;
//
//     while(node != NULL) {
//         if(index > 0) {
//             containerH += container->container.gap;
//         }
//
//         switch(node->type) {
//             case UI_BUTTON_NODE:
//                 int textWidth = MeasureText(node->button.text, UI_BUTTON_FONT_SIZE);
//                 node->rect.width = parse_size(node->button.width, containerWidth, textWidth);
//                 node->rect.height = parse_size(node->button.height, containerHeight, UI_BUTTON_FONT_SIZE);
//                 break;
//             case UI_CONTAINER_NODE:
//                 node->rect.width = parse_size(node->container.width, containerWidth, -1);
//                 node->rect.height = parse_size(node->container.height, containerHeight, -1);
//                 break;
//             case UI_TEXT_NODE:
//                 node->rect.width = MeasureText(node->text.text, node->text.fontSize);
//                 node->rect.height = node->text.fontSize;
//                 break;
//         }
//
//         containerH += node->rect.height;
//         node = node->next;
//         index++;
//     }
//
//
//     if(container->container.height.type == UI_SIZE_FIT_CONTENT) {
//         containerH += padding.bottom;
//         container->rect.height = containerH;
//     }
//
//     Vector2 pos = {
//         .x = container->rect.x + padding.left,
//         .y = container->rect.y + padding.top,
//     };
//
//     index = 0;
//     node = container->children.head;
//     while(node != NULL) {
//         if(index > 0) {
//             pos.y += container->container.gap;
//         }
//
//         if(container->container.align.x == UI_ALIGN_CENTER) {
//             node->rect.x = pos.x + container->rect.width / 2 - node->rect.width / 2;
//         } else {
//             node->rect.x = pos.x;
//         }
//
//         if(container->container.align.y == UI_ALIGN_CENTER) {
//             node->rect.y = pos.y + container->rect.height / 2 - node->rect.height / 2;
//         } else {
//             node->rect.y = pos.y;
//         }
//
//         if(node->type == UI_CONTAINER_NODE) {
//             handle_container(node);
//         }
//
//         pos.y += node->rect.height;
//
//         node = node->next;
//         index++;
//     }
// }

static void draw_button_node(Rectangle rect, UIButtonNode button) {
    update_control(button.id, rect);

    bool hovered = is_hovered(button.id);

    if(hovered) {
        gui_set_mouse_cursor(MOUSE_CURSOR_POINTING_HAND);

        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)
            && is_focused(button.id)
            && button.onClick != NULL) {
            button.onClick();
        }
    }

    Color bgColor = hovered ? UI_BUTTON_HOVER_BG : UI_BUTTON_BG;
    DrawRectangleRec(rect, bgColor);

    int textWidth = MeasureText(button.text, UI_BUTTON_FONT_SIZE);
    int textX = rect.x + rect.width / 2 - textWidth / 2;
    int textY = rect.y + rect.height / 2 - UI_BUTTON_FONT_SIZE / 2;
    DrawText(button.text, textX, textY, UI_BUTTON_FONT_SIZE, UI_BUTTON_FONT_COLOR);
}

static void draw_node_list(UINodeList nodes) {
    UINode *node = nodes.head;
    while(node != NULL) {
        switch(node->type) {
            case UI_BUTTON_NODE:
                draw_button_node(node->rect, node->button);
                break;
            case UI_CONTAINER_NODE:
                DrawRectangleRec(node->rect, node->container.bgColor);
                draw_node_list(node->children);
                break;
            case UI_TEXT_NODE:
                float textX = node->rect.x;
                float textY = node->rect.y;
                Color color = node->text.color;
                int fontSize = node->text.fontSize;
                DrawText(node->text.text, textX, textY, fontSize, color);
                break;
        }
        node = node->next;
    }
}

// returns the parsed width or -1 if the "size" should be treated as fit-content
static float parse_size(UISize size, UISizeType containerSizeType, float containerSize) {
    switch(size.type) {
        case UI_SIZE_FIT_CONTENT:
            return -1;
        case UI_SIZE_FIXED:
            return size.value;
        case UI_SIZE_PERCENT:
            if(containerSizeType == UI_SIZE_FIT_CONTENT) {
                return -1;
            }

            return size.value * containerSize;
    }

    panic("UNREACHABLE");

}

static float parse_width(UISize width, UINode *container) {
    assert(container->type == UI_CONTAINER_NODE && "container is not a container");
    UISizeType containerWidthType = container->container.width.type;
    UIPadding padding = container->container.padding;
    float containerWidth = container->rect.width - padding.left - padding.right;
    return parse_size(width, containerWidthType, containerWidth);
}

static float parse_height(UISize height, UINode *container) {
    assert(container->type == UI_CONTAINER_NODE && "container is not a container");
    UISizeType containerHeightType = container->container.height.type;
    UIPadding padding = container->container.padding;
    float containerHeight = container->rect.height - padding.top - padding.bottom;
    return parse_size(height, containerHeightType, containerHeight);
}

static void calculate_container_size(UINode *container) {
    assert(container->type == UI_CONTAINER_NODE && "container is not a container");
    UIPadding padding = container->container.padding;

    float fitHeight = padding.top + padding.bottom;
    // gets the width of the largest element, is used when width is set to fit-content
    float maxWidth = 0;

    int index = 0;
    UINode *node = container->children.head;
    while(node != NULL) {
        if(index > 0) {
            fitHeight += container->container.gap;
        }

        switch(node->type) {
            case UI_BUTTON_NODE: {
                node->rect.width = parse_width(node->button.width, container);
                if(node->rect.width == -1) {
                    node->rect.width = MeasureText(node->button.text, UI_BUTTON_FONT_SIZE);
                }

                node->rect.height = parse_height(node->button.height, container);
                if(node->rect.height == -1) {
                    node->rect.height = UI_BUTTON_FONT_SIZE;
                }
            } break;
            case UI_CONTAINER_NODE: {
                node->rect.width = parse_width(node->container.width, container);
                node->rect.height = parse_height(node->container.height, container);

                calculate_container_size(node);
            } break;
            case UI_TEXT_NODE:
                node->rect.width = MeasureText(node->text.text, node->text.fontSize);
                node->rect.height = node->text.fontSize;
                break;
        }

        fitHeight += node->rect.height;

        if(node->rect.width > maxWidth) {
            maxWidth = node->rect.width;
        }

        node = node->next;
        index++;
    }

    if(container->container.width.type == UI_SIZE_FIT_CONTENT) {
        container->rect.width = maxWidth;
    }

    if(container->container.height.type == UI_SIZE_FIT_CONTENT) {
        container->rect.height = fitHeight;
    }

    container->container.fitHeight = fitHeight;

    // TODO: for default this only works with the layout direction of top-bottom
    // it will not work with left-right direction
}

static void calculate_container_position(UINode *container) {
    assert(container->type == UI_CONTAINER_NODE && "container is not a container");

    UIPadding padding = container->container.padding;

    Vector2 pos = {
        .x = container->rect.x + padding.left,
        .y = container->rect.y + padding.top,
    };

    if(container->container.align.y == UI_ALIGN_CENTER) {
        pos.y = container->rect.y + container->rect.height / 2 - container->container.fitHeight / 2;
    }

    int index = 0;
    UINode *node = container->children.head;
    while(node != NULL) {
        if(index > 0) {
            pos.y += container->container.gap;
        }

        node->rect.y = pos.y;

        if(container->container.align.x == UI_ALIGN_CENTER) {
            node->rect.x = pos.x + container->rect.width / 2 - node->rect.width / 2;
        } else {
            node->rect.x = pos.x;
        }

        if(node->type == UI_CONTAINER_NODE) {
            calculate_container_position(node);
        }

        pos.y += node->rect.height;

        node = node->next;
        index++;
    }

    // TODO: for default this only works with the layout direction of top-bottom
    // it will not work with left-right direction
}

void ui_window_close(void) {

    calculate_container_size(ui.tree);
    calculate_container_position(ui.tree);

    draw_node_list(ui.tree->children);
}

static UINode *get_last_container() {
    if(ui.containers.count == 0) {
        panic("There's no containers");
    }

    return ui.containers.items[ui.containers.count - 1];
}

static void add_child_to_node(UINode *node, UINode *child) {
    if(node->children.count == 0) {
        node->children.head = child;
    } else {
        node->children.tail->next = child;
    }

    node->children.tail = child;
    node->children.count++;
}

static char *copy_text(const char *text) {
    // TODO: make use of the arena
    return strdup(text);
}

void ui_button(UIButtonOpts opts) {
    UINode *node = create_node();
    node->type = UI_BUTTON_NODE;
    node->button = (UIButtonNode) {
        .text = copy_text(opts.text),
        .id = opts.id,
        .width = opts.width,
        .height = opts.height,
        .onClick = opts.onClick,
    };

    UINode *container = get_last_container();
    add_child_to_node(container, node);
}

void ui_container(UIContainerOpts opts) {
    UINode *node = create_node();
    node->type = UI_CONTAINER_NODE;
    node->container = (UIContainerNode) {
        .width = opts.width,
        .height = opts.height,
        .bgColor = opts.bgColor,
        .padding = opts.padding,
        .gap = opts.gap,
        .align = opts.align,
    };

    UINode *container = get_last_container();
    add_child_to_node(container, node);
    da_append(&ui.containers, node);
}

void ui_container_close(void) {
    if(ui.containers.count == 0) {
        TraceLog(LOG_ERROR, "There's no containers, but ui_container_close was called");
        return;
    }

    ui.containers.count--;
}

void ui_text(const char *text, Color color, int fontSize) {
    UINode *node = create_node();
    node->type = UI_TEXT_NODE;
    node->text = (UITextNode) {
        .text = copy_text(text),
        .color = color,
        .fontSize = fontSize,
    };

    UINode *container = get_last_container();
    add_child_to_node(container, node);
}

void ui_gen_and_set_id(int *res) {
    static int currentId = 0;
    if(*res == 0) *res = ++currentId;
}
