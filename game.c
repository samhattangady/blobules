#include <stdlib.h>
#include <stdio.h>
#include "cb_lib/cb_types.h"
#include "game.h"

#define INPUT_LAG 0.2

int get_position_index(world* w, int x, int y, int z) {
    return ( z * w->x_size * w->y_size ) + ( y * w->x_size ) + x;
}

int add_entity(world* w, entity_type e, int x, int y, int z) {
    int index = get_position_index(w, x, y, z);
    w->entities[index] = e;
    return 0;
}

level_data get_level_0() {
    level_data result = {10, 7, 2, "\
        NNNNNNNNNN\
        NGGGGGGGGN\
        NGGGGGGGGN\
        GGGGGGGGGN\
        NGGGGGGGGN\
        NGGGGGGGGN\
        NNNNNNNNNN\
        \
        NNNNNNNNNN\
        NNNNNNNNNN\
        NNNNNNNNNN\
        BNNNNNNINN\
        NNNNNNNNNN\
        NBNNNNNPNN\
        NBNNNNNNNN\
    "};
    return result;
}

level_data get_level1() {
    level_data result = {1, 1, 1, "I"};
    return result;
}

entity_type get_entity_type(char c) {
    if (c == 'N')
        return NONE;
    if (c == 'G')
        return GROUND;
    if (c == 'P')
        return PLAYER;
    if (c == 'B')
        return WALL;
    if (c == 'I')
        return CUBE;
    return INVALID;
}

int load_level(world* w, level_data* l) {
    int x, y, z, i; 
    if (w->entities != NULL)
        free(w->entities);
    w->size = l->x_size * l->y_size * l->z_size;
    w->x_size = l->x_size;
    w->y_size = l->y_size;
    w->z_size = l->z_size;
    w->entities = (entity_type*) malloc(w->size * sizeof(entity_type));
    i = 0;
    for (z=0; z<l->z_size; z++) {
        for (y=l->y_size-1; y>=0; y--) {
            for (x=0; x<l->x_size; x++) {
                vec3i position = {x, y, z};
                entity_type type = INVALID;
                while (type == INVALID) {
                    type = get_entity_type(l->data[i]);
                    i++;
                }
                add_entity(w, type, x, y, z);
                if (type == PLAYER)
                    w->player_position = position;
            }
        }
    }
}

int init_world(world* w, uint number) {
    entity_type* entities = (entity_type*) malloc(number * sizeof(entity_type));
    player_input input = {NO_INPUT, 0.0};
    world tmp = {0, 0, 0, 0, NULL, {0, 0, 0}, input};
    *w = tmp;
    level_data zero = get_level_0();
    load_level(w, &zero);
    return 0;
}

int can_stand(entity_type et) {
    if (et == GROUND ||
        et == SLIPPERY_GROUND)
        return 1;
    return 0;
}

int maybe_move_cube(world* w, int x, int y, int z, int dx, int dy, int dz) {
    printf("cube trying to move to %i, %i, %i\t", x+dx, y+dy, z+dz);
    w->entities[get_position_index(w, x, y, z-1)] = SLIPPERY_GROUND;
    int index = get_position_index(w, x, y, z);
    if ((x+dx < 0 || x+dx > w->x_size-1) ||
        (y+dy < 0 || y+dy > w->y_size-1) ||
        (z+dz < 0 || z+dz > w->z_size-1)) {
        // remove cube
        printf("removing cube because oob\n");
        w->entities[index] = NONE;
        return -1;
    }
    int g_index = get_position_index(w, x+dx, y+dy, z+dz-1);
    if (!can_stand(w->entities[g_index])) {
        // remove cube
        printf("removing cube because no support\n");
        w->entities[index] = NONE;
        return -2;
    }
    // see what's already in desired place
    int t_index = get_position_index(w, x+dx, y+dy, z+dz);
    if (w->entities[t_index] != NONE) {
        if (w->entities[t_index] == WALL) {
            printf("hit wall\n");
            return 1;
        }
        return 1;
    }
    printf("successfully moved \n");
    w->entities[t_index] = CUBE;
    w->entities[index] = NONE;
    w->entities[get_position_index(w, x, y, z-1)] = SLIPPERY_GROUND;
    maybe_move_cube(w, x+dx, y+dy, z+dz, dx, dy, dz); 
    return 0;
}

int maybe_move_player(world* w, int dx, int dy, int dz) {
    // TODO (15 Apr 2020 sam): Add flag here to see whether it was player
    // triggered (move input) or whether it is a player uncontrolled (slide)
    vec3i pos = w->player_position;
    // check out of bounds;
    if ((pos.x+dx < 0 || pos.x+dx > w->x_size-1) ||
        (pos.y+dy < 0 || pos.y+dy > w->y_size-1) ||
        (pos.z+dz < 0 || pos.z+dz > w->z_size-1))
        return -1;
    // check if can stand
    int g_index = get_position_index(w, pos.x+dx, pos.y+dy, pos.z+dz-1);
    if (!can_stand(w->entities[g_index]))
        return -2;
    // see what's already in desired place
    int p_index = get_position_index(w, pos.x, pos.y, pos.z);
    int t_index = get_position_index(w, pos.x+dx, pos.y+dy, pos.z+dz);
    if (w->entities[t_index] != NONE) {
        if (w->entities[t_index] == CUBE)
            maybe_move_cube(w, pos.x+dx, pos.y+dy, pos.z+dz, dx, dy, dz);
        return 1;
    }
    w->player_position.x += dx;
    w->player_position.y += dy;
    w->player_position.z += dz;
    w->entities[p_index] = NONE;
    w->entities[t_index] = PLAYER;
}

int trigger_input(world* w, input_type it) {
    vec3i pos = w->player_position;
    if (it == MOVE_UP)
        maybe_move_player(w, 0, 1, 0);
    if (it == MOVE_DOWN)
        maybe_move_player(w, 0, -1, 0);
    if (it == MOVE_RIGHT)
        maybe_move_player(w, 1, 0, 0);
    if (it == MOVE_LEFT)
        maybe_move_player(w, -1, 0, 0);
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
}


