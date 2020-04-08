#include <stdio.h>
#include "cb_renderer.h"
#include "cb_lib/cb_string.h"
#include "game.h"


int main(int argc, char** argv) {
    // setup glfw

    // TODO (31 Mar 2020 sam): Name this window according to the current version;
    renderer r;
    init_renderer(&r, "blobules");
    game_state state = { {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 0.0 };
    uint frame = 0;
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
            glfwSetWindowShouldClose(window, GL_TRUE);
        if (key == GLFW_KEY_W && action == GLFW_RELEASE)
            state.target_position[2] += 0.5;
        if (key == GLFW_KEY_S && action == GLFW_RELEASE)
            state.target_position[2] -= 0.5;
        if (key == GLFW_KEY_A && action == GLFW_RELEASE)
            state.target_position[0] -= 0.5;
        if (key == GLFW_KEY_D && action == GLFW_RELEASE)
            state.target_position[0] += 0.5;
    }

    struct timeval start_time;
    struct timeval current_time;
    float frame_time;
    gettimeofday(&start_time, NULL);
    double mouse_x, mouse_y;
    while (!glfwWindowShouldClose(r.window)) {
        frame += 1;
        gettimeofday(&current_time, NULL);
        frame_time = (current_time.tv_sec - start_time.tv_sec) +
                     ((current_time.tv_usec - start_time.tv_usec) / 1000000.0);
        start_time = current_time;
        state.seconds += frame_time;

        if (state.position[0] > state.target_position[0])
            state.position[0] -= 0.05;
        if (state.position[0] < state.target_position[0])
            state.position[0] += 0.05;
        if (state.position[2] > state.target_position[2])
            state.position[2] -= 0.05;
        if (state.position[2] < state.target_position[2])
            state.position[2] += 0.05;
        glfwPollEvents();
        glfwSetKeyCallback(r.window, key_callback);
        // if (glfwGetKey(r.window, GLFW_KEY_A) == GLFW_RELEASE)
        //     state.position[0] -= 1.0;
        // if (glfwGetKey(r.window, GLFW_KEY_D) == GLFW_RELEASE)
        //     state.position[0] += 1.0;
        // if (glfwGetKey(r.window, GLFW_KEY_W) == GLFW_RELEASE)
        //     state.rotation[1] -= 0.01;
        // if (glfwGetKey(r.window, GLFW_KEY_S) == GLFW_RELEASE)
        //     state.rotation[1] += 0.01;
        // if (glfwGetKey(r.window, GLFW_KEY_E) == GLFW_RELEASE)
        //     printf("E Released\n");
        // if (glfwGetKey(r.window, GLFW_KEY_E) == GLFW_PRESS)
        //     printf("E Pressed\n");
        // if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        //     player_rotation[1] += 0.1;
        // if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        //     player_rotation[1] -= 0.1;
        // glfwGetCursorPos(window, &mouse_x, &mouse_y);
        // float angle = (mouse_x - window_width/2)*1.0 / window_width*1.0;
        // // if (window_width/2-mouse_x > 0.001)
        // //     printf("frame %i: %f\n", frame, angle);
        // player_rotation[1] += angle;
        render_scene(&r, &state);
    }

    // TOD (05 Apr 2020 sam): Run all the closing and exit things...
    return 0;
}
