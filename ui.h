#ifndef CHAPLIBOY_UI_DEFINED
#define CHAPLIBOY_UI_DEFINED

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include <stdbool.h>
#include FT_FREETYPE_H
#include "cb_lib/cb_types.h"

typedef struct ft_char {
    uint texture_id;
    uint size_x;
    uint size_y;
    uint bearing_x;
    uint bearing_y;
    uint advance;
} ft_char;

typedef struct gl_values {
    uint vao;
    uint vbo;
    uint vertex_shader;
    uint fragment_shader;
    uint shader_program;
} gl_values;

typedef enum {
    CB_TEXT,
    CB_BUTTON,
} widget_type;

typedef struct {
    char* title;
    widget_type type;
    uint position[2];
    uint size[2];
} cb_widget;

typedef struct {
    uint allotted;
    uint used;
    cb_widget* widgets;
} cb_widget_array;

typedef struct {
    char* title;
    uint position[2];
    uint size[2];
    int current_x;
    int current_y;
    cb_widget_array widgets;
} cb_window;

typedef struct {
    bool l_pressed;
    bool l_released;
    bool r_pressed;
    bool r_released;
    float l_down_x;
    float l_down_y;
    float r_down_x;
    float r_down_y;
    float current_x;
    float current_y;
} mouse_state_struct;

typedef struct cb_ui_state {
    gl_values values;
    ft_char glyphs[128];
    mouse_state_struct mouse;
} cb_ui_state;

int init_ui(cb_ui_state* state);
int cb_ui_render_text(cb_ui_state* state, char* text, float x, float y);
int init_gl_values(cb_ui_state* state);
int init_character_glyphs(cb_ui_state* state);
int init_cb_window(cb_window* w, char* title, uint position[2], uint size[2]);
int add_text(cb_ui_state* state, cb_window* window, char* text, bool newline);
bool add_button(cb_ui_state* state, cb_window* window, char* text, bool newline);
int new_line(cb_ui_state* state, cb_window* window, bool padding);
int vert_spacer(cb_ui_state* state, cb_window* window, bool padding);
int cb_render_window(cb_ui_state* state, cb_window* window);

#endif