#ifndef BLOBULES_GAME_DEFINED
#define BLOBULES_GAME_DEFINED
#include "SDL.h"
#include "SDL_mixer.h"
#include <stdbool.h>
#include "cb_lib/cb_types.h"
#include "ui.h"
#include "game_settings.h"
#include "common.h"

typedef enum {
    NO_SOUND,
    PLAYER_MOVE,
    PLAYER_SLIP,
    PLAYER_PUSH,
    PLAYER_STOP,
    PLAYER_WIN,
    CUBE_MOVE,
    CUBE_TO_CUBE,
    CUBE_STOP,
    FURN_MOVE,
    TARGET_FILLING,
    SOUNDS_COUNT,
} sound_type;

typedef struct {
    Mix_Chunk* data;   
} sound_data;

typedef struct {
    sound_type sound;
    float seconds;
} sound_queue_item;

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
    ENTER,
    ESCAPE,
    INPUT_TYPE_COUNT,
} input_type;

typedef enum {
    LEVEL_UP,
    LEVEL_LEFT,
    LEVEL_DOWN,
    LEVEL_RIGHT,
} level_select_direction;

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
    u32 size;
    string* levels;
} levels_list;

typedef struct {
    bool editor_enabled;
    u32 z_level;
    entity_type active_type;
    cb_ui_state* ui_state;
    cb_window ui_window;
} editor_data;

typedef struct {
    u32 current_level;
    char entities[MAX_WORLD_ENTITIES];        
} world_freezeframe;

typedef struct {
    u32 index;
    world_freezeframe* history;
} world_history;

typedef struct {
    u32 index;
    u32* history;
} level_history;

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
    JUMPING_LEFT,
    ANIMATIONS_COUNT,
} animations;

typedef struct {
    bool looping;
    u32 length;
    u32 index;
    int frame_list[NUM_ANIMATION_FRAMES];
    vec2 frame_positions[NUM_ANIMATION_FRAMES];
} animation_frames_data;

typedef struct {
    // TODO (05 Jul 2020 sam): We may also need to store how many animations there
    // are in total for the given entity.
    bool currently_animating;    
    u32 current_animation_index;
    u32 default_animation_index;
    u32 queue_length;
    animations queue[MAX_QUEUED_ANIMATIONS];
    animation_frames_data animation_data[ENTITY_NUM_ANIMATIONS];
} animation_state;

typedef struct {
    entity_type type;
    // TODO (01 Jul 2020 sam): rename to movement_index
    u32 movement_index;
    u32 animation_index;
    u32 data;
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
    bool confirm_reset;
    int active_option;
    u32 total_options;
    menu_option options[8];
} main_menu_struct;

typedef struct {
    int head_index;
    int tail_index;
    float x1;
    float y1;
    float x2;
    float y2;
    float draw_time;
    bool discovered;
    bool is_left_right;
    bool head_unlocks_tail;
    bool tail_unlocks_head;
} level_connection;

typedef struct {
    bool playable;
    bool unlocked;
    bool discovered; // TODO (12 Aug 2020 sam): now that we have connections, this may be unused
    bool completed;
    // when one bonus is unlock+discovered, we need to unlock and discover
    // all
    bool is_bonus;
    string name;
    string path;
    // TODO (15 Jun 2020 sam): This is unused. Currently we still load data from file.
    string data;
    float xpos;
    float ypos;
    // u32 index;
    int up_index;
    int down_index;
    int left_index;
    int right_index;
    float complete_time;
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
    bool moving;
    bool moving_left_right;
    bool just_left_level;
    level_option* levels;
    level_connection* connections;
    u32 current_level;
    u32 total_levels;
    u32 total_connections;
    level_history history;
    float start_x;
    float start_y;
    float cx;
    float cy;
    float end_x;
    float end_y;
    float move_start;
} level_select_struct;

typedef struct {
    bool currently_moving;
    bool win_scheduled;
    u32 size;
    u32 x_size;
    u32 y_size;
    u32 z_size;
    world_mode active_mode;
    main_menu_struct main_menu;
    level_editor_modes level_mode;
    level_select_struct level_select;
    u32 entities_occupied;
    u32 movements_occupied;
    u32 animations_occupied;
    // boombox* boom;
    mouse_data mouse;
    void* data;
    u32* grid_data;
    entity_data* entities;
    movement_state* movements;
    animation_state* animations;
    vec3i player_position;
    player_input input;
    player_state player;
    // u32 current_level;
    // levels_list levels;
    float seconds;
    float animation_seconds_update;
    float win_schedule_time;
    editor_data editor;
    world_history history;
    sound_data* sounds;
    sound_queue_item* sound_queue;
} world;

int init_main_menu(world* w);
int next_option(world* w);
int previous_option(world* w);
int select_active_option(world* w);

int init_world(world* w, u32 number);
int get_position_index(world* w, int x, int y, int z);
int simulate_world(world* w, float seconds);
int reset_inputs(world* w);
char* as_text (entity_type et);
int change_world_xsize(world* w, int direction, int sign);
int change_world_ysize(world* w, int direction, int sign);
int save_level(world* w);
entity_type get_entity_at(world* w, int index);
u32 get_entity_anim_index(world* w, int index);
void set_renderer(void* r);
int process_input_event(world* w, SDL_Event event);
int handle_input_state(world* w, SDL_GameController* controller);
float get_x_padding(world* w);
float get_y_padding(world* w);

#endif
