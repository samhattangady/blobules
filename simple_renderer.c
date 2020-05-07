#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "simple_renderer.h"
#include "game_settings.h"


static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Errors: %s\n", description);
}

int test_shader_compilation(uint shader, char* type) {
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        fprintf(stderr, "%s shader failed to compile... %s\n", type, buffer);
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

    int width, height, nrChannels;
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    uint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("static/ground2.png",&width,&height,&nrChannels,0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    glGenerateMipmap(GL_TEXTURE_2D);
    renderer r_ = { window, { WINDOW_WIDTH, WINDOW_HEIGHT },
                  { vao, vbo, vertex_shader, fragment_shader, shader_program, texture},
                    NULL };
    *r = r_;
    return 0;
}

int get_buffer_size(world* w) {
    return sizeof(float) * w->size * 36;
}

int set_world(renderer* r, world* w) {
    if (r->vertex_buffer)
        free(r->vertex_buffer);
    r->buffer_size = get_buffer_size(w);
    printf("mallocing... set_world\n");
    r->vertex_buffer = (float*) malloc(r->buffer_size);

    string vertex_source = read_file("glsl/simple_vertex.glsl");
    string fragment_source = read_file("glsl/simple_fragment.glsl");
    glShaderSource(r->shader.vertex_shader, 1, &vertex_source.text, NULL);
    glCompileShader(r->shader.vertex_shader);
    test_shader_compilation(r->shader.vertex_shader, "vertex");
    glShaderSource(r->shader.fragment_shader, 1, &fragment_source.text, NULL);
    glCompileShader(r->shader.fragment_shader);
    test_shader_compilation(r->shader.fragment_shader, "frag");
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
    if (type == FURNITURE)
        return 9.0;
    if (type == DESTROYED_TARGET)
        return 10.0;
}

float get_block_size(entity_type type) {
    // if (type == PLAYER)
    //     return 0.4;
    if (type == CUBE)
        return 0.8;
    if (type == FURNITURE)
        return 0.6;
    return 1.0;
}

int update_vertex_buffer(renderer* r, world* w) {
    // TODO (16 Apr 2020 sam): This check is currently being done every
    // frame. Figure out best way to do this...
    if (r->buffer_size != get_buffer_size(w))
        set_world(r, w);
    float blockx = BLOCK_SIZE*1.0 / WINDOW_WIDTH*1.0;
    float blocky = blockx * r->size[0] / r->size[1];
    for (int z=0; z<w->z_size; z++) {
        for (int y=0; y<w->y_size; y++) {
            for (int x=0; x<w->x_size; x++) {
                int i = get_position_index(w, x, y, z);
                float material = get_entity_material(get_entity_at(w, i));
                float size = get_block_size(get_entity_at(w, i));
                r->vertex_buffer[(36*i)+0] = X_PADDING + (blockx*x) - (size*blockx/2.0);
                r->vertex_buffer[(36*i)+1] = Y_PADDING + (blocky*y) - (size*blocky/2.0);
                r->vertex_buffer[(36*i)+2] = material;
                r->vertex_buffer[(36*i)+3] = 0.0;
                r->vertex_buffer[(36*i)+4] = 0.0;
                r->vertex_buffer[(36*i)+5] = 1.0*w->animation_frames[i];
                r->vertex_buffer[(36*i)+6] = X_PADDING + (blockx*x) + (size*blockx/2.0);
                r->vertex_buffer[(36*i)+7] = Y_PADDING + (blocky*y) - (size*blocky/2.0);
                r->vertex_buffer[(36*i)+8] = material;
                r->vertex_buffer[(36*i)+9] = 1.0;
                r->vertex_buffer[(36*i)+10] = 0.0;
                r->vertex_buffer[(36*i)+11] = 1.0*w->animation_frames[i];
                r->vertex_buffer[(36*i)+12] = X_PADDING + (blockx*x) - (size*blockx/2.0);
                r->vertex_buffer[(36*i)+13] = Y_PADDING + (blocky*y) + (size*blocky/2.0);
                r->vertex_buffer[(36*i)+14] = material;
                r->vertex_buffer[(36*i)+15] = 0.0;
                r->vertex_buffer[(36*i)+16] = 1.0;
                r->vertex_buffer[(36*i)+17] = 1.0*w->animation_frames[i];
                r->vertex_buffer[(36*i)+18] = X_PADDING + (blockx*x) - (size*blockx/2.0);
                r->vertex_buffer[(36*i)+19] = Y_PADDING + (blocky*y) + (size*blocky/2.0);
                r->vertex_buffer[(36*i)+20] = material;
                r->vertex_buffer[(36*i)+21] = 0.0;
                r->vertex_buffer[(36*i)+22] = 1.0;
                r->vertex_buffer[(36*i)+23] = 1.0*w->animation_frames[i];
                r->vertex_buffer[(36*i)+24] = X_PADDING + (blockx*x) + (size*blockx/2.0);
                r->vertex_buffer[(36*i)+25] = Y_PADDING + (blocky*y) - (size*blocky/2.0);
                r->vertex_buffer[(36*i)+26] = material;
                r->vertex_buffer[(36*i)+27] = 1.0;
                r->vertex_buffer[(36*i)+28] = 0.0;
                r->vertex_buffer[(36*i)+29] = 1.0*w->animation_frames[i];
                r->vertex_buffer[(36*i)+30] = X_PADDING + (blockx*x) + (size*blockx/2.0);
                r->vertex_buffer[(36*i)+31] = Y_PADDING + (blocky*y) + (size*blocky/2.0);
                r->vertex_buffer[(36*i)+32] = material;
                r->vertex_buffer[(36*i)+33] = 1.0;
                r->vertex_buffer[(36*i)+34] = 1.0;
                r->vertex_buffer[(36*i)+35] = 1.0*w->animation_frames[i];
            }
        }
    }
    return 0;
}

int render_scene(renderer* r, world* w) {
    update_vertex_buffer(r, w);
    glUseProgram(r->shader.shader_program);
    glLinkProgram(r->shader.shader_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->shader.texture);
    glBindVertexArray(r->shader.vao);
    glClearColor(0.1, 0.1, 0.101, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, r->shader.vbo);
    glBufferData(GL_ARRAY_BUFFER, r->buffer_size, r->vertex_buffer, GL_DYNAMIC_DRAW);
    int position_attribute = glGetAttribLocation(r->shader.shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
    int tex_attribute = glGetAttribLocation(r->shader.shader_program, "tex");
    glEnableVertexAttribArray(tex_attribute);
    glVertexAttribPointer(tex_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 3*sizeof(float));
    int uni_ybyx = glGetUniformLocation(r->shader.shader_program, "ybyx");
    int uni_time = glGetUniformLocation(r->shader.shader_program, "time");
    glUniform1f(uni_ybyx, WINDOW_HEIGHT*1.0/WINDOW_WIDTH);
    glUniform1f(uni_time, w->seconds);
    glViewport(0, 0, r->size[0], r->size[1]);
    glDrawArrays(GL_TRIANGLES, 0, w->size*12);
    return 0;
}
