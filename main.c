#include <stdio.h>
#include <stdbool.h>
#include "ui.h"
#include "simple_renderer.h"
#include "cb_lib/cb_string.h"
#include "game.h"
#include "game_settings.h"


int main(int argc, char** argv) {
    // TODO (31 Mar 2020 sam): Name this window according to the current version;
    renderer r;
    init_renderer(&r, "blobules");
    world w;
    init_world(&w, 1024);
    cb_ui_state ui_state;
    init_ui(&ui_state);

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
        process_inputs(r.window, &w, seconds);
        render_scene(&r, &w);
        cb_ui_render_text(&ui_state, w.levels.levels[w.current_level].text, 0.0, WINDOW_HEIGHT-(1.0*18.0));
        char active_z_level[32];
        sprintf(active_z_level, "z_level: %i", w.editor.z_level);
        cb_ui_render_text(&ui_state, active_z_level, 0.0, WINDOW_HEIGHT-(2.0*18.0));
        char active_block[48];
        sprintf(active_block, "active_block: %s", as_text(w.editor.active_type));
        cb_ui_render_text(&ui_state, active_block, 0.0, WINDOW_HEIGHT-(3.0*18.0));
        glfwSwapBuffers(r.window);
    }

    // TOD (05 Apr 2020 sam): Run all the closing and exit things...
    return 0;
}
