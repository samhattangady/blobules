#ifndef BLOBULES_GAME_DEFINED
#define BLOBULES_GAME_DEFINED
#include <stdbool.h>
#include "cb_lib/cb_types.h"
#include "ui.h"
#include <GLFW/glfw3.h>
#include "game_settings.h"

typedef enum {
    PLAYER,
    CUBE,
    WALL,
    GROUND,
    SLIPPERY_GROUND,
    HOT_TARGET,
    COLD_TARGET,
    DESTROYED_TARGET,
    NONE,
    FURNITURE,
    INVALID,
} entity_type;

typedef enum {
    NO_INPUT,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    RESTART_LEVEL,
    NEXT_LEVEL,
    PREVIOUS_LEVEL,
    UNDO_MOVE,
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
    bool l_pressed;
    bool r_pressed;
    double xpos;
    double ypos;
} mouse_state;

typedef struct {
    bool editor_enabled;
    uint z_level;
    entity_type active_type;
    mouse_state mouse;
} editor_data;

typedef struct {
    uint current_level;
    char entities[MAX_WORLD_ENTITIES];        
} world_freezeframe;

typedef struct {
    uint index;
    world_freezeframe* history;
} world_history;

typedef struct {
    // okay what is required here?
} animation_state;

typedef struct {
    entity_type type;
} entity_data;

typedef struct {
    uint size;
    uint x_size;
    uint y_size;
    uint z_size;
    uint grid_data[MAX_WORLD_ENTITIES];
    uint entities_occupied;
    entity_data entities[MAX_WORLD_ENTITIES];
    uint* animation_frames;
    vec3i player_position;
    player_input input;
    player_state player;
    uint current_level;
    levels_list levels;
    float seconds;
    editor_data editor;
    cb_ui_state* ui_state;
    cb_window ui_window;
    world_history history;
} world;

int init_world(world* w, uint number);
int get_position_index(world* w, int x, int y, int z);
int process_inputs(GLFWwindow* window, world* w, float seconds);
char* as_text (entity_type et);
int change_world_xsize(world* w, int direction, int sign);
int change_world_ysize(world* w, int direction, int sign);
int save_level(world* w);
entity_type get_entity_at(world* w, int index);

#endif
