#ifndef CHAPLIBOY_UI_DEFINED
#define CHAPLIBOY_UI_DEFINED

#define GLEW_STATIC
#include <stdbool.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_truetype.h"
#include "cb_lib/cb_types.h"
#include "common.h"

typedef struct {
    uint occupied;
    float* vertex_buffer;
} vertex_buffer_data;

typedef struct gl_values {
    uint char_vao;
    uint char_vbo;
    uint rect_vao;
    uint rect_vbo;
    uint line_vao;
    uint line_vbo;
    uint vertex_shader;
    uint fragment_shader;
    uint shader_program;
    vertex_buffer_data rect_buffer;
    vertex_buffer_data char_buffer;
    vertex_buffer_data line_buffer;
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

typedef struct cb_ui_state {
    gl_values values;
    uint font_texture;
    stbtt_bakedchar glyphs[128];
    mouse_data mouse;
} cb_ui_state;

int init_ui(cb_ui_state* state);
int cb_ui_render_text(cb_ui_state* state, char* text, float x, float y);
int cb_ui_render_rectangle(cb_ui_state* state, float xpos, float ypos, float w, float h, float opactity);
int cb_ui_render_line(cb_ui_state* state, float xpos, float ypos, float w, float h, float opactity);
int init_gl_values(cb_ui_state* state);
int init_character_glyphs(cb_ui_state* state);
int init_cb_window(cb_window* w, char* title, uint position[2], uint size[2]);
int add_text(cb_ui_state* state, cb_window* window, char* text, bool newline);
bool add_button(cb_ui_state* state, cb_window* window, char* text, bool newline);
int new_line(cb_ui_state* state, cb_window* window, bool padding);
int vert_spacer(cb_ui_state* state, cb_window* window, bool padding);
int cb_render_window(cb_ui_state* state, cb_window* window);
int render_chars(cb_ui_state* state);
int render_ui(cb_ui_state* state);

#endif
