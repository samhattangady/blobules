#ifndef BLOBULES_GAME_DEFINED
#define BLOBULES_GAME_DEFINED
#include <stdbool.h>
#include "cb_lib/cb_types.h"
#include "ui.h"
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
    bool animating;
    float start_time;
    float duration;
    float x;
    float y;
    int start_x;
    int start_y;
    int dx;
    int dy;
} animation_state;

typedef struct {
    entity_type type;
    uint anim_index;
    uint data;
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
    level_select_struct level_select;
    bool animating;
    uint entities_occupied;
    uint animations_occupied;
    // TODO (14 Jun 2020 sam): Figure out whether this stuff should actually
    // be on the heap... I had moved it onto the stack at some point, but I
    // don't really know whether that is more efficient/performant. Also, this
    // whole system might need to be redesigned, and ideally put into a single
    // struct.
    uint grid_data[MAX_WORLD_ENTITIES];
    entity_data entities[MAX_WORLD_ENTITIES];
    animation_state animations[MAX_WORLD_ENTITIES/16];
    vec3i player_position;
    player_input input;
    player_state player;
    // uint current_level;
    // levels_list levels;
    float seconds;
    editor_data editor;
    world_history history;
} world;

int init_main_menu(world* w);
int next_option(world* w);
int previous_option(world* w);
int select_active_option(world* w);


int init_world(world* w, uint number);
int get_position_index(world* w, int x, int y, int z);
int process_inputs(GLFWwindow* window, world* w, float seconds);
char* as_text (entity_type et);
int change_world_xsize(world* w, int direction, int sign);
int change_world_ysize(world* w, int direction, int sign);
int save_level(world* w);
entity_type get_entity_at(world* w, int index);
uint get_entity_anim_index(world* w, int index);

#endif
