#include <stdio.h>
#include <stdlib.h>
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

    // string vertex_source = read_file("glsl/vertex.glsl");
    // string fragment_source = read_file("glsl/fragment.glsl");
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    // glShaderSource(vertex_shader, 1, &vertex_source.text, NULL);
    // glCompileShader(vertex_shader);
    // test_shader_compilation(vertex_shader);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    // glShaderSource(fragment_shader, 1, &fragment_source.text, NULL);
    // glCompileShader(fragment_shader);
    // test_shader_compilation(fragment_shader);
    shader_program = glCreateProgram();
    // glAttachShader(shader_program, vertex_shader);
    // glAttachShader(shader_program, fragment_shader);
    // glBindFragDataLocation(shader_program, 0, "fragColor");
    // glLinkProgram(shader_program);
    // glUseProgram(shader_program);
    // dispose_string(&vertex_source);
    // dispose_string(&fragment_source);

    // TODO (13 Apr 2020 sam): figure out how much space to allocate. note that
    // this has to be in sync with the uniform in frag.glsl as well as the world
    // data alloted space.
    printf("mallocing... cb_renderer.c\n");
    float* posdata = (float*) malloc(sizeof(float) * 128);
    int* typedata = (int*) malloc(sizeof(int) * 128);

    renderer r_ = { window, { WINDOW_WIDTH, WINDOW_HEIGHT },
                  { vao, vbo, vertex_shader, fragment_shader, shader_program},
                    posdata, typedata };
    *r = r_;
    return 0;
}

int convert_world_to_shader(uint count, vec3i* positions, float* spositions) {
    for (int i=0; i<count; i++) {
        spositions[(3*i) + 0] = positions[i].x;
        spositions[(3*i) + 1] = positions[i].z;
        spositions[(3*i) + 2] = positions[i].y;
    }
    return 0;
}

float get_entity_size(entity_type type) {
    if (type == PLAYER)
        return 0.3;
    if (type == ICECUBE)
        return 0.3;
    if (type == BLOCKER)
        return 0.3;
    if (type == GROUND)
        return 0.3;
    if (type == NONE)
        return 0.0;
}

float get_entity_material(entity_type type) {
    if (type == PLAYER)
        return 1.0;
    if (type == ICECUBE)
        return 2.0;
    if (type == BLOCKER)
        return 3.0;
    if (type == GROUND)
        return 4.0;
    if (type == NONE)
        return 5.0;
}

int get_entity_position_shader_space(string* s, world* w, uint index) {
    clear_string(s);
    append_sprintf(s, "vec3(%f, %f, %f)", (float)w->positions[index].x, (float)w->positions[index].z, (float)w->positions[index].y);
    return 0;
}

string generate_distance_field(world* w) {
    char* base = "\
    pos = moveAndRotate(pos, %s, vec3(0.0));\
    d1 = sdfRoundedBox(pos, vec3(%f), 0.03);\
    if (d1 < d)\
        m = %f;\
    d = min(d, d1);\
    pos = og_pos;";
    string s = empty_string();
    string source = string_from("vec4 distanceField(vec3 pos) {\nfloat d, d1, m;\nvec3 og_pos=pos;\nd=1000.0;\nm=1.0;\n");
    for (int i=0; i<w->occupied; i++) {
        get_entity_position_shader_space(&s, w, i);
        append_sprintf(&source, base, s.text, get_entity_size(w->entities[i]), get_entity_material(w->entities[i]));
    }
    append_chars(&source, "return vec4(d, m, 0.0, m);}\n");
    print_string(&source);
    dispose_string(&s);
    return source;
}

int set_world(renderer* r, world* w) {
    string vertex_source = read_file("glsl/vertex.glsl");
    string fragment_source = read_file("glsl/header.glsl");
    string footer = read_file("glsl/footer.glsl");
    string source = generate_distance_field(w);
    append_string(&fragment_source, &source);
    append_string(&fragment_source, &footer);
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
    return 0;
}

int render_scene(renderer* r, world* w) {
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
    glUniform1f(uni_ybyx, WINDOW_HEIGHT*1.0/WINDOW_WIDTH);
    glViewport(0, 0, r->size[0], r->size[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // glfwSwapBuffers(r->window);
    return 0;
}
