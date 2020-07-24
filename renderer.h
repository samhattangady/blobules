#ifndef SIMPLE_RENDERER_DEFINED
#define SIMPLE_RENDERER_DEFINED

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
//#include <sys/time.h>
#include "cb_lib/cb_string.h"
#include "cb_lib/cb_types.h"
#include "game.h"

typedef struct {
    u32 vao;
    u32 vbo;
    u32 vertex_shader;
    u32 fragment_shader;
    u32 shader_program;
    u32 texture;
} shader_data;

typedef struct {
    GLFWwindow* window;
    int size[2];
    shader_data shader;
    u32 buffer_size;
    u32 buffer_occupied;
    float* vertex_buffer;
    entity_type* ground_entities;
} renderer;

int init_renderer(renderer* r, char* window_name);
int render_scene(renderer* r, world* w);
int load_shaders(renderer* r);

#endif
