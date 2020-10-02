#include "SDL.h"
#include "SDL_mixer.h"
#include <stdio.h>
#include <stdbool.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "ui.h"
#include "renderer.h"
#include "cb_lib/cb_string.h"
#include "game.h"
#include "game_settings.h"
#define STB_LEAKCHECK_IMPLEMENTATION
#define STB_LEAKCHECK_SHOWALL
#include "stb_leakcheck.h"


int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
    renderer r;
    init_renderer(&r, "Mouse in Slippers");
    world w;
    init_world(&w, 1024);
    printf("initted world\n");
    cb_ui_state ui_state;
    init_ui(&ui_state);
    w.editor.ui_state = &ui_state;
    set_renderer(&r);
    set_global_renderer(&r);

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
    // start_time = clock();
    printf("starting game loop\n");
    SDL_Event event;
    while (w.active_mode != EXIT) {
        frame += 1;
        while (SDL_PollEvent(&event)) {
            process_input_event(&w, event);
            if (event.type == SDL_QUIT)
                w.active_mode = EXIT;
        }
        handle_input_queue(&w);
        clock_time = SDL_GetTicks();
        frame_time = ((double)clock_time-(double)start_time)/1000.0;
        start_time = clock_time;
        seconds += frame_time;
        simulate_world(&w, seconds);
        ui_state.mouse = w.mouse;
        render_scene(&r, &w);
        // if (w.editor.editor_enabled)
        //     draw_editor(&w);
        char fps_counter[48];
        char cps_counter[48];
        sprintf(fps_counter, "%.2f fps", 1.0/frame_time);
        if (DEBUG_BUILD)
            cb_ui_render_text(&ui_state, fps_counter, WINDOW_WIDTH-100, 20);
        render_ui(&ui_state);
        reset_inputs(&w);
        SDL_GL_SwapWindow(r.window);
    }
    SDL_GL_DeleteContext(r.context);
    SDL_DestroyWindow(r.window);
    SDL_Quit();
    stb_leakcheck_dumpmem();
    printf("quitting game\n");
    return 0;
}
