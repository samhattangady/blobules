#include <stdio.h>
#include <sys/time.h>
#include "cb_renderer.h"
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

    uint vao;
    uint vbo;
    uint vertex_shader;
    uint fragment_shader;
    uint shader_program;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    GLfloat vertices[] = {
        -1.0f,  1.0f,
        -1.0f, -3.0f,
         3.0f,  1.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    string vertex_source = read_file("glsl/vertex.glsl");
    string fragment_source = read_file("glsl/fragment.glsl");
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source.text, NULL);
    glCompileShader(vertex_shader);
    test_shader_compilation(vertex_shader);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source.text, NULL);
    glCompileShader(fragment_shader);
    test_shader_compilation(fragment_shader);
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glBindFragDataLocation(shader_program, 0, "fragColor");
    glLinkProgram(shader_program);
    glUseProgram(shader_program);
    dispose_string(&vertex_source);
    dispose_string(&fragment_source);

    renderer r_ = { window, { WINDOW_WIDTH, WINDOW_HEIGHT },
                  { vao, vbo, vertex_shader, fragment_shader, shader_program} };
    *r = r_;
    return 0;
}

int render_scene(renderer* r, game_state* state) {
    glUseProgram(r->shader.shader_program);
    glLinkProgram(r->shader.shader_program);
    glBindVertexArray(r->shader.vao);
    glClearColor(0.2, 0.2, 0.3, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, r->shader.vbo);
    int position_attribute = glGetAttribLocation(r->shader.shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FLOAT, 0, 0);
    int uni_ybyx = glGetUniformLocation(r->shader.shader_program, "ybyx");
    int uni_position = glGetUniformLocation(r->shader.shader_program, "player_position");
    int uni_rotation = glGetUniformLocation(r->shader.shader_program, "player_rotation");
    int uni_time = glGetUniformLocation(r->shader.shader_program, "time");
    glUniform1f(uni_ybyx, WINDOW_HEIGHT*1.0/WINDOW_WIDTH);
    glUniform3f(uni_position, state->position[0], state->position[1], state->position[2]);
    glUniform3f(uni_rotation, state->rotation[0], state->rotation[1], state->rotation[2]);
    glUniform1f(uni_time, state->seconds);
    glViewport(0, 0, r->size[0], r->size[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(r->window);
    return 0;
}
