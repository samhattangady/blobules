#include <stdio.h>
#include <stdbool.h>
#include "simple_renderer.h"
#include "cb_lib/cb_string.h"
#include "game.h"


int main(int argc, char** argv) {
    // TODO (31 Mar 2020 sam): Name this window according to the current version;
    printf("size of bool = %i", sizeof(bool));
    renderer r;
    init_renderer(&r, "blobules");
    world w;
    init_world(&w, 1024);
    set_world(&r, &w);

    uint frame = 0;
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
            glfwSetWindowShouldClose(window, GL_TRUE);
        // if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
        //     state.target_position[2] += 1.0;
        // if (key == GLFW_KEY_S && action == GLFW_PRESS)
        //     state.target_position[2] -= 1.0;
        // if (key == GLFW_KEY_A && action == GLFW_RELEASE)
        //     state.target_position[0] -= 1.0;
        // if (key == GLFW_KEY_D && action == GLFW_RELEASE)
        //     state.target_position[0] += 1.0;
    }

    struct timeval start_time;
    struct timeval current_time;
    float frame_time;
    float seconds = 0.0;
    gettimeofday(&start_time, NULL);
    double mouse_x, mouse_y;
    while (!glfwWindowShouldClose(r.window)) {
        frame += 1;
        gettimeofday(&current_time, NULL);
        frame_time = (current_time.tv_sec - start_time.tv_sec) +
                     ((current_time.tv_usec - start_time.tv_usec) / 1000000.0);
        start_time = current_time;
        seconds += frame_time;
        glfwPollEvents();
        // glfwSetKeyCallback(r.window, key_callback);
        process_inputs(r.window, &w, seconds);
        render_scene(&r, &w);
    }

    // TOD (05 Apr 2020 sam): Run all the closing and exit things...
    return 0;
}
