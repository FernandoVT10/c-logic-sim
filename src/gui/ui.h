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

#define UIButton(text, id, width, height) ui_button(text, id, width, height)

#define UIContainer(...) for(                           \
    LAYCONTAINER_DEFINITION = false, ui_container((UIContainerOpts) __VA_ARGS__); \
    !LAYCONTAINER_DEFINITION;                        \
    LAYCONTAINER_DEFINITION = true, ui_container_close())

#define UISizeFixed(n) (UISize){.type = UI_SIZE_FIXED, .value = (n)}
#define UISizeFitContent() (UISize){.type = UI_SIZE_FIT_CONTENT}
#define UISizePercent(n) (UISize){.type = UI_SIZE_PERCENT, .value = (n)}

typedef enum {
    UI_CMD_RECT,
    UI_CMD_TEXT,
} UICmdType;

typedef struct { Rectangle rect; Color color; } UICmdRect;
typedef struct {
    char *text;
    Vector2 pos;
    Color color;
    int fontSize;
} UICmdText;

typedef struct {
    UICmdType type;
    union {
        UICmdRect rect;
        UICmdText text;
    };
} UICmd;

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

typedef struct {
    UISize width;
    UISize height;
    Color bgColor;
    UIPadding padding;
} UIContainerOpts;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    UIPadding padding;

    // used to know what cmd will draw it, and update it when calling ui_container_close
    UICmd *cmd;
} UIContainer;

typedef struct {
    struct {
        int x;
        int y;
    } layout;

    struct {
        UIContainer *items;
        size_t count;
        size_t capacity;
    } containers;

    Rectangle last_rec;

    struct {
        UICmd *items;
        size_t count;
        size_t capacity;
    } cmds;

    int hover;
    int focus;
} UIState;

void ui_window(int x, int y, int width, int height);
void ui_window_close(void);

bool ui_button(const char *text, int id, UISize width, UISize height);

void ui_container(UIContainerOpts opts);
void ui_container_close(void);

void ui_gen_and_set_id(int *res);

#endif // UI_H
