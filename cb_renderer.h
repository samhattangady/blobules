#ifndef CHAPLIBOY_GL_DEFINED
#define CHAPLIBOY_GL_DEFINED

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "cb_lib/cb_string.h"
#include "cb_lib/cb_types.h"
#include "game.h"

typedef struct {
    uint vao;
    uint vbo;
    uint vertex_shader;
    uint fragment_shader;
    uint shader_program;
} shader_data;

typedef struct {
    GLFWwindow* window;
    int size[2];
    shader_data shader;
} renderer;

int init_renderer(renderer* r, char* window_name);
int render_scene(renderer* r, game_state* state);

#endif
