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
    uint vao;
    uint vbo;
    uint vertex_shader;
    uint fragment_shader;
    uint shader_program;
    uint texture;
} shader_data;

typedef struct {
    GLFWwindow* window;
    int size[2];
    shader_data shader;
    uint buffer_size;
    uint buffer_occupied;
    float* vertex_buffer;
} renderer;

int init_renderer(renderer* r, char* window_name);
int render_scene(renderer* r, world* w);
int load_shaders(renderer* r);

#endif
