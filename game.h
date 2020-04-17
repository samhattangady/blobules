#ifndef BLOBULES_GAME_DEFINED
#define BLOBULES_GAME_DEFINED
#include "cb_lib/cb_types.h"
#include <GLFW/glfw3.h>

typedef enum {
    PLAYER,
    CUBE,
    WALL,
    GROUND,
    SLIPPERY_GROUND,
    HOT_TARGET,
    COLD_TARGET,
    NONE,
    INVALID,
} entity_type;

typedef enum {
    NO_INPUT,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    NEXT_LEVEL,
    PREVIOUS_LEVEL,
} input_type;

typedef enum {
    ALIVE,
    DEAD,
    WIN,
} player_state;

typedef struct {
    input_type type;
    float time;
} player_input;

typedef struct {
    uint size;
    string* levels;
} levels_list;

typedef struct {
    uint size;
    uint x_size;
    uint y_size;
    uint z_size;
    entity_type* entities;
    vec3i player_position;
    player_input input;
    player_state player;
    uint current_level;
    levels_list levels;
    float seconds;
} world;

int init_world(world* w, uint number);
int get_position_index(world* w, int x, int y, int z);
int process_inputs(GLFWwindow* window, world* w, float seconds);

#endif
