#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "cb_lib/cb_types.h"
#include "cb_lib/cb_string.h"
#include "game.h"

#define INPUT_LAG 0.2
#define LEVEL "levels/000.txt"
#define LEVEL_LISTING "levels/listing.txt"

int get_position_index(world* w, int x, int y, int z) {
    return ( z * w->x_size * w->y_size ) + ( y * w->x_size ) + x;
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
    return "INVALID";
}

int add_entity(world* w, entity_type e, int x, int y, int z) {
    int index = get_position_index(w, x, y, z);
    w->entities[index] = e;
    return 0;
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
    return INVALID;
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
    for (int i=0; i<n; i++) {
        print_string(&l[i]);
    }
    return list;
}

int load_level(world* w) {
    w->player = ALIVE;
    int x, y, z;
    char c = ' ';
    if (w->entities != NULL)
        free(w->entities);
    char* level_name = w->levels.levels[w->current_level].text;
    printf("loading level %s\n", level_name);
    FILE* level_file = fopen(level_name, "r");
    fscanf(level_file, "%i %i %i\n", &x, &y, &z);
    w->size = x * y * z;
    w->x_size = x;
    w->y_size = y;
    w->z_size = z;
    w->entities = (entity_type*) malloc(w->size * sizeof(entity_type));
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
                if (type == PLAYER)
                    w->player_position = position;
            }
        }
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
    entity_type* entities = (entity_type*) malloc(number * sizeof(entity_type));
    player_input input = {NO_INPUT, 0.0};
    levels_list list = load_levels_list();
    world tmp = {0, 0, 0, 0, NULL, {0, 0, 0}, input, ALIVE, 0, list, 0.0};
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

int maybe_move_cube(world* w, int x, int y, int z, int dx, int dy, int dz) {
    int on_index = get_position_index(w, x, y, z-1);
    if (w->entities[on_index] != HOT_TARGET)
        w->entities[on_index] = SLIPPERY_GROUND;
    int index = get_position_index(w, x, y, z);
    if ((x+dx < 0 || x+dx > w->x_size-1) ||
        (y+dy < 0 || y+dy > w->y_size-1) ||
        (z+dz < 0 || z+dz > w->z_size-1)) {
        // remove cube
        printf("removing cube because oob\n");
        w->entities[index] = NONE;
        return -1;
    }
    // see what's already in desired place
    int target_pos_index = get_position_index(w, x+dx, y+dy, z+dz);
    if (w->entities[target_pos_index] != NONE) {
        if (w->entities[target_pos_index] == WALL) {
            // check if win.
            if (w->entities[on_index] == HOT_TARGET) {
                printf("extinguiishing fire\n");
                // put out fire
                w->entities[index] = NONE;
                w->entities[on_index] = COLD_TARGET;
            }
            return 1;
        }
        // what if it's player?
    }
    int target_on_index = get_position_index(w, x+dx, y+dy, z+dz-1);
    if (!can_support_cube(w->entities[target_on_index])) {
        // remove cube
        printf("removing cube because no support\n");
        w->entities[index] = NONE;
        return -2;
    }
    w->entities[target_pos_index] = CUBE;
    w->entities[index] = NONE;
    maybe_move_cube(w, x+dx, y+dy, z+dz, dx, dy, dz);
    return 0;
}

bool can_push_player_back(entity_type et) {
    if (et == CUBE ||
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
            w->entities[position_index] = NONE;
            return 0;
        }
        return -1;
    }
    // see what's already in desired place
    if (w->entities[target_index] != NONE) {
        if (can_push_player_back(w->entities[target_index]) && !force) {
            if (w->entities[ground_index] == SLIPPERY_GROUND)
            maybe_move_player(w, -dx, -dy, -dz, true);
        }
        if (w->entities[target_index] == CUBE)
            maybe_move_cube(w, pos.x+dx, pos.y+dy, pos.z+dz, dx, dy, dz);
        return 1;
    }
    // check if can stand
    if (!can_support_player(w->entities[target_ground_index])) {
        if (force) {
            w->player = DEAD;
            w->player_position.x += dx;
            w->player_position.y += dy;
            w->player_position.z += dz;
            w->entities[position_index] = NONE;
            return 0;
        }
        return -2;
    }
    if (w->entities[ground_index] == SLIPPERY_GROUND && !force)
        return 0;
    w->player_position.x += dx;
    w->player_position.y += dy;
    w->player_position.z += dz;
    w->entities[position_index] = NONE;
    w->entities[target_index] = PLAYER;
    if (w->entities[target_ground_index] == SLIPPERY_GROUND) {
        maybe_move_player(w, dx, dy, dz, true);
    }
    if (w->entities[target_ground_index] == COLD_TARGET) {
        w->player = WIN;
    }
    return 0;
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
    if (it == NEXT_LEVEL)
        load_next_level(w);
    if (it == PREVIOUS_LEVEL)
        load_previous_level(w);
}

int set_input(world* w, input_type it, float seconds) {
    if ((w->input.type == NO_INPUT) || ((seconds - w->input.time) > INPUT_LAG)) {
        w->input.type = it;
        w->input.time = seconds;
        trigger_input(w, it);
    }
    return 0;
}

int process_inputs(GLFWwindow* window, world* w, float seconds) {
    if (w->player == WIN)
        load_next_level(w);
    if (w->player == DEAD)
        load_level(w);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        set_input(w, MOVE_UP, seconds);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        set_input(w, MOVE_DOWN, seconds);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        set_input(w, MOVE_LEFT, seconds);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        set_input(w, MOVE_RIGHT, seconds);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        load_level(w);
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        set_input(w, NEXT_LEVEL, seconds);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        set_input(w, PREVIOUS_LEVEL, seconds);
    w->seconds = seconds;
}


