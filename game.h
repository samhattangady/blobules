#ifndef BLOBULES_GAME_DEFINED
#define BLOBULES_GAME_DEFINED
#include <stdbool.h>
#include "cb_lib/cb_types.h"
#include "ui.h"
#include "boombox.h"
#include "game_settings.h"
#include "common.h"

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
    REFLECTOR,
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
    bool editor_enabled;
    uint z_level;
    entity_type active_type;
    cb_ui_state* ui_state;
    cb_window ui_window;
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
    bool currently_moving;
    float start_time;
    float duration;
    float x;
    float y;
    float z;
    int start_x;
    int start_y;
    int dx;
    int dy;
} movement_state;

typedef enum {
    STATIC,
    MOVING_LEFT,
    MOVING_RIGHT,
    PUSHING_LEFT,
    SLIPPING,
    STOPPING_HARD_LEFT,
} animations;

typedef struct {
    bool looping;
    uint length;
    uint index;
    int frame_list[NUM_ANIMATION_FRAMES];
} animation_frames_data;

typedef struct {
    // TODO (01 Jul 2020 sam): We might need to add a next animation, or somehow queue animations
    bool currently_animating;    
    uint current_animation_index;
    uint default_animation_index;
    uint queue_length;
    animations queue[MAX_QUEUED_ANIMATIONS];
    animation_frames_data animation_data[ENTITY_NUM_ANIMATIONS];
} animation_state;

typedef struct {
    entity_type type;
    // TODO (01 Jul 2020 sam): rename to movement_index
    uint movement_index;
    uint animation_index;
    uint data;
    int x;
    int y;
    int z;
    float removal_time;
} entity_data;

typedef enum {
    MAIN_MENU,
    LEVEL_SELECT,
    IN_GAME,
    EXIT,
} world_mode;

typedef struct {
    string text;
} menu_option;

typedef struct {
    int active_option;
    uint total_options;
    menu_option options[10];
} main_menu_struct;

typedef struct {
    string name;
    // TODO (15 Jun 2020 sam): This is unused. Currently we still load data from file.
    string data;
    float xpos;
    float ypos;
    // uint index;
    int up_index;
    int down_index;
    int left_index;
    int right_index;
} level_option;

typedef enum {
    NEUTRAL,
    SET_POSITION,
    SET_LEFT,
    SET_RIGHT,
    SET_TOP,
    SET_BOTTOM,
    LEVEL_EDITOR_MODES_COUNT,
} level_editor_modes;

typedef struct {
    bool level_editor_enabled;
} level_editor;

typedef struct {
    level_option* levels;
    uint current_level;
    uint total_levels;
} level_select_struct;

typedef struct {
    uint size;
    uint x_size;
    uint y_size;
    uint z_size;
    world_mode active_mode;
    main_menu_struct main_menu;
    level_editor_modes level_mode;
    level_select_struct level_select;
    bool currently_moving;
    uint entities_occupied;
    uint movements_occupied;
    uint animations_occupied;
    boombox* boom;
    mouse_data mouse;
    void* data;
    uint* grid_data;
    entity_data* entities;
    movement_state* movements;
    animation_state* animations;
    vec3i player_position;
    player_input input;
    player_state player;
    // uint current_level;
    // levels_list levels;
    float seconds;
    float animation_seconds_update;
    editor_data editor;
    world_history history;
} world;

int init_main_menu(world* w);
int next_option(world* w);
int previous_option(world* w);
int select_active_option(world* w);


int init_world(world* w, uint number);
int get_position_index(world* w, int x, int y, int z);
int simulate_world(world* w, float seconds);
int reset_inputs(world* w);
char* as_text (entity_type et);
int change_world_xsize(world* w, int direction, int sign);
int change_world_ysize(world* w, int direction, int sign);
int save_level(world* w);
int set_callbacks(GLFWwindow* window);
entity_type get_entity_at(world* w, int index);
uint get_entity_anim_index(world* w, int index);

#endif
