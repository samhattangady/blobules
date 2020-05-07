#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "cb_lib/cb_types.h"
#include "cb_lib/cb_string.h"
#include "game.h"
#include "game_settings.h"

#define INPUT_LAG 0.05
#define LEVEL "levels/000.txt"
#define LEVEL_LISTING "levels/listing.txt"

// TODO (18 Apr 2020 sam): This is required to get the inputs working correctly
// with callbacks. See if there is a better way to accomplish this.
world* global_w;
float global_seconds;
float global_frame_change_seconds;

int get_position_index_sizes(int x_size, int y_size, int z_size, int x, int y, int z) {
    return ( z * x_size * y_size ) + ( y * x_size ) + x;
}

int get_position_index(world* w, int x, int y, int z) {
    return get_position_index_sizes(w->x_size, w->y_size, w->z_size, x, y, z);
}

char* as_text(entity_type e) {
    if (e == NONE)
        return "NONE";
    if (e == GROUND)
        return "GROUND";
    if (e == PLAYER)
        return "PLAYER";
    if (e == WALL)
        return "WALL";
    if (e == CUBE)
        return "CUBE";
    if (e == HOT_TARGET)
        return "HOT_TARGET";
    if (e == COLD_TARGET)
        return "COLD_TARGET";
    if (e == SLIPPERY_GROUND)
        return "SLIPPERY_GROUND";
    if (e == FURNITURE)
        return "FURNITURE";
    return "INVALID";
}

int set_none(world* w, int index) {
    w->grid_data[index] = 0;
    return 0;
}
int set_ground(world* w, int index) {
    w->grid_data[index] = 1;
    return 0;
}
int set_slippery(world* w, int index) {
    w->grid_data[index] = 2;
    return 0;
}
int set_cold_target(world* w, int index) {
    w->grid_data[index] = 4;
    return 0;
}

int add_entity(world* w, entity_type e, int x, int y, int z) {
    if (x < 0 || y < 0 || z  < 0)
        return -1;
    if (x >= w->x_size || y >= w->y_size || z >= w->z_size)
        return -1;
    int index = get_position_index(w, x, y, z);
    int entity_index;
    if (e == NONE)
        entity_index = 0;
    else if (e == GROUND)
        entity_index = 1;
    else if (e == GROUND)
        entity_index = 2;
    else {
        entity_index = w->entities_occupied;
        entity_data ed = {e};
        w->entities[entity_index] = ed;
        w->entities_occupied++;
    }
    w->grid_data[index] = entity_index;
    if (e == PLAYER) {
        vec3i pos = {x, y, z};
        w->player_position = pos;
    }
    return 0;
}

entity_type get_entity_at(world* w, int index) {
    return w->entities[w->grid_data[index]].type;
}

entity_type get_entity_type(char c) {
    if (c == '.')
        return NONE;
    if (c == 'o')
        return GROUND;
    if (c == 'P')
        return PLAYER;
    if (c == 'B')
        return WALL;
    if (c == 'I')
        return CUBE;
    if (c == 'T')
        return HOT_TARGET;
    if (c == '~')
        return SLIPPERY_GROUND;
    if (c == 'F')
        return FURNITURE;
    if (c == 'C')
        return COLD_TARGET;
    if (c == 'D')
        return DESTROYED_TARGET;
    return INVALID;
}

char get_entity_char(entity_type et) {
    if (et ==NONE)
        return  '.';
    if (et == GROUND)
        return 'o';
    if (et == PLAYER)
        return 'P';
    if (et == WALL)
        return 'B';
    if (et == CUBE)
        return 'I';
    if (et == HOT_TARGET)
        return 'T';
    if (et == SLIPPERY_GROUND)
        return '~';
    if (et == FURNITURE)
        return 'F';
    if (et == COLD_TARGET)
        return 'C';
    if (et == DESTROYED_TARGET)
        return 'D';
    return ' ';
}

levels_list load_levels_list() {
    // TODO (15 Apr 2020 sam): PERFORMANCE. Looping through string twice
    string levels = read_file(LEVEL_LISTING);
    uint n = 0;
    char c;
    for (int i=0; true; i++) {
        if (levels.text[i] == '\n')
            n++;
        if (levels.text[i] == '\0')
            break;
    }
    printf("mallocing... load_leves_list\n");
    string* l = (string*) malloc(sizeof(string) * n);
    string tmp = empty_string();
    n = 0;
    for (int i=0; true; i++) {
        c = levels.text[i];
        if (c == '\n') {
            l[n] = string_from(tmp.text);
            n++;
            clear_string(&tmp);
        }
        else if (c == '\0')
            break;
        else
            append_sprintf(&tmp, "%c", c);
    }
    levels_list list = {n, l};
    return list;
}

int init_entities(world* w) {
    // Initialise all the entities that will not have additional data
    // or which can have only one instance
    w->entities_occupied = 5;
    entity_data ed;
    ed.type = NONE;
    w->entities[0] = ed;
    ed.type = GROUND;
    w->entities[1] = ed;
    ed.type = SLIPPERY_GROUND;
    w->entities[2] = ed;
    ed.type = HOT_TARGET;
    w->entities[3] = ed;
    ed.type = COLD_TARGET;
    w->entities[4] = ed;
}

int load_level(world* w) {
    w->player = ALIVE;
    init_entities(w);
    int x, y, z;
    char c = ' ';
    char* level_name = w->levels.levels[w->current_level].text;
    printf("loading level %s\n", level_name);
    FILE* level_file = fopen(level_name, "r");
    fscanf(level_file, "%i %i %i\n", &x, &y, &z);
    w->size = x * y * z;
    if (w->size > MAX_WORLD_ENTITIES)
        printf("not enough room malloced for all entities... Might crash.");
    w->x_size = x;
    w->y_size = y;
    w->z_size = z;
    for (z=0; z<w->z_size; z++) {
        for (y=w->y_size-1; y>=0; y--) {
            for (x=0; x<w->x_size; x++) {
                vec3i position = {x, y, z};
                entity_type type = INVALID;
                while (type == INVALID) {
                    fscanf(level_file, "%c", &c);
                    type = get_entity_type(c);
                }
                add_entity(w, type, x, y, z);
            }
        }
    }
    fclose(level_file);
    global_w = w;
    save_freezeframe(w);
    return 0;
}

int save_level(world* w) {
    int x, y, z;
    int index;
    char c = ' ';
    char* level_name = w->levels.levels[w->current_level].text;
    printf("saving level %s\n", level_name);
    FILE* level_file = fopen(level_name, "w");
    fprintf(level_file, "%i %i %i\n", w->x_size, w->y_size, w->z_size);
    for (z=0; z<w->z_size; z++) {
        for (y=w->y_size-1; y>=0; y--) {
            for (x=0; x<w->x_size; x++) {
                index = get_position_index(w, x, y, z);
                c = get_entity_char(get_entity_at(w, index));
                fprintf(level_file, "%c", c);
            }
            fprintf(level_file, "\n");
        }
        fprintf(level_file, "\n");
    }
    fclose(level_file);
    return 0;
}

int load_next_level(world* w) {
    w->current_level++;
    if (w->current_level == w->levels.size)
        w->current_level = 0;
    load_level(w);
}
int load_previous_level(world* w) {
    w->current_level--;
    if (w->current_level == -1)
        w->current_level = w->levels.size-1;
    load_level(w);
}

int init_world(world* w, uint number) {
    player_input input = {NO_INPUT, 0.0};
    levels_list list = load_levels_list();
    world_freezeframe* frames = (world_freezeframe*) malloc(HISTORY_STEPS * sizeof(world_freezeframe));
    world_history history = {0, frames};
    uint* anim_frames = (uint*) malloc(MAX_WORLD_ENTITIES*sizeof(uint));
    for (int i=0; i<MAX_WORLD_ENTITIES; i++)
        anim_frames[i] = 0;
    world tmp = {0, 0, 0, 0, {}, 0, {}, anim_frames, {0, 0, 0}, input, ALIVE, 0, list,
                 0.0, {true, 0, GROUND, {false, false, 0.0, 0.0}}, NULL, {}, history};
    *w = tmp;
    load_level(w);
    return 0;
}

int can_support_player(entity_type et) {
    if (et == GROUND ||
        et == COLD_TARGET ||
        et == SLIPPERY_GROUND)
        return 1;
    return 0;
}

int can_support_cube(entity_type et) {
    if (et == GROUND ||
        et == HOT_TARGET ||
        et == SLIPPERY_GROUND)
        return 1;
    return 0;
}

int can_support_furniture(entity_type et) {
    if (et == GROUND ||
        et == SLIPPERY_GROUND)
        return 1;
    return 0;
}

int can_stop_cube_slide(entity_type et) {
    if (et == CUBE ||
        et == FURNITURE ||
        et == WALL)
        return true;
    return false;
}

int maybe_move_cube(world* w, int x, int y, int z, int dx, int dy, int dz) {
    int on_index = get_position_index(w, x, y, z-1);
    if (get_entity_at(w, on_index) != HOT_TARGET)
        set_slippery(w, on_index);
    int index = get_position_index(w, x, y, z);
    if ((x+dx < 0 || x+dx > w->x_size-1) ||
        (y+dy < 0 || y+dy > w->y_size-1) ||
        (z+dz < 0 || z+dz > w->z_size-1)) {
        // remove cube
        // TODO (07 May 2020 sam): Figure out how to handle animations here?
        printf("removing cube because oob\n");
        set_none(w, index);
        return -1;
    }
    // see what's already in desired place
    int target_pos_index = get_position_index(w, x+dx, y+dy, z+dz);
    if (get_entity_at(w, target_pos_index) != NONE) {
        if (can_stop_cube_slide(get_entity_at(w, target_pos_index))) {
            if (get_entity_at(w, target_pos_index) == CUBE)
                maybe_move_cube(w, x+dx, y+dy, z+dz, dx, dy, dz);
            if (get_entity_at(w, target_pos_index) == FURNITURE)
                maybe_move_furniture(w, x+dx, y+dy, z+dz, dx, dy, dz);
            // check if win.
            if (get_entity_at(w, on_index) == HOT_TARGET) {
                printf("extinguiishing fire\n");
                // put out fire
                set_none(w, index);
                set_cold_target(w, on_index);
            }
            return 1;
        }
        // what if it's player?
    }
    int target_on_index = get_position_index(w, x+dx, y+dy, z+dz-1);
    if (!can_support_cube(get_entity_at(w, target_on_index))) {
        // remove cube
        // TODO (07 May 2020 sam): Figure out how to handle animations here?
        printf("removing cube because no support\n");
        set_none(w, index);
        return -2;
    }
    int cube_index = w->grid_data[index];
    w->grid_data[target_pos_index] = cube_index;
    set_none(w, index);
    maybe_move_cube(w, x+dx, y+dy, z+dz, dx, dy, dz);
    return 0;
}

int maybe_move_furniture(world* w, int x, int y, int z, int dx, int dy, int dz) {
    int on_index = get_position_index(w, x, y, z-1);
    int index = get_position_index(w, x, y, z);
    if ((x+dx < 0 || x+dx > w->x_size-1) ||
        (y+dy < 0 || y+dy > w->y_size-1) ||
        (z+dz < 0 || z+dz > w->z_size-1)) {
        printf("removing furniture because oob\n");
        set_none(w, index);
        return -1;
    }
    // see what's already in desired place
    int target_pos_index = get_position_index(w, x+dx, y+dy, z+dz);
    if (get_entity_at(w, target_pos_index) != NONE) {
        if (get_entity_at(w, target_pos_index) == WALL)
            return 1;
        if (get_entity_at(w, target_pos_index) == FURNITURE)
            return 1;
        if (get_entity_at(w, target_pos_index) == CUBE) {
            return 1;
        }
        // what if it's player?
    }
    int target_on_index = get_position_index(w, x+dx, y+dy, z+dz-1);
    if (!can_support_furniture(get_entity_at(w, target_on_index))) {
        printf("removing furniture because no support\n");
        set_none(w, index);
        return -2;
    }
    int furniture_index = w->grid_data[index];
    w->grid_data[target_pos_index] = furniture_index;
    set_none(w, index);
    if (get_entity_at(w, target_on_index) == SLIPPERY_GROUND)
        maybe_move_furniture(w, x+dx, y+dy, z+dz, dx, dy, dz);
    return 0;
}

bool can_push_player_back(entity_type et) {
    if (et == CUBE ||
        et == FURNITURE ||
        et == WALL)
        return true;
    return false;
}

int maybe_move_player(world* w, int dx, int dy, int dz, bool force) {
    vec3i pos = w->player_position;
    int position_index = get_position_index(w, pos.x, pos.y, pos.z);
    int ground_index = get_position_index(w, pos.x, pos.y, pos.z-1);
    int target_index = get_position_index(w, pos.x+dx, pos.y+dy, pos.z+dz);
    int target_ground_index = get_position_index(w, pos.x+dx, pos.y+dy, pos.z+dz-1);
    // check out of bounds;
    if ((pos.x+dx < 0 || pos.x+dx > w->x_size-1) ||
        (pos.y+dy < 0 || pos.y+dy > w->y_size-1) ||
        (pos.z+dz < 0 || pos.z+dz > w->z_size-1)) {
        if (force) {
            w->player = DEAD;
            w->player_position.x += dx;
            w->player_position.y += dy;
            w->player_position.z += dz;
            set_none(w, index);
            return 0;
        }
        return -1;
    }
    // see what's already in desired place
    if (get_entity_at(w, target_index) != NONE) {
        if (can_push_player_back(get_entity_at(w, target_index)) && !force) {
            if (get_entity_at(w, ground_index) == SLIPPERY_GROUND)
                maybe_move_player(w, -dx, -dy, -dz, true);
        }
        if (get_entity_at(w, target_index) == CUBE)
            maybe_move_cube(w, pos.x+dx, pos.y+dy, pos.z+dz, dx, dy, dz);
        if (get_entity_at(w, target_index) == FURNITURE)
            maybe_move_furniture(w, pos.x+dx, pos.y+dy, pos.z+dz, dx, dy, dz);
        return 1;
    }
    // check if can stand
    if (!can_support_player(get_entity_at(w, target_ground_index))) {
        if (force) {
            w->player = DEAD;
            w->player_position.x += dx;
            w->player_position.y += dy;
            w->player_position.z += dz;
            set_none(w, position_index);
            return 0;
        }
        return -2;
    }
    if (get_entity_at(w, ground_index) == SLIPPERY_GROUND && !force)
        return 0;
    w->player_position.x += dx;
    w->player_position.y += dy;
    w->player_position.z += dz;
    int player_index = w->grid_data[position_index];
    w->grid_data[target_index] = player_index;
    set_none(w, position_index);
    if (get_entity_at(w, target_ground_index) == SLIPPERY_GROUND) {
        maybe_move_player(w, dx, dy, dz, true);
    }
    if (get_entity_at(w, target_ground_index) == COLD_TARGET) {
        w->player = WIN;
    }
    return 0;
}

int save_freezeframe(world* w) {
    int i, index;
    w->history.index++;
    if (w->history.index == HISTORY_STEPS) {
        // When the index is above HISTORY_STEPS, move the whole
        // array backwards by HISTORY_STEPS/2
        int num = HISTORY_STEPS/2;
        memcpy(&w->history.history[0], &w->history.history[num], num*sizeof(world_freezeframe));
        printf("history memory full...\n");
        w->history.index = num;
    }
    index = w->history.index;
    w->history.history[index].current_level = w->current_level;
    for (i=0; i<w->size; i++)
        w->history.history[index].entities[i] = get_entity_char(get_entity_at(w, i));
}

int load_previous_freezeframe(world* w) {
    int i, index;
    if (w->history.index <= 1) return 0;
    w->history.index--;
    index = w->history.index;
    if (w->history.history[index].current_level != w->current_level) {
        // TODO (27 Apr 2020 sam): Changing level requires us to press z twice?
        // Bug needs to be found and fixed.
        w->current_level = w->history.history[index].current_level;
        load_level(w);
    }
    init_entities(w);
    for (i=0; i<w->size; i++) {
        entity_type et = get_entity_type(w->history.history[index].entities[i]);
        int x, y, z;
        z = i / (w->x_size * w->y_size);
        y = (i - (z*w->x_size*w->y_size)) / w->x_size;
        x = i % w->x_size;
        add_entity(w, et, x, y, z);
        if (et == PLAYER) {
            vec3i pos = {x, y, z};
            w->player_position = pos;
        }
    }
}

int trigger_input(world* w, input_type it) {
    vec3i pos = w->player_position;
    if (it == MOVE_UP)
        maybe_move_player(w, 0, 1, 0, false);
    if (it == MOVE_DOWN)
        maybe_move_player(w, 0, -1, 0, false);
    if (it == MOVE_RIGHT)
        maybe_move_player(w, 1, 0, 0, false);
    if (it == MOVE_LEFT)
        maybe_move_player(w, -1, 0, 0, false);
    if (it == RESTART_LEVEL)
        load_level(w);
    if (it == NEXT_LEVEL)
        load_next_level(w);
    if (it == PREVIOUS_LEVEL)
        load_previous_level(w);
    if (it == UNDO_MOVE)
        load_previous_freezeframe(w);
}

int set_input(world* w, input_type it, float seconds) {
    if ((w->input.type == NO_INPUT) || ((seconds - w->input.time) > INPUT_LAG)) {
        w->input.type = it;
        w->input.time = seconds;
        trigger_input(w, it);
        // TODO (27 Apr 2020 sam): Figure out exactly when to save the freezeframe
        if (it != UNDO_MOVE)
            save_freezeframe(w);
    }
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        set_input(global_w, RESTART_LEVEL, global_seconds);
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
        set_input(global_w, NEXT_LEVEL, global_seconds);
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        set_input(global_w, PREVIOUS_LEVEL, global_seconds);
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
        set_input(global_w, MOVE_UP, global_seconds);
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
        set_input(global_w, MOVE_DOWN, global_seconds);
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
        set_input(global_w, MOVE_LEFT, global_seconds);
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
        set_input(global_w, MOVE_RIGHT, global_seconds);
    if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT))
        set_input(global_w, UNDO_MOVE, global_seconds);
    if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
        global_w->editor.z_level = (global_w->editor.z_level+1)  % 2;
    if (key == GLFW_KEY_TAB && (action == GLFW_PRESS || action == GLFW_REPEAT))
        global_w->editor.active_type = (global_w->editor.active_type+1)  % INVALID;
    if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
        global_w->editor.editor_enabled =  !global_w->editor.editor_enabled;
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    global_w->editor.mouse.xpos = xpos;
    global_w->editor.mouse.ypos = ypos;
    global_w->ui_state->mouse.current_x = xpos;
    global_w->ui_state->mouse.current_y = ypos;
}

int get_world_x(world* w) {
    double xpos = global_w->editor.mouse.xpos;
    return (int) (0.5 + ((xpos - (X_PADDING*WINDOW_WIDTH/2.0) - (WINDOW_WIDTH/2.0)) /
                         (BLOCK_SIZE/2.0))
                 );
}

int get_world_y(world* w) {
    double ypos = global_w->editor.mouse.ypos;
    return (int) (0.5 - ((ypos+ (Y_PADDING*WINDOW_HEIGHT/2.0) - WINDOW_HEIGHT/2.0) /
                         (BLOCK_SIZE/2.0))
                 );
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    global_w->editor.mouse.l_pressed = (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS);
    global_w->editor.mouse.r_pressed = (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        global_w->ui_state->mouse.l_pressed = true;
        global_w->ui_state->mouse.l_down_x = global_w->ui_state->mouse.current_x;
        global_w->ui_state->mouse.l_down_y = global_w->ui_state->mouse.current_y;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        global_w->ui_state->mouse.l_pressed = false;
        global_w->ui_state->mouse.l_released = true;
    }
    if (global_w->editor.mouse.l_pressed) {
        int x = get_world_x(global_w);
        int y = get_world_y(global_w);
        // TODO (19 Apr 2020 sam): Add check here to see whether the type is
        // on "correct z level"
        if (global_w->editor.editor_enabled) {
            add_entity(global_w, global_w->editor.active_type, x, y, global_w->editor.z_level);
            save_freezeframe(global_w);
        }
    }
    if (global_w->editor.mouse.r_pressed) {
        int x = get_world_x(global_w);
        int y = get_world_y(global_w);
        if (global_w->editor.editor_enabled) {
            add_entity(global_w, NONE, x, y, global_w->editor.z_level);
            save_freezeframe(global_w);
        }
    }
}

int process_inputs(GLFWwindow* window, world* w, float seconds) {
    if (w->player == WIN)
        load_next_level(w);
    // if (w->player == DEAD)
    //     load_level(w);
    global_seconds = seconds;
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (seconds-global_frame_change_seconds >= 1.0/12.0) {
        uint current_frame = w->animation_frames[0];
        current_frame += 1;
        current_frame %= 3;
        for (int i=0; i<MAX_WORLD_ENTITIES; i++)
            w->animation_frames[i] = current_frame+1;
        global_frame_change_seconds = seconds;
    }
    w->seconds = seconds;
}

// int change_world_xsize_right(world* w, int sign) {
//     int ogx = w->x_size;
//     printf("mallocing... change_world_size\n");
//     entity_type* old_entities = malloc(w->size * sizeof(entity_type));
//     memcpy(old_entities, ccc_entities, w->size*sizeof(entity_type));
//     // TODO (19 Apr 2020 sam): This should get the sign value. It's wrong here.
//     w->x_size = w->x_size + (1.0 * sign);
//     w->size = w->x_size * w->y_size * w->z_size;
//     for (int z=0; z<w->z_size; z++) {
//         for (int y=0; y<w->y_size; y++) {
//             for (int x=0; x<w->x_size; x++) {
//                 int index = get_position_index_sizes(ogx, w->y_size, w->z_size, x, y, z);
//                 if (x==w->x_size-1 && sign>0)
//                     add_entity(w, NONE, x, y, z);
//                 else
//                     add_entity(w, old_entities[index], x, y, z);
//             }
//         }
//     }
//     free(old_entities);
// }
// 
// int change_world_xsize_left(world* w, int sign) {
//     int ogx = w->x_size;
//     printf("mallocing... change_world_size\n");
//     entity_type* old_entities = malloc(w->size * sizeof(entity_type));
//     memcpy(old_entities, ccc_entities, w->size*sizeof(entity_type));
//     // TODO (19 Apr 2020 sam): This should get the sign value. It's wrong here.
//     w->x_size = w->x_size + (1.0 * sign);
//     w->size = w->x_size * w->y_size * w->z_size;
//     for (int z=0; z<w->z_size; z++) {
//         for (int y=0; y<w->y_size; y++) {
//             for (int x=0; x<w->x_size; x++) {
//                 if (sign>0){
//                     int index = get_position_index_sizes(ogx, w->y_size, w->z_size, x-1, y, z);
//                     if (x==0)
//                         add_entity(w, NONE, 0, y, z);
//                     else
//                         add_entity(w, old_entities[index], x, y, z);
//                 }
//                 else {
//                     int index = get_position_index_sizes(ogx, w->y_size, w->z_size, x+1, y, z);
//                     add_entity(w, old_entities[index], x, y, z);
//                 }
//             }
//         }
//     }
//     free(old_entities);
// }
// 
// 
// int change_world_ysize_top(world *w, int sign) {
//     int ogy = w->y_size;
//     printf("mallocing... change_world_size\n");
//     entity_type* old_entities = malloc(w->size * sizeof(entity_type));
//     memcpy(old_entities, ccc_entities, w->size*sizeof(entity_type));
//     // TODO (19 Apr 2020 sam): This should get the sign value. It's wrong here.
//     w->y_size = w->y_size + (1.0 * sign);
//     w->size = w->x_size * w->y_size * w->z_size;
//     for (int z=0; z<w->z_size; z++) {
//         for (int y=0; y<w->y_size; y++) {
//             for (int x=0; x<w->x_size; x++) {
//                 int index = get_position_index_sizes(w->x_size, ogy, w->z_size, x, y, z);
//                 if (y==w->y_size-1 && sign>0)
//                     add_entity(w, NONE, x, y, z);
//                 else
//                     add_entity(w, old_entities[index], x, y, z);
//             }
//         }
//     }
//     free(old_entities);
// }
// 
// int change_world_ysize_bottom(world *w, int sign) {
//     int ogy = w->y_size;
//     printf("mallocing... change_world_size\n");
//     entity_type* old_entities = malloc(w->size * sizeof(entity_type));
//     memcpy(old_entities, ccc_entities, w->size*sizeof(entity_type));
//     // TODO (19 Apr 2020 sam): This should get the sign value. It's wrong here.
//     w->y_size = w->y_size + (1.0 * sign);
//     w->size = w->x_size * w->y_size * w->z_size;
//     for (int z=0; z<w->z_size; z++) {
//         for (int y=0; y<w->y_size; y++) {
//             for (int x=0; x<w->x_size; x++) {
//                 if (sign>0){
//                     int index = get_position_index_sizes(w->x_size, ogy, w->z_size, x, y-1, z);
//                     if (y==0)
//                         add_entity(w, NONE, x, 0, z);
//                     else
//                         add_entity(w, old_entities[index], x, y, z);
//                 }
//                 else {
//                     int index = get_position_index_sizes(w->x_size, ogy, w->z_size, x, y+1, z);
//                     add_entity(w, old_entities[index], x, y, z);
//                 }
//             }
//         }
//     }
//     free(old_entities);
// }
// 
// int change_world_xsize(world* w, int direction, int sign) {
//     if (direction == 1)
//         change_world_xsize_right(w, sign);
//     if (direction == -1)
//         change_world_xsize_left(w, sign);
// }
// 
// int change_world_ysize(world* w, int direction, int sign) {
//     if (direction == 1)
//         change_world_ysize_top(w, sign);
//     else
//         change_world_ysize_bottom(w, sign);
// }
int change_world_xsize(world* w, int direction, int sign) {
    return 0;
}
int change_world_ysize(world* w, int direction, int sign) {
    return 0;
}
