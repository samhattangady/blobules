#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "ui.h"
#include "simple_renderer.h"
#include "cb_lib/cb_string.h"
#include "game.h"
#include "game_settings.h"


int main(int argc, char** argv) {
    // TODO (31 Mar 2020 sam): Name this window according to the current version;
    world w;
    init_world(&w, 1024);
    printf("initted world\n");
    renderer r;
    init_renderer(&r, "blobules");
    cb_ui_state ui_state;

    printf("initting ui\n");
    init_ui(&ui_state);
    printf("initted ui\n");
    w.editor.ui_state = &ui_state;

    uint frame = 0;

    //struct timeval start_time;
    //struct timeval current_time;
    clock_t clock_time;
    float frame_time;
    float frame_cycles;
    float seconds = 0.0;
    //gettimeofday(&start_time, NULL);
    double mouse_x, mouse_y;

    uint window_pos[2] = {20, 40};
    uint window_size[2] = {UI_WIDTH, WINDOW_HEIGHT};
    init_cb_window(&w.editor.ui_window, "Level Editor", window_pos, window_size);
    printf("starting game loop\n");
    while (!glfwWindowShouldClose(r.window) && w.active_mode != EXIT) {
        frame += 1;
        // gettimeofday(&current_time, NULL);
        clock_time = clock();
		frame_time = 1.0/60.0;
        // frame_time = (current_time.tv_sec - start_time.tv_sec) + ((current_time.tv_usec - start_time.tv_usec) / 1000000.0);
        // start_time = current_time;
        seconds += frame_time;
        glfwPollEvents();
        process_inputs(r.window, &w, seconds);
        render_scene(&r, &w);
        if (w.editor.editor_enabled) {
            // TODO (15 Jun 2020 sam): Move this method to some other file. It's too much
            // of a mess here...
            char active_z_level[32];
            add_text(&ui_state, &w.editor.ui_window, w.levels.levels[w.current_level].text, true);
            sprintf(active_z_level, "z_level: %i", w.editor.z_level);
            add_text(&ui_state, &w.editor.ui_window, active_z_level, true);
            char active_block[48];
            sprintf(active_block, "active_block: %s", as_text(w.editor.active_type));
            add_text(&ui_state, &w.editor.ui_window, active_block, true);
            new_line(&ui_state, &w.editor.ui_window, false);
            add_text(&ui_state, &w.editor.ui_window, "Select a block type...", true);
            if (add_button(&ui_state, &w.editor.ui_window, "CUBE", true)) {
                w.editor.z_level = 1;
                w.editor.active_type = CUBE;
            }
            if (add_button(&ui_state, &w.editor.ui_window, "FURNITURE", true)) {
                w.editor.z_level = 1;
                w.editor.active_type = FURNITURE;
            }
            if (add_button(&ui_state, &w.editor.ui_window, "REFLECTOR", true)) {
                w.editor.z_level = 1;
                w.editor.active_type = REFLECTOR;
            }
            if (add_button(&ui_state, &w.editor.ui_window, "PLAYER", true)) {
                w.editor.z_level = 1;
                w.editor.active_type = PLAYER;
            }
            if (add_button(&ui_state, &w.editor.ui_window, "WALL", true)) {
                w.editor.z_level = 1;
                w.editor.active_type = WALL;
            }
            if (add_button(&ui_state, &w.editor.ui_window, "GROUND", true)) {
                w.editor.z_level = 0;
                w.editor.active_type = GROUND;
            }
            if (add_button(&ui_state, &w.editor.ui_window, "SLIPPERY_GROUND", true)) {
                w.editor.z_level = 0;
                w.editor.active_type = SLIPPERY_GROUND;
            }
            if (add_button(&ui_state, &w.editor.ui_window, "HOT_TARGET", true)) {
                w.editor.z_level = 0;
                w.editor.active_type = HOT_TARGET;
            }
            new_line(&ui_state, &w.editor.ui_window, false);
            if (add_button(&ui_state, &w.editor.ui_window, "add col left", false)) {
                change_world_xsize(&w, -1, 1);
            }
            vert_spacer(&ui_state, &w.editor.ui_window, true);
            if (add_button(&ui_state, &w.editor.ui_window, "add col right", true)) {
                change_world_xsize(&w, 1, 1);
            }
            new_line(&ui_state, &w.editor.ui_window, false);
            if (add_button(&ui_state, &w.editor.ui_window, "del col left", false)) {
                change_world_xsize(&w, -1, -1);
            }
            vert_spacer(&ui_state, &w.editor.ui_window, true);
            if (add_button(&ui_state, &w.editor.ui_window, "del col right", true)) {
                change_world_xsize(&w, 1, -1);
            }
            new_line(&ui_state, &w.editor.ui_window, false);
            if (add_button(&ui_state, &w.editor.ui_window, "add row top", false)) {
                change_world_ysize(&w, 1, 1);
            }
            vert_spacer(&ui_state, &w.editor.ui_window, true);
            if (add_button(&ui_state, &w.editor.ui_window, "del row top", true)) {
                change_world_ysize(&w, 1, -1);
            }
            new_line(&ui_state, &w.editor.ui_window, false);
            if (add_button(&ui_state, &w.editor.ui_window, "add row bottom", false)) {
                change_world_ysize(&w, -1, 1);
            }
            vert_spacer(&ui_state, &w.editor.ui_window, true);
            if (add_button(&ui_state, &w.editor.ui_window, "del row bottom", true)) {
                change_world_ysize(&w, -1, -1);
            }
            new_line(&ui_state, &w.editor.ui_window, false);
            new_line(&ui_state, &w.editor.ui_window, false);
            new_line(&ui_state, &w.editor.ui_window, false);
            if (add_button(&ui_state, &w.editor.ui_window, "save_level", true)) {
                save_level(&w);
            }
            cb_render_window(&ui_state, &w.editor.ui_window);
        }
        char fps_counter[48];
        char cps_counter[48];
        clock_time = clock() - clock_time;
        sprintf(fps_counter, "%.2f fps", 1.0/frame_time);
        sprintf(cps_counter, "%.2f cps", 1.0/((double)clock_time/CLOCKS_PER_SEC));
        cb_ui_render_text(&ui_state, fps_counter, WINDOW_WIDTH-100, 20);
        cb_ui_render_text(&ui_state, cps_counter, WINDOW_WIDTH-100, 40);
        //printf("%f cps\n", cps_counter);
        render_ui(&ui_state);
        // Sleep(1000);
        glfwSwapBuffers(r.window);
    }
    // TODO (05 Apr 2020 sam): Run all the closing and exit things...
    return 0;
}
