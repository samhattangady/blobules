/*
 * This is a tool to help with placing the sprites in the correct x/y positions so that
 * the animations look good. Note that due to the coupling of renderer to the rest of the
 * codebase, I just link this to the rest of the codebase. Separating it out would be a
 * little hairy, and this is probably not important enough to do that. However apart from
 * using the init_renderer function, everything else is self contained. Oh and we also use
 * the ui system. Okay it's fairly deeply linked to the game probably, but yeah lets see.
 */
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#define STB_TRUETYPE_IMPLEMENTATION
#define MINIAUDIO_IMPLEMENTATION
#include "ui.h"
#include "renderer.h"
#include "cb_lib/cb_string.h"
#include "game.h"
#include "boombox.h"
#include "game_settings.h"
#define STB_LEAKCHECK_IMPLEMENTATION
#define STB_LEAKCHECK_SHOWALL
#include "stb_leakcheck.h"

#undef ANIMATION_FRAMES_PER_SECOND
uint ANIMATION_FRAMES_PER_SECOND = 5;

typedef enum {
    FP_DEFAULT,
    FP_PLAY,
    FP_PAUSE,
    FP_ALL,
    FP_MODES_COUNT,
} fp_mode;

typedef struct {
    fp_mode mode;
    bool edit_x;
    bool edit_y;
    int default_x;
    int default_y;
    float seconds;
    float animation_seconds_update;
    animations current_animation;
    animation_state as;
} fp_state;

mouse_data mouse;
fp_state state;
cb_window tools_window;

char* get_anim_name(animations a) {
    if (a == STATIC)
        return "STATIC";
    if (a == MOVING_LEFT)
        return "MOVING_LEFT";
    if (a == MOVING_RIGHT)
        return "MOVING_RIGHT";
    if (a == PUSHING_LEFT)
        return "PUSHING_LEFT";
    if (a == SLIPPING)
        return "SLIPPING";
    if (a == STOPPING_HARD_LEFT)
        return "STOPPING_HARD_LEFT";
    return "whoops";
}

animations get_anim_from_name(char* a) {
    printf("checking %s len=%i\n", a, strlen(a));
    if (strcmp(a, "STATIC") == 0)
        return STATIC;
    if (strcmp(a, "MOVING_LEFT") == 0)
        return MOVING_LEFT;
    if (strcmp(a, "MOVING_RIGHT") == 0)
        return MOVING_RIGHT;
    if (strcmp(a, "PUSHING_LEFT") == 0)
        return PUSHING_LEFT;
    if (strcmp(a, "SLIPPING") == 0)
        return SLIPPING;
    if (strcmp(a, "STOPPING_HARD_LEFT") == 0)
        return STOPPING_HARD_LEFT;
    return ANIMATIONS_COUNT;
}

int fp_queue_animation(fp_state* state, animations a) {
    printf("queueing animation\n");
    uint queue_index = state->as.queue_length;
    state->as.queue_length++;
    state->as.queue[queue_index] = a;
    return 0;
}

int fp_next_frame(fp_state* state, bool run_queue) {
    state->animation_seconds_update = state->seconds;
    animation_state as = state->as;
    as.animation_data[as.current_animation_index].index++;
    if (as.animation_data[as.current_animation_index].index ==
            as.animation_data[as.current_animation_index].length) {
        as.animation_data[as.current_animation_index].index = 0;
        if (run_queue) {
            if (as.queue_length > 0) {
                as.current_animation_index = as.queue[0];
                // we want to shift the whole array inwards;
                as.queue_length--;
                for (int j=0; j<as.queue_length; j++)
                    as.queue[j] = as.queue[j+1];
            } else {
                as.current_animation_index = as.default_animation_index; 
            }
        }
    }
    state->as = as;
}

char* fp_get_mode(fp_state* state) {
    if (state->mode == FP_DEFAULT)
        return "Default Mode";
    if (state->mode == FP_PLAY)
        return "Play Mode";
    if (state->mode == FP_PAUSE)
        return "Pause Mode";
    if (state->mode == FP_ALL)
        return "See all Mode";
    return "Doesn't knows Mode";
}

int fp_toggle_edit_x(fp_state* state) {
    state->edit_x = !state->edit_x;
    if (state->edit_x)
        printf("editting x\n");
    return 0;
}

int fp_toggle_edit_y(fp_state* state) {
    state->edit_y = !state->edit_y;
    return 0;
}

int fp_previous_frame(fp_state* state) {
    state->animation_seconds_update = state->seconds;
    animation_state as = state->as;
    if (as.animation_data[as.current_animation_index].index == 0)
        as.animation_data[as.current_animation_index].index = as.animation_data[as.current_animation_index].length-1;
    else
        as.animation_data[as.current_animation_index].index--;
    state->as = as;
    return 0;
}

int fp_cycle_modes(fp_state* state) {
    state->mode++;
    if (state->mode == FP_MODES_COUNT)
        state->mode = 0;
    printf("current mode %i\n", state->mode);
    return 0;
}

void fp_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
        fp_queue_animation(&state, MOVING_LEFT);
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        fp_cycle_modes(&state);
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
        fp_next_frame(&state, false);
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        fp_previous_frame(&state);
    if (key == GLFW_KEY_X && action == GLFW_PRESS)
        fp_toggle_edit_x(&state);
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
        fp_toggle_edit_y(&state);
}

void fp_cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mouse.current_x = xpos;
    mouse.current_y = ypos;
}

void fp_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse.l_pressed = true;
        mouse.l_down_x = mouse.current_x;
        mouse.l_down_y = mouse.current_y;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse.l_pressed = false;
        mouse.l_released = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        mouse.r_pressed = true;
        mouse.r_down_x = mouse.current_x;
        mouse.r_down_y = mouse.current_y;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        mouse.r_pressed = false;
        mouse.r_released = true;
    }
}

int fp_set_callbacks(GLFWwindow* window) {
    glfwSetKeyCallback(window, fp_key_callback);
    glfwSetCursorPosCallback(window, fp_cursor_position_callback);
    glfwSetMouseButtonCallback(window, fp_mouse_button_callback);
    return 0;
}

void fp_add_vertex_to_buffer(renderer* r, float xpos, float ypos, float x_size, float y_size, float depth, float sprite_position) {
    float blockx = BLOCK_WIDTH*1.0 / WINDOW_WIDTH*1.0;
    float blocky = BLOCK_HEIGHT*1.0 / WINDOW_HEIGHT*1.0;
    int j = r->buffer_occupied;
    r->buffer_occupied++;
    r->vertex_buffer[(36*j)+ 0] = X_PADDING + (blockx * xpos);
    r->vertex_buffer[(36*j)+ 1] = Y_PADDING + (blocky * ypos);
    r->vertex_buffer[(36*j)+ 2] = depth;
    r->vertex_buffer[(36*j)+ 3] = 0.0;
    r->vertex_buffer[(36*j)+ 4] = 0.0;
    r->vertex_buffer[(36*j)+ 5] = sprite_position;
    r->vertex_buffer[(36*j)+ 6] = X_PADDING + (blockx * xpos) + (x_size*blockx);
    r->vertex_buffer[(36*j)+ 7] = Y_PADDING + (blocky * ypos);
    r->vertex_buffer[(36*j)+ 8] = depth;
    r->vertex_buffer[(36*j)+ 9] = 1.0;
    r->vertex_buffer[(36*j)+10] = 0.0;
    r->vertex_buffer[(36*j)+11] = sprite_position;
    r->vertex_buffer[(36*j)+12] = X_PADDING + (blockx * xpos);
    r->vertex_buffer[(36*j)+13] = Y_PADDING + (blocky * ypos) + (y_size*blocky);
    r->vertex_buffer[(36*j)+14] = depth;
    r->vertex_buffer[(36*j)+15] = 0.0;
    r->vertex_buffer[(36*j)+16] = 1.0;
    r->vertex_buffer[(36*j)+17] = sprite_position;
    r->vertex_buffer[(36*j)+18] = X_PADDING + (blockx * xpos);
    r->vertex_buffer[(36*j)+19] = Y_PADDING + (blocky * ypos) + (y_size*blocky);
    r->vertex_buffer[(36*j)+20] = depth;
    r->vertex_buffer[(36*j)+21] = 0.0;
    r->vertex_buffer[(36*j)+22] = 1.0;
    r->vertex_buffer[(36*j)+23] = sprite_position;
    r->vertex_buffer[(36*j)+24] = X_PADDING + (blockx * xpos) + (x_size*blockx);
    r->vertex_buffer[(36*j)+25] = Y_PADDING + (blocky * ypos);
    r->vertex_buffer[(36*j)+26] = depth;
    r->vertex_buffer[(36*j)+27] = 1.0;
    r->vertex_buffer[(36*j)+28] = 0.0;
    r->vertex_buffer[(36*j)+29] = sprite_position;
    r->vertex_buffer[(36*j)+30] = X_PADDING + (blockx * xpos) + (x_size*blockx);
    r->vertex_buffer[(36*j)+31] = Y_PADDING + (blocky * ypos) + (y_size*blocky);
    r->vertex_buffer[(36*j)+32] = depth;
    r->vertex_buffer[(36*j)+33] = 1.0;
    r->vertex_buffer[(36*j)+34] = 1.0;
    r->vertex_buffer[(36*j)+35] = sprite_position;
}

vec2 fp_get_index_linear_frame_position(fp_state* state, int index) {
    float ANIM_TOTAL_LENGTH = 6.0;
    float anim_elapsed = 1.0 + (float) index;
    // TODO (05 Jul 2020 sam): Assuming that we are just doing move left.
    vec2 pos;
    if (state->as.current_animation_index == MOVING_LEFT) {
        pos.x = -anim_elapsed/ANIM_TOTAL_LENGTH;
        pos.y = 0;
    } else if (state->as.current_animation_index == MOVING_RIGHT) {
        pos.x = anim_elapsed/ANIM_TOTAL_LENGTH;
        pos.y = 0;
    } else {
        pos.x = 0;
        pos.y = 0;
    }
    return pos;
}

vec2 fp_get_linear_frame_position(fp_state* state) {
    animation_frames_data afd = state->as.animation_data[state->as.current_animation_index];
    return fp_get_index_linear_frame_position(state, afd.index);
}

vec2 fp_get_index_frame_position(fp_state* state, int index) {
    animation_frames_data afd = state->as.animation_data[state->as.current_animation_index];
    vec2 pos = fp_get_index_linear_frame_position(state, index);
    pos.x += afd.frame_positions[index].x;
    pos.y += afd.frame_positions[index].y;
    return pos;
}

vec2 fp_get_frame_position(fp_state* state) {
    animation_frames_data afd = state->as.animation_data[state->as.current_animation_index];
    return fp_get_index_frame_position(state, afd.index);
}

int fp_create_tools_window(fp_state* state, cb_ui_state* ui_state) {
    add_text(ui_state, &tools_window, "Select Animation", true);
    if (add_button(ui_state, &tools_window, "MOVING_LEFT", true))
        state->as.current_animation_index = MOVING_LEFT;
    if (add_button(ui_state, &tools_window, "STATIC", true))
        state->as.current_animation_index = STATIC;
    if (add_button(ui_state, &tools_window, "MOVING_RIGHT", true))
        state->as.current_animation_index = MOVING_RIGHT;
    if (add_button(ui_state, &tools_window, "PUSHING_LEFT", true))
        state->as.current_animation_index = PUSHING_LEFT;
    if (add_button(ui_state, &tools_window, "SLIPPING", true))
        state->as.current_animation_index = SLIPPING;
    if (add_button(ui_state, &tools_window, "STOPPING_HARD_LEFT", true))
        state->as.current_animation_index = STOPPING_HARD_LEFT;
    new_line(ui_state, &tools_window, false);
    if (add_button(ui_state, &tools_window, "Set FPS to 20", true))
        ANIMATION_FRAMES_PER_SECOND = 20;
    if (add_button(ui_state, &tools_window, "Set FPS to 5", true))
        ANIMATION_FRAMES_PER_SECOND = 5;
    new_line(ui_state, &tools_window, false);
    if (add_button(ui_state, &tools_window, "Save Animation Data", true))
        fp_save_to_file(state);
    cb_render_window(ui_state, &tools_window);
    return 0;
}

int fp_draw_arrays(renderer* r, fp_state* state, float opacity) {
    glLinkProgram(r->shader.shader_program);
    glUseProgram(r->shader.shader_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->shader.texture);
    glBindVertexArray(r->shader.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->shader.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->buffer_size, r->vertex_buffer);
    int position_attribute = glGetAttribLocation(r->shader.shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
    int tex_attribute = glGetAttribLocation(r->shader.shader_program, "tex");
    glEnableVertexAttribArray(tex_attribute);
    glVertexAttribPointer(tex_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*) (3*sizeof(float)));
    int uni_ybyx = glGetUniformLocation(r->shader.shader_program, "ybyx");
    int uni_time = glGetUniformLocation(r->shader.shader_program, "time");
    int uni_opac = glGetUniformLocation(r->shader.shader_program, "opacity");
    glUniform1f(uni_ybyx, WINDOW_HEIGHT*1.0/WINDOW_WIDTH);
    glUniform1f(uni_time, state->seconds);
    glUniform1f(uni_opac, opacity);
    glViewport(0, 0, r->size[0], r->size[1]);
    glDrawArrays(GL_TRIANGLES, 0, r->buffer_size);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    memset(r->vertex_buffer, 0, r->buffer_size);
    r->buffer_occupied = 0;
    return 0;
}

int render_frame_perfect(renderer* r, fp_state* state) {
    glClearColor(0.4, 0.4, 0.401, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    animation_frames_data afd = state->as.animation_data[state->as.current_animation_index];
    if (state->mode == FP_ALL) {
        for (int i=0; i<afd.length; i++) {
            float sprite_position = 7.0+(float) afd.frame_list[i];
            vec2 pos = fp_get_index_frame_position(state, i);
            fp_add_vertex_to_buffer(r, state->default_x+pos.x, state->default_y+pos.y, 1.0, 2.0, 1.0, sprite_position);
            float opac = 4.0 - abs(afd.index - i);
            opac = opac/4.0;
            fp_draw_arrays(r, state, opac);
        }
    } else {
        float sprite_position = 7.0+(float) afd.frame_list[afd.index];
        vec2 pos = fp_get_frame_position(state);
        fp_add_vertex_to_buffer(r, state->default_x+pos.x, state->default_y+pos.y, 1.0, 2.0, 1.0, sprite_position);
        fp_draw_arrays(r, state, 1.0);
    }
    return 0;
}

int init_fp_state(fp_state* state) {
    fp_load_animation_data_from_file(state);
    state->as.current_animation_index = MOVING_LEFT;
    state->mode = FP_PLAY;
    state->seconds = 0.0;
    state->animation_seconds_update = 0.0;
    state->default_x = 2;
    state->default_y = 2;
    return 0;
}

int fp_save_to_file(fp_state* state) {
    FILE* anim_file = fopen("player_animations.txt", "w");
    for (int i=0; i<ANIMATIONS_COUNT; i++) {
        fprintf(anim_file, "%s\n", get_anim_name(i));
        animation_frames_data afd = state->as.animation_data[i];
        fprintf(anim_file, "%i\n", afd.length);
        for (int j=0; j<afd.length; j++)
            fprintf(anim_file, "%i %f %f\n", afd.frame_list[j], afd.frame_positions[j].x, afd.frame_positions[j].y);
    }
    fclose(anim_file);
    return 0;
}

int fp_load_animation_data_from_file(fp_state* state) {
    FILE* anim_file = fopen("player_animations.txt", "r");
    for (int i=0; i<ANIMATIONS_COUNT; i++) {
        char* anim_name[128];
        fscanf(anim_file, "%s\n", &anim_name);
        printf("loading animations %s\n", anim_name);
        animations a = get_anim_from_name(anim_name);
        printf("loading animations a= %s\n", get_anim_name(a));
        animation_frames_data afd;
        fscanf(anim_file, "%i\n", &afd.length);
        for (int j=0; j<afd.length; j++)
            fscanf(anim_file, "%i %f %f\n", &afd.frame_list[j], &afd.frame_positions[j].x, &afd.frame_positions[j].y);
        state->as.animation_data[a] = afd;
    }
    fclose(anim_file);
    return 0;
}

int simulate_frame_perfect(fp_state* state) {
    if (state->mode == FP_PLAY && state->seconds - state->animation_seconds_update > 1.0 / (float) ANIMATION_FRAMES_PER_SECOND)
        fp_next_frame(state, false);
    return 0;
}

float fp_get_world_x() {
    float xpos = mouse.current_x;
    return -0.5 + (((xpos - (X_PADDING*WINDOW_WIDTH/2.0) - (WINDOW_WIDTH/2.0)) /
             (BLOCK_WIDTH/2.0))
            );
}

float fp_get_world_y() {
    float ypos = mouse.current_y;
    return (0.0 - ((ypos+ (Y_PADDING*WINDOW_HEIGHT/2.0) - WINDOW_HEIGHT/2.0) /
                   (BLOCK_HEIGHT/2.0))
            );
}

int fp_handle_mouse_events(fp_state* state) {
    if (state->edit_x) {
        animation_frames_data afd = state->as.animation_data[state->as.current_animation_index];
        vec2 pos = fp_get_linear_frame_position(state);
        float new_x = fp_get_world_x();
        afd.frame_positions[afd.index].x = new_x - (state->default_x+pos.x);
        state->as.animation_data[state->as.current_animation_index] = afd;
    }
    return 0;
}

int fp_clear_mouse(fp_state* state) {
    mouse.l_released = false;
    mouse.r_released = false;
}

int main(int argc, char** argv) {
    renderer r;
    init_renderer(&r, "frame perfect");
    cb_ui_state ui_state;
    init_ui(&ui_state);
    init_fp_state(&state);
    fp_set_callbacks(r.window);
    clock_t start_time;
    clock_t clock_time;
    float frame_time;
    start_time = clock();
    uint window_pos[2] = {20, 40};
    uint window_size[2] = {UI_WIDTH, WINDOW_HEIGHT};
    init_cb_window(&tools_window, "Frame Editor", window_pos, window_size);
    while (!glfwWindowShouldClose(r.window)) {
        clock_time = clock();
        frame_time = ((double)clock_time-(double)start_time)/CLOCKS_PER_SEC;
        start_time = clock_time;
        state.seconds += frame_time;
        glfwPollEvents();
        ui_state.mouse = mouse;
        fp_handle_mouse_events(&state);
        simulate_frame_perfect(&state);
        render_frame_perfect(&r, &state);
        cb_ui_render_text(&ui_state, fp_get_mode(&state), WINDOW_WIDTH-100, 20);
        fp_create_tools_window(&state, &ui_state);
        render_ui(&ui_state);
        fp_clear_mouse(&state);
        glfwSwapBuffers(r.window);
    }
}
