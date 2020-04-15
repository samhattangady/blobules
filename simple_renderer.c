#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "simple_renderer.h"
#define WINDOW_WIDTH 1536
#define WINDOW_HEIGHT 864

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Errors: %s\n", description);
}

int test_shader_compilation(uint shader) {
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        fprintf(stderr, "fragment shader failed to compile... %s\n", buffer);
        return -1;
    }
    return 0;
}

int init_renderer(renderer* r, char* window_name) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* window;
    int window_height = WINDOW_HEIGHT;
    int window_width = WINDOW_WIDTH;
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, window_name, NULL, NULL);
    if (!window) {
        fprintf(stderr, "GLFW could not create a window...\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "GLEW could not initiate.\n");
        return -1;
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    uint vao;
    uint vbo;
    uint vertex_shader;
    uint fragment_shader;
    uint shader_program;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    shader_program = glCreateProgram();

    renderer r_ = { window, { WINDOW_WIDTH, WINDOW_HEIGHT },
                  { vao, vbo, vertex_shader, fragment_shader, shader_program},
                    NULL };
    *r = r_;
    return 0;
}

int set_world(renderer* r, world* w) {
    if (r->vertex_buffer)
        free(r->vertex_buffer);
    r->buffer_size = sizeof(float) * w->size * 18;
    r->vertex_buffer = (float*) malloc(r->buffer_size);

    string vertex_source = read_file("glsl/simple_vertex.glsl");
    string fragment_source = read_file("glsl/simple_fragment.glsl");
    glShaderSource(r->shader.vertex_shader, 1, &vertex_source.text, NULL);
    glCompileShader(r->shader.vertex_shader);
    test_shader_compilation(r->shader.vertex_shader);
    glShaderSource(r->shader.fragment_shader, 1, &fragment_source.text, NULL);
    glCompileShader(r->shader.fragment_shader);
    test_shader_compilation(r->shader.fragment_shader);
    glAttachShader(r->shader.shader_program, r->shader.vertex_shader);
    glAttachShader(r->shader.shader_program, r->shader.fragment_shader);
    glBindFragDataLocation(r->shader.shader_program, 0, "fragColor");
    glLinkProgram(r->shader.shader_program);
    glUseProgram(r->shader.shader_program);
    dispose_string(&vertex_source);
    dispose_string(&fragment_source);
    return 0;
}

float get_entity_material(entity_type type) {
    if (type == PLAYER)
        return 1.0;
    if (type == CUBE)
        return 2.0;
    if (type == WALL)
        return 3.0;
    if (type == GROUND)
        return 4.0;
    if (type == NONE)
        return 5.0;
    if (type == SLIPPERY_GROUND)
        return 6.0;
    if (type == HOT_TARGET)
        return 7.0;
    if (type == COLD_TARGET)
        return 8.0;
}

float get_block_size(entity_type type) {
    if (type == PLAYER)
        return 0.4;
    if (type == CUBE)
        return 0.8;
    return 1.0;
}

int update_vertex_buffer(renderer* r, world* w) {
    // the size of a block is 1/20 screen width
    float blockx = 1.0 / 20.0;
    float blocky = blockx * r->size[0] / r->size[1];
    for (int z=0; z<w->z_size; z++) {
        for (int y=0; y<w->y_size; y++) {
            for (int x=0; x<w->x_size; x++) {
                int i = get_position_index(w, x, y, z); 
                float material = get_entity_material(w->entities[i]);
                float size = get_block_size(w->entities[i]);
                r->vertex_buffer[(18*i)+0] = (blockx*x) - (size*blockx/2.0);
                r->vertex_buffer[(18*i)+1] = (blocky*y) - (size*blocky/2.0);
                r->vertex_buffer[(18*i)+2] = material;
                r->vertex_buffer[(18*i)+3] = (blockx*x) + (size*blockx/2.0);
                r->vertex_buffer[(18*i)+4] = (blocky*y) - (size*blocky/2.0);
                r->vertex_buffer[(18*i)+5] = material;
                r->vertex_buffer[(18*i)+6] = (blockx*x) - (size*blockx/2.0);
                r->vertex_buffer[(18*i)+7] = (blocky*y) + (size*blocky/2.0);
                r->vertex_buffer[(18*i)+8] = material;
                r->vertex_buffer[(18*i)+9] = (blockx*x) - (size*blockx/2.0);
                r->vertex_buffer[(18*i)+10] = (blocky*y) + (size*blocky/2.0);
                r->vertex_buffer[(18*i)+11] = material;
                r->vertex_buffer[(18*i)+12] = (blockx*x) + (size*blockx/2.0);
                r->vertex_buffer[(18*i)+13] = (blocky*y) - (size*blocky/2.0);
                r->vertex_buffer[(18*i)+14] = material;
                r->vertex_buffer[(18*i)+15] = (blockx*x) + (size*blockx/2.0);
                r->vertex_buffer[(18*i)+16] = (blocky*y) + (size*blocky/2.0);
                r->vertex_buffer[(18*i)+17] = material;
            }
        }
    }
    return 0;
}

int render_scene(renderer* r, world* w) {
    update_vertex_buffer(r, w);
    glUseProgram(r->shader.shader_program);
    glLinkProgram(r->shader.shader_program);
    glBindVertexArray(r->shader.vao);
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, r->shader.vbo);
    glBufferData(GL_ARRAY_BUFFER, r->buffer_size, r->vertex_buffer, GL_DYNAMIC_DRAW);
    int position_attribute = glGetAttribLocation(r->shader.shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FLOAT, 0, 0);
    int uni_ybyx = glGetUniformLocation(r->shader.shader_program, "ybyx");
    glUniform1f(uni_ybyx, WINDOW_HEIGHT*1.0/WINDOW_WIDTH);
    glViewport(0, 0, r->size[0], r->size[1]);
    glDrawArrays(GL_TRIANGLES, 0, w->size*6);
    glfwSwapBuffers(r->window);
    return 0;
}
