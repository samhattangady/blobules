#include "SDL.h"
#include <stdio.h>
#include <stdbool.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "ui.h"
#include "renderer.h"
#include "cb_lib/cb_string.h"
#include "game.h"
#include "boombox.h"
#include "game_settings.h"
#define STB_LEAKCHECK_IMPLEMENTATION
#define STB_LEAKCHECK_SHOWALL
#include "stb_leakcheck.h"


int main(int argc, char** argv) {
    // TODO (31 Mar 2020 sam): Name this window according to the current version;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    world w;
    init_world(&w, 1024);
    printf("initted world\n");
    renderer r;
    init_renderer(&r, "blobules");
    // boombox b;
    // init_boombox(&b);
    // w.boom = &b;
    // play_sound(&b, MUSIC, false, 0.0);
    cb_ui_state ui_state;
    init_ui(&ui_state);
    w.editor.ui_state = &ui_state;
    set_renderer(&r);

    SDL_GameController *controller = NULL;
    for (int i=0; i<SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                break;
            } else {
                fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
            }
        }
    }

    u32 frame = 0;
    u32 start_time;
    u32 clock_time;
    float frame_time;
    float frame_cycles;
    float seconds = 0.0;
    start_time = SDL_GetTicks();
    double mouse_x, mouse_y;

    u32 window_pos[2] = {20, 40};
    u32 window_size[2] = {UI_WIDTH, WINDOW_HEIGHT};
    init_cb_window(&w.editor.ui_window, "Level Editor", window_pos, window_size);
    start_time = clock();
    printf("starting game loop\n");
    SDL_Event event;
    // set_callbacks(r.window);
    while (w.active_mode != EXIT) {
        frame += 1;
        handle_input_state(&w, controller);
        while (SDL_PollEvent(&event)) {
            process_input_event(&w, event);
            if (event.type == SDL_QUIT)
                w.active_mode = EXIT;
        }
        clock_time = SDL_GetTicks();
        frame_time = ((double)clock_time-(double)start_time)/1000.0;
        start_time = clock_time;
        seconds += frame_time;
        // update_boombox(&b, seconds);
        simulate_world(&w, seconds);
        ui_state.mouse = w.mouse;
        render_scene(&r, &w);
        if (w.editor.editor_enabled) {
            // TODO (15 Jun 2020 sam): Move this method to some other file. It's too much
            // of a mess here...
            char active_z_level[32];
            add_text(&ui_state, &w.editor.ui_window, w.level_select.levels[w.level_select.current_level].name.text, true);
            sprintf(active_z_level, "z_level: %i", w.editor.z_level);
            add_text(&ui_state, &w.editor.ui_window, active_z_level, true);
            char active_block[48];
            sprintf(active_block, "active_block: %s", as_text(w.editor.active_type));
            add_text(&ui_state, &w.editor.ui_window, active_block, true);
            new_line(&ui_state, &w.editor.ui_window, false);
            add_text(&ui_state, &w.editor.ui_window, "Select a block type...", true);
            if (add_button(&ui_state, &w.editor.ui_window, "CUBE", true)) {
                printf("cube\n");
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
        sprintf(fps_counter, "%.2f fps", 1.0/frame_time);
        cb_ui_render_text(&ui_state, fps_counter, WINDOW_WIDTH-100, 20);
        cb_ui_render_text(&ui_state, cps_counter, WINDOW_WIDTH-100, 40);
        render_ui(&ui_state);
        reset_inputs(&w);
        SDL_GL_SwapWindow(r.window);
    }
    // TODO (05 Apr 2020 sam): Run all the closing and exit things...
    stb_leakcheck_dumpmem();
    return 0;
}
