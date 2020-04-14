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
    NONE,
    INVALID,
} entity_type;

typedef enum {
    NO_INPUT,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
} input_type;

typedef struct {
    input_type type;
    float time;
} player_input;

typedef struct {
    uint size;
    uint x_size;
    uint y_size;
    uint z_size;
    entity_type* entities;
    vec3i player_position;
    player_input input;
} world;

typedef struct {
    uint x_size;
    uint y_size;
    uint z_size;
    char* data;
} level_data;

int init_world(world* w, uint number);
int get_position_index(world* w, int x, int y, int z);
int process_inputs(GLFWwindow* window, world* w, float seconds);

#endif
