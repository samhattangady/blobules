/*
 * SDF tool written using GPU.
 */

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "cb_lib/cb_string.h"
#include "cb_lib/cb_types.h"

#define MIN_ALPHA 0.7
#define MAX_PIXEL_DISTANCE 127
#define u8 uint8_t
#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH 800

#define NUM_FRAMES 60
#define OUTPUT_FPS 60
#define INPUT_FPS 12

typedef struct {
    u32 vao;
    u32 vbo;
    u32 vertex_shader;
    u32 fragment_shader;
    u32 shader_program;
} gl_values;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Errors: %s\n", description);
}

int test_shader_compilation(u32 shader, char* type) {
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

void sdf_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void load_shaders(gl_values* glv) {
    /* load shaders */
    string vertex_source = read_file("glsl/wc_vertex.glsl");
    string fragment_source = read_file("glsl/wc_fragment.glsl");
    glShaderSource(glv->vertex_shader, 1, &vertex_source.text, NULL);
    glCompileShader(glv->vertex_shader);
    test_shader_compilation(glv->vertex_shader, "vertex");
    glShaderSource(glv->fragment_shader, 1, &fragment_source.text, NULL);
    glCompileShader(glv->fragment_shader);
    test_shader_compilation(glv->fragment_shader, "frag");
    glAttachShader(glv->shader_program, glv->vertex_shader);
    glAttachShader(glv->shader_program, glv->fragment_shader);
    glBindFragDataLocation(glv->shader_program, 0, "fragColor");
    glLinkProgram(glv->shader_program);
    glUseProgram(glv->shader_program);
    dispose_string(&vertex_source);
    dispose_string(&fragment_source);
}

int save_wc_file(GLFWwindow* window, gl_values* glv, char* fillname, char* sdfname, char* outfname, float seconds) {
    
    /* create and load textures */
    u32 textures[4];
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *body = stbi_load(fillname,&width,&height,&nrChannels,0);
    unsigned char *sdf = stbi_load(sdfname,&width,&height,&nrChannels,0);
    unsigned char *bg = stbi_load("static/main_menu/fillbg.png",&width,&height,&nrChannels,0);
    unsigned char *bgsdf = stbi_load("static/main_menu/sdf_bg.png",&width,&height,&nrChannels,0);
    glGenTextures(4, &textures);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bg);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bgsdf);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    /* render textures */
    float vertices[] = {
          1.0,  1.0, 1.0, 1.0,   // top right
         -1.0, -1.0, 0.0, 0.0,   // bottom left
          1.0, -1.0, 1.0, 0.0,   // bottom right
         -1.0, -1.0, 0.0, 0.0,   // bottom left
          1.0,  1.0, 1.0, 1.0,   // top right
         -1.0,  1.0, 0.0, 1.0    // top left 
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*24, vertices, GL_STATIC_DRAW);
    int position_attribute = glGetAttribLocation(glv->shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glUniform1i(glGetUniformLocation(glv->shader_program, "spritesheet"), 0);
    glUniform1i(glGetUniformLocation(glv->shader_program, "sdfsheet"), 1);
    glUniform1f(glGetUniformLocation(glv->shader_program, "time"), seconds);

    /* render window */
    glfwSetKeyCallback(window, sdf_key_callback);
        glClearColor(0.94, 0.94, 0.92, 1.0);
        glClear(GL_COLOR_BUFFER_BIT); 
        glDrawArrays(GL_TRIANGLES, 0, 24);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, body);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textures[3]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, sdf);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glUniform1i(glGetUniformLocation(glv->shader_program, "spritesheet"), 2);
        glUniform1i(glGetUniformLocation(glv->shader_program, "sdfsheet"), 3);

        glDrawArrays(GL_TRIANGLES, 0, 24);
    int i=0;
    // while (!glfwWindowShouldClose(window)) {
    while (i<1) {
        glfwPollEvents();
        glfwSwapBuffers(window);
        i++;
    }

    /* capture and save to file */
    u8* output = (u8*) malloc(sizeof(u8) * width*height*4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, output);
    stbi_flip_vertically_on_write(true);
    stbi_write_png(outfname, width, height, 4, output, 0);
    stbi_image_free(body);
    stbi_image_free(sdf);
    stbi_image_free(bg);
    stbi_image_free(bgsdf);
    return 0;
}

int main(int argc, char** argv) {
    clock_t start_time;
    start_time = clock();
    /* creating gl context */
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* window;
    int window_height = WINDOW_HEIGHT;
    int window_width = WINDOW_WIDTH;
    printf("trying to create window\n");
    glfwWindowHint(GLFW_SAMPLES, 16);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "gpu_sdf", NULL, NULL);
    if (!window) {
        printf("GLFW could not create a window...\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("GLEW could not initiate.\n");
        return -1;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    /* initialise basic gl stuffs */
    u32 vao;
    u32 vbo;
    u32 vertex_shader;
    u32 fragment_shader;
    u32 shader_program;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    shader_program = glCreateProgram();

    gl_values glv;
    glv.vao = vao;
    glv.vbo = vbo;
    glv.vertex_shader = vertex_shader;
    glv.fragment_shader = fragment_shader;
    glv.shader_program = shader_program;

    load_shaders(&glv);
    
    for (int i=0; i<NUM_FRAMES*(OUTPUT_FPS/INPUT_FPS); i++) {
    // for (int i=100; i<101; i++) {
        char fillname[50];
        char sdfname[50];
        char outfname[50];
        int index = i / (OUTPUT_FPS/INPUT_FPS);
        float seconds = (i*1.0) * (1.0/OUTPUT_FPS);
        sprintf(fillname, "static/main_menu/anim_%i.png", index);
        sprintf(sdfname, "static/main_menu/sdf_%i.png", index);
        sprintf(outfname, "static/main_menu/wc_%i.png", i);
        printf("calculating %s...\t", outfname);
        printf("seconds %f...\t", seconds);
        save_wc_file(window, &glv, fillname, sdfname, outfname, seconds);
        printf("done %s...\n", outfname);
    }
    start_time = clock() - start_time;
    printf("processing complete in %f seconds\n", (double)start_time/CLOCKS_PER_SEC);
    printf("done.\n");
    return 0;
}
