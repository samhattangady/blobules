#ifndef CHAPLIBOY_UI_DEFINED
#define CHAPLIBOY_UI_DEFINED

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
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

// typedef enum {
//     CB_BUTTON,
// } widget_type;
// 
// typedef struct {
//     char* title;
//     widget_type type;
//     uint position[2];
//     uint size[2];
// } cb_widget;
// 
// typedef struct {
//     uint allotted;
//     uint used;
//     cb_widget* widgets;
// } cb_widget_array;
// 
// typedef struct {
//     char* title;
//     uint position[2];
//     uint size[2];
//     int current_x;
//     int current_y;
//     cb_widget_array widgets;
// } cb_window;

typedef struct cb_ui_state {
    gl_values values;
    ft_char glyphs[128];
} cb_ui_state;

typedef struct {
    // TODO (07 Apr 2020 sam): How can this use less memory? it should just be one bit?
    uint down;
    float down_x;
    float down_y;
} mouse_state_struct;

int init_ui(cb_ui_state* state);
int cb_ui_render_text(cb_ui_state* state, char* text, float x, float y);
int init_gl_values(cb_ui_state* state);
int init_character_glyphs(cb_ui_state* state);
// cb_window init_cb_window(char* title, uint position[2], uint size[2]);

#endif
