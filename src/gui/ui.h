#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stddef.h>

static bool LAYCONTAINER_DEFINITION;
// GCC marks the above CLAY__ELEMENT_DEFINITION_LATCH as an unused variable for files that include clay.h but don't declare any layout
// This is to suppress that warning
static inline void layout_suppress_warning(void) {(void)LAYCONTAINER_DEFINITION;}

// this is a weird macro that I stole from CLAY: https://github.com/nicbarker/clay
#define UIWindow(x, y, width, height) for(                                   \
    LAYCONTAINER_DEFINITION = false, ui_window((x), (y), (width), (height)); \
    !LAYCONTAINER_DEFINITION;                                                \
    LAYCONTAINER_DEFINITION = true, ui_window_close())

#define UIButton(...) ui_button((UIButtonOpts) __VA_ARGS__)

#define UIContainer(...) for(                           \
    LAYCONTAINER_DEFINITION = false, ui_container((UIContainerOpts) __VA_ARGS__); \
    !LAYCONTAINER_DEFINITION;                        \
    LAYCONTAINER_DEFINITION = true, ui_container_close())

#define UIText(text, color, fontSize) ui_text(text, color, fontSize)

#define UISizeFixed(n) (UISize){.type = UI_SIZE_FIXED, .value = (n)}
#define UISizeFitContent() (UISize){.type = UI_SIZE_FIT_CONTENT}
#define UISizePercent(n) (UISize){.type = UI_SIZE_PERCENT, .value = (n)}

typedef enum {
    UI_SIZE_FIXED,
    UI_SIZE_FIT_CONTENT,
    UI_SIZE_PERCENT,
} UISizeType;

typedef struct {
    UISizeType type;
    float value;
} UISize;

typedef struct {
    // ordered in the way the are ordered in css
    int top;
    int right;
    int bottom;
    int left;
} UIPadding;

typedef enum {
    UI_ALIGN_LEFT,
    UI_ALIGN_CENTER,
    UI_ALIGN_RIGHT,
} UIAlignmentType;

typedef struct {
    UIAlignmentType x;
    UIAlignmentType y;
} UIAlignment;

typedef struct {
    UISize width;
    UISize height;
    Color bgColor;
    UIPadding padding;
    // gap between columns or rows
    float gap;
    UIAlignment align;
} UIContainerOpts;

typedef void(*UIButtonOnClick)(void);

typedef struct {
    const char *text;
    int id;
    UISize width;
    UISize height;
    UIButtonOnClick onClick;
} UIButtonOpts;

typedef struct UINode UINode;

typedef enum {
    UI_BUTTON_NODE,
    UI_CONTAINER_NODE,
    UI_TEXT_NODE,
} UINodeType;

typedef struct {
    char *text;
    int id;
    UISize width;
    UISize height;
    UIButtonOnClick onClick;
} UIButtonNode;

typedef struct {
    UISize width;
    UISize height;
    Color bgColor;
    UIPadding padding;
    float gap;
    UIAlignment align;
} UIContainerNode;

typedef struct { char *text; Color color; int fontSize; } UITextNode;

typedef struct {
    UINode *head, *tail;
    size_t count;
} UINodeList;

struct UINode {
    UINodeType type;
    Rectangle rect;
    union {
        UIButtonNode button;
        UIContainerNode container;
        UITextNode text;
    };
    UINodeList children;
    UINode *next;
};

typedef struct {
    UINode *tree;

    // stack of node containers
    struct {
        UINode **items;
        size_t count;
        size_t capacity;
    } containers;

    int hover;
    int focus;
} UIState;

void ui_window(float x, float y, float width, float height);
void ui_window_close(void);

void ui_button(UIButtonOpts opts);

void ui_container(UIContainerOpts opts);
void ui_container_close(void);

void ui_text(const char *text, Color color, int fontSize);

void ui_gen_and_set_id(int *res);

#endif // UI_H
