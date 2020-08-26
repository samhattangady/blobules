#ifndef SIMPLE_RENDERER_DEFINED
#define SIMPLE_RENDERER_DEFINED

#define GLEW_STATIC
#include <GL/glew.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include <stdio.h>
#include <stdbool.h>
#include "cb_lib/cb_string.h"
#include "cb_lib/cb_types.h"
#include "game.h"

typedef struct {
    u32 vertex_shader;
    u32 fragment_shader;
    u32 shader_program;
    u32 fill_texture;
    u32 framebuffer;
    u32 rendered_texture;
} shader_data;

typedef enum {
    FLIP_NONE,
    FLIP_VERTICAL,
    FLIP_HORIZONTAL,
    FLIP_DIAGONAL,
} orientation_t;

typedef struct {
    u32 vao;
    u32 vbo;
    u32 buffer_size;
    u32 buffer_occupied;
    float* vertex_buffer;
} buffer_data;

typedef struct {
    int w;
    int h;
    double x1;
    double y1;
    double x2;
    double y2;
} sprite_data;

typedef struct {
    bool fullscreen;
    SDL_Window* window;
    int size[2];
    shader_data ingame_shader;
    shader_data level_background_shader;
    shader_data level_shader;
    buffer_data ingame_buffer;
    buffer_data level_buffer;
    sprite_data* sprites;
    sprite_data* level_sprites;
    entity_type* ground_entities;
} renderer;

int init_renderer(renderer* r, char* window_name);
int render_scene(renderer* r, world* w);
int load_shaders(renderer* r);
int set_fullscreen(renderer* r, bool flag);
int toggle_fullscreen(renderer* r);

#endif
