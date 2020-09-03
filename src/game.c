#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "cb_lib/cb_types.h"
#include "cb_lib/cb_string.h"
#include "game.h"
#include "game_settings.h"
#include "renderer.h"

#define INPUT_LAG 0.05
#define LEVEL_LISTING "mis_data/levels/data.txt"
#define SAVEFILE "mis_data/savefile.txt"
#define PI 3.14159265

// TODO (18 Apr 2020 sam): This is required to get the inputs working correctly
// with callbacks. See if there is a better way to accomplish this.
world* global_w;
renderer* global_r;
int global_fullscreen;

void set_renderer(void* r) {
    global_r = (renderer*) r;
    set_fullscreen(r, global_fullscreen);
}

float get_x_padding(world* w) {
    // we want the world to be centered based on how big the level is
    float window_width = global_r->size[0];
    float block_width = window_width*BLOCK_WIDTH;
    float cols = w->x_size;
    float occupied = (cols * block_width/2.0) / window_width;
    // this is some fuckall math. basically the padding is a negative value
    // that is added. so it's not the padding from the border, rather the padding
    // from the center outward. also since gl has screen middle as 0.0, and total
    // width as 2.0, things just get cancelled out, and it looks like we are returning
    // the negative occupied as the result... but yeah.
    return -occupied;
}

float get_y_padding(world* w) {
    // we want the world to be centered based on how big the level is
    float window_width = global_r->size[0];
    float window_height = global_r->size[1];
    float block_height = window_width*BLOCK_HEIGHT;
    float rows = w->y_size;
    float occupied = (rows * block_height/2.0) / window_height;
    return -occupied;
}

int get_position_index_sizes(int x_size, int y_size, int z_size, int x, int y, int z) {
    return ( z * x_size * y_size ) + ( y * x_size ) + x;
}

int get_position_index(world* w, int x, int y, int z) {
    return get_position_index_sizes(w->x_size, w->y_size, w->z_size, x, y, z);
}

int get_world_x(world* w) {
    float window_width = global_r->size[0];
    float block_width = window_width*BLOCK_WIDTH;
    double xpos = global_w->mouse.current_x;
    double x_padding = get_x_padding(w);
    return (int) (((xpos - (x_padding*window_width/2.0) - (window_width/2.0)) /
                   (block_width/2.0))
                  );
}

int get_world_y(world* w) {
    float window_width = global_r->size[0];
    float block_height = window_width*BLOCK_HEIGHT;
    double ypos = global_w->mouse.current_y;
    double y_padding = get_y_padding(w);
    return (int) (0.0 - ((ypos+ (y_padding*WINDOW_HEIGHT/2.0) - WINDOW_HEIGHT/2.0) /
                         (block_height/2.0))
                  );
}

int go_to_main_menu(world* w) {
    printf("going to main menu\n");
    w->active_mode = MAIN_MENU;
    return 0;
}
int go_to_level_select(world* w) {
    printf("going to level select\n");
    w->editor.editor_enabled = false;
    w->active_mode = LEVEL_SELECT;
    return 0;
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
    if (e == REFLECTOR)
        return "REFLECTOR";
    return "INVALID";
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
    if (c == 'R')
        return REFLECTOR;
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
    if (et == REFLECTOR)
        return 'R';
    if (et == COLD_TARGET)
        return 'C';
    if (et == DESTROYED_TARGET)
        return 'D';
    return ' ';
}

bool has_movements(entity_type et) {
    return true;
    // TODO (25 Jun 2020 sam): See what all need to be here. Should it
    // just be everything? Is there any benefit in not having all
    return (et==PLAYER || et==CUBE || et==FURNITURE || et==REFLECTOR);
}

bool has_animations(entity_type et) {
    return (et==PLAYER);
}

animations get_anim_from_name(char* a) {
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
    if (strcmp(a, "JUMPING_LEFT") == 0)
        return JUMPING_LEFT;
    return ANIMATIONS_COUNT;
}

int load_player_animations(world* w, u32 anim_index) {
    // TODO (29 Aug 2020 sam): Should this all be done in init_world? Then loaded in later?
    FILE* anim_file = fopen("mis_data/img/player_animations.txt", "r");
    for (int i=0; i<ANIMATIONS_COUNT; i++) {
        char* anim_name[128];
        fscanf(anim_file, "%s\n", &anim_name);
        animations a = get_anim_from_name(anim_name);
        animation_frames_data afd;
        afd.index = 0;
        fscanf(anim_file, "%i\n", &afd.length);
        for (int j=0; j<afd.length; j++)
            fscanf(anim_file, "%i %f %f\n", &afd.frame_list[j], &afd.frame_positions[j].x, &afd.frame_positions[j].y);
        w->animations[anim_index].animation_data[a] = afd;
    }
    fclose(anim_file);
    return 0;
}

int load_entity_animations(world* w, entity_type et, u32 anim_index) {
    if (et==PLAYER)
        load_player_animations(w, anim_index);
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
    // else if (e == GROUND)
    //     entity_index = 1;
    // else if (e == SLIPPERY_GROUND)
    //     entity_index = 2;
    else {
        entity_index = w->entities_occupied;
        int movement_index = 0;
        if (has_movements(e)) {
            movement_index = w->movements_occupied;
            w->movements[movement_index].currently_moving = false;
            w->movements[movement_index].x = (float)x;
            w->movements[movement_index].y = (float)y;
            w->movements[movement_index].z = (float)z;
            w->movements[movement_index].start_x = x;
            w->movements[movement_index].start_y = y;
            w->movements_occupied++;
        }
        int animation_index = 0;
        if (has_animations(e)) {
            animation_index = w->animations_occupied;
            w->animations[animation_index].currently_animating = true;
            w->animations[animation_index].current_animation_index = 0;
            w->animations[animation_index].default_animation_index = 0;
            w->animations[animation_index].queue_length = 0;
            load_entity_animations(w, e, animation_index);
            w->animations_occupied++;
        }
        entity_data ed = {e, movement_index, animation_index, x, y, z, -1.0};
        // if (e == REFLECTOR)
        //     ed.data = 1;
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

int save_freezeframe(world* w) {
    int i, index;
    // TODO (25 May 2020 sam): Since we increment index here, does that mean that
    // we never use the 0th index?
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
    w->history.history[index].current_level = w->level_select.current_level;
    for (i=0; i<w->size; i++)
        w->history.history[index].entities[i] = get_entity_char(get_entity_at(w, i));
    // Prevent save if there was no change in the world
    if (index > 0 && w->history.history[index-1].current_level==w->level_select.current_level) {
        bool same = true;
        for (int i=0; i<w->size; i++) {
            if (w->history.history[index-1].entities[i] !=
                w->history.history[index].entities[i]) {
                same = false;
                break;
            }
        }
        if (same)
            w->history.index--;
    }
    return 0;
}

int save_level_history(world* w) {
    int i, index;
    w->level_select.history.index++;
    if (w->level_select.history.index == HISTORY_STEPS) {
        int num = HISTORY_STEPS/2;
        memcpy(&w->level_select.history.history[0], &w->level_select.history.history[num], num*sizeof(u32));
        w->level_select.history.index = num;
    }
    index = w->level_select.history.index;
    w->level_select.history.history[index] = w->level_select.current_level;
    w->level_select.just_left_level = false;
    return 0;
}

int load_previous_level_from_history(world* w) {
    int i, index;
    if (w->level_select.history.index <= 1)
        return 0;
    w->level_select.history.index--;
    index = w->level_select.history.index;
    w->level_select.current_level = w->level_select.history.history[index];
    level_option current_level = w->level_select.levels[w->level_select.current_level];
    w->level_select.cx = current_level.xpos;
    w->level_select.cy = current_level.ypos;
    return 0;
}

int load_previous_freezeframe(world* w) {
    int i, index;
    if (w->history.index <= 1) return 0;
    w->history.index--;
    index = w->history.index;
    if (w->history.history[index].current_level != w->level_select.current_level)
        return 0;
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
    return 0;
}

int set_none(world* w, int index) {
    w->grid_data[index] = 0;
    return 0;
}
int set_slippery(world* w, int x, int y, int z, float seconds) {
    add_entity(w, SLIPPERY_GROUND, x, y, z);
    return 0;
}
int set_cold_target(world* w, int x, int y, int z, float seconds) {
    add_entity(w, COLD_TARGET, x, y, z);
    return 0;
}

entity_type get_entity_at(world* w, int index) {
    return w->entities[w->grid_data[index]].type;
}
u32 get_entity_movement_index(world* w, int index) {
    return w->entities[w->grid_data[index]].movement_index;
}


int complete_level(world* w, int l_index) {
    level_option lev = w->level_select.levels[l_index];
    if (!lev.completed) {
        w->level_select.levels[l_index].completed = true;
        w->level_select.levels[l_index].complete_time = w->seconds;
    }
    int next;
    int cons[4];
    cons[0] = lev.up_index;
    cons[1] = lev.left_index;
    cons[2] = lev.down_index;
    cons[3] = lev.right_index;
    level_connection con;
    for (int i=0; i<4; i++) {
        int con_index = cons[i];
        if (con_index == -1)
            continue;
        con = w->level_select.connections[con_index];
        if (con.head_index == l_index && con.head_unlocks_tail) {
            if (!w->level_select.levels[con.tail_index].unlocked)
                w->level_select.levels[con.tail_index].unlocked = true;
            if (!con.discovered) {
                w->level_select.connections[con_index].discovered = true;
                w->level_select.connections[con_index].draw_time = w->seconds;
            }
        } else if (con.tail_index == l_index && con.tail_unlocks_head) {
            if  (!w->level_select.levels[con.head_index].unlocked)
                w->level_select.levels[con.head_index].unlocked = true;
            if (!con.discovered) {
                w->level_select.connections[con_index].discovered = true;
                w->level_select.connections[con_index].draw_time = w->seconds;
            }
        }
    }
    save_game_progress(w);
    return 0;
}

int complete_current_level(world* w) {
    int l_index = w->level_select.current_level;
    complete_level(w, l_index);
    return 0;
}

int load_levels_list(level_select_struct* l) {
    // the level file format is as follows. Each line is either a level or a connection
    // between two levels. so first char is either L or C.
    // L path_to_level pos_x pos_y playable is_bonus
    // C level_1_index level_2_index is_left_right head_unlocks_tail tail_unlocks_head
    // if is_left_right, level_1 gets connection as right_index, level_2 as left_index
    // if not, level_1 gets as down_index, level_2 as up_index
    // all the bools are either 0 or 1.
    printf("mallocing... load_levels_list\n");
    string levels_data = read_file(LEVEL_LISTING);
    // n_levels is set to 1 because we check for \nL and the first line would not get counted
    // in that case 
    u32 n_levels = 1;
    u32 n_connections = 0;
    u32 n = 0;
    char c;
    for (int i=0; true; i++) {
        c = levels_data.text[i];
        if (c == '\0')
           break; 
        if (c == '\n') {
            c = levels_data.text[i+1];
            if (c == 'L')
                n_levels++;
            if (c == 'C')
                n_connections++;
        }
    }    
    level_option* levels = (level_option*) malloc(sizeof(level_option)*n_levels);
    level_connection* connections = (level_connection*) malloc(sizeof(level_option)*n_connections);
    string tmp = empty_string();
    bool levels_done = false;
    int index = 0;
    level_option lev;
    lev.unlocked = false;
    lev.completed = false;
    lev.complete_time = 0.0;
    lev.left_index = -1;
    lev.right_index = -1;
    lev.up_index = -1;
    lev.down_index = -1;
    level_connection con;
    con.discovered = false;
    con.draw_time = 1.0;
    for (int i=0; true; i++) {
        c = levels_data.text[i];
        if (c == ' ') {
            if (!levels_done) {
                if (index == 1)
                    lev.path = string_from(tmp.text);
                else if (index == 2)
                    lev.xpos = (float) atof(tmp.text);
                else if (index == 3)
                    lev.ypos = (float) atof(tmp.text);
                else if (index == 4)
                    lev.name = string_from(tmp.text);
            } else {
                if (index == 1)
                    con.head_index = atoi(tmp.text);
                else if (index == 2)
                    con.tail_index = atoi(tmp.text);
                else if (index == 3)
                    con.is_left_right = (tmp.text[0] == '1');
                else if (index == 4)
                    con.head_unlocks_tail = (tmp.text[0]== '1');
                else if (index == 5)
                    con.tail_unlocks_head = (tmp.text[0] == '1');
            }
            index++;
            clear_string(&tmp);
        } else if (c == '\n') {
            if (!levels_done) {
                levels[n] = lev;
                printf("%s %f %f\n", lev.path.text, lev.xpos, lev.ypos);
            }
            else {
                u32 con_index = n-n_levels;
                connections[con_index] = con;
                if (con.is_left_right) {
                    levels[con.head_index].right_index = con_index;
                    levels[con.tail_index].left_index = con_index;
                } else {
                    levels[con.head_index].down_index = con_index;
                    levels[con.tail_index].up_index = con_index;
                }
            }
            n++;
            if (n == n_levels)
                levels_done = true;
            index = 0;
            clear_string(&tmp);
            // if (n>5)
            //     lev.unlocked = false;
        }
        else if (c == '\0')
            break;
        else if (c == '_' && index == 4)
            append_sprintf(&tmp, " ");
        else
            append_sprintf(&tmp, "%c", c);
    }
    l->total_levels = n_levels;
    l->total_connections = n_connections;
    l->levels = levels;
    l->levels[0].unlocked = true;
    l->connections = connections;
    l->current_level = 0;
    u32* level_frames = (u32*) calloc(HISTORY_STEPS, sizeof(u32));
    l->history.index = 0;
    l->history.history = level_frames;
    return 0;
}

int save_game_progress(world* w) {
    FILE* save_file = fopen(SAVEFILE, "w");
    // first line has the settings saved
    fprintf(save_file, "beta v0.0\n");
    fprintf(save_file, "%i %i %i %i\n", w->level_select.current_level, global_r->fullscreen, w->level_select.total_levels, w->level_select.total_connections);
    for (int i=0; i<w->level_select.total_levels; i++) {
        level_option lev = w->level_select.levels[i];
        fprintf(save_file, "L %i %i %i\n", lev.unlocked, lev.discovered, lev.completed);
    }
    for (int i=0; i<w->level_select.total_connections; i++) {
        level_connection con = w->level_select.connections[i];
        fprintf(save_file, "C %i\n", con.discovered);
    }
    fclose(save_file);
    return 0;
}

int load_game_progress(world* w) {
    FILE* save_file = fopen(SAVEFILE, "r");
    int total_levels, total_connections;
    int major_version, minor_version;
    // getting file version
    fscanf(save_file, "beta v%i.%i\n", &major_version, &minor_version);
    // TODO (12 Aug 2020 sam): Check version here.
    // getting saved settings
    fscanf(save_file, "%i %i %i %i\n", &w->level_select.current_level, &global_fullscreen, &total_levels, &total_connections);
    printf("total levels: %i\t total connections: %i\n", total_levels, total_connections);
    // TODO (12 Aug 2020 sam): Make sure there aren't more levels/connections than data loaded.
    for (int i=0; i<total_levels; i++) {
        int unlocked, discovered, completed;
        fscanf(save_file, "L %i %i %i\n", &unlocked, &discovered, &completed);
        w->level_select.levels[i].unlocked = unlocked;
        w->level_select.levels[i].discovered = discovered;
        w->level_select.levels[i].completed = completed;
    }
    for (int i=0; i<total_connections; i++) {
        int discovered;
        fscanf(save_file, "C %i\n", &discovered);
        w->level_select.connections[i].discovered = discovered;
    }
    fclose(save_file);
    level_option lev = w->level_select.levels[w->level_select.current_level];
    w->level_select.cx = lev.xpos;
    w->level_select.cy = lev.ypos;
    return 0;
}

int reset_game_progress(world* w) {
    for (int i=0; i<w->level_select.total_levels; i++) {
        w->level_select.levels[i].unlocked = false;
        w->level_select.levels[i].discovered = false;
        w->level_select.levels[i].completed = false;
    }
    for (int i=0; i<w->level_select.total_connections; i++) {
        w->level_select.connections[i].discovered = false;
    }
    w->level_select.levels[0].unlocked = true;
    w->level_select.levels[0].discovered = true;
    w->level_select.current_level = 0;
    w->level_select.cx = w->level_select.levels[0].xpos;
    w->level_select.cy = w->level_select.levels[0].ypos;
    save_game_progress(w);
    return 0;
}

// int save_levels_list(world* w) {
//     printf("saving level_select list\n");
//     FILE* level_listing_file = fopen(LEVEL_LISTING, "w");
//     for (int n=0; n<w->level_select.total_levels; n++) {
//         level_option lev = w->level_select.levels[n];
//         fprintf(level_listing_file, "%s %f %f %i %i %i %i\n", lev.path.text, lev.xpos, lev.ypos, lev.up_index, lev.down_index, lev.left_index, lev.right_index);
//     }
//     return 0;
// }

int init_entities(world* w) {
    // TODO (25 Jun 2020 sam): This had all been initialised to nonzero values. I'm not sure
    // that is actually required. Except for maybe the set_none?
    // Initialise all the entities that will not have additional data
    // or which can have only one instance
    w->entities_occupied = 1;
    // This is initialised at 1 so index 0 has all the unanimated entities
    w->movements_occupied = 1;
    w->movements[0].currently_moving = false;
    w->animations_occupied = 1;
    w->animations[0].currently_animating = false;
    w->animations[0].queue_length = 0;
    entity_data ed;
    ed.movement_index = 0;
    ed.type = NONE;
    ed.x = 0;
    ed.y = 0;
    ed.z = 0;
    ed.removal_time = -1.0;
    w->entities[0] = ed;
    return 0;
}

int clear_world_history(world* w) {
    w->history.index = 0;
    return 0;
}

int load_level(world* w) {
    w->player = ALIVE;
    w->currently_moving = false;
    printf("initting entities\n");
    init_entities(w);
    int x, y, z;
    char c = ' ';
    printf("loading level data\n");
    char* level_name = w->level_select.levels[w->level_select.current_level].path.text;
    FILE* level_file = fopen(level_name, "r");
    printf("opening level file %s\n", level_name); 
    fscanf(level_file, "%i %i %i\n", &x, &y, &z);
    w->size = x * y * z;
    if (w->size > MAX_WORLD_ENTITIES)
        printf("not enough room malloced for all entities... Might crash.");
    w->x_size = x;
    w->y_size = y;
    w->z_size = z;
    printf("adding entities\n");
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
    // clear_world_history(w);
    save_freezeframe(w);
    return 0;
}

int save_level(world* w) {
    int x, y, z;
    int index;
    char c = ' ';
    char* level_name = w->level_select.levels[w->level_select.current_level].path.text;
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
    w->level_select.current_level++;
    if (w->level_select.current_level == w->level_select.total_levels)
        w->level_select.current_level = 0;
    load_level(w);
    return 0;
}

int load_previous_level(world* w) {
    w->level_select.current_level--;
    if (w->level_select.current_level == -1)
        w->level_select.current_level = w->level_select.total_levels-1;
    load_level(w);
    return 0;
}

int add_sound_to_queue(world* w, sound_type sound, float seconds) {
    for (int i=0; i<SOUND_QUEUE_LENGTH; i++) {
        if (w->sound_queue[i].sound == NO_SOUND) {
            w->sound_queue[i].sound = sound;
            w->sound_queue[i].seconds = seconds;
            break;
        }
    }
    return 0;
}

int load_sounds(world* w) {
    w->sounds = (sound_data*) malloc(SOUNDS_COUNT * sizeof(sound_data));
    w->sounds[PLAYER_MOVE].data = Mix_LoadWAV("mis_data/sounds/moving.wav");
    w->sounds[PLAYER_SLIP].data = Mix_LoadWAV("mis_data/sounds/slipping.wav");
    w->sounds[PLAYER_PUSH].data = Mix_LoadWAV("mis_data/sounds/pushing.wav");
    w->sounds[PLAYER_STOP].data = Mix_LoadWAV("mis_data/sounds/stopping.wav");
    w->sounds[PLAYER_WIN].data = Mix_LoadWAV("mis_data/sounds/dive.wav");
    w->sounds[CUBE_MOVE].data = Mix_LoadWAV("mis_data/sounds/ice_slide.wav");
    w->sounds[CUBE_TO_CUBE].data = Mix_LoadWAV("mis_data/sounds/ice_to_ice.wav");
    w->sounds[CUBE_STOP].data = Mix_LoadWAV("mis_data/sounds/thud.wav");
    w->sounds[FURN_MOVE].data = Mix_LoadWAV("mis_data/sounds/box_slide.wav");
    w->sounds[TARGET_FILLING].data = Mix_LoadWAV("mis_data/sounds/filling.wav");
    w->sound_queue = (sound_queue_item*) malloc(SOUND_QUEUE_LENGTH * sizeof(sound_queue_item));
    for (int i=0; i<SOUND_QUEUE_LENGTH; i++)
        w->sound_queue[i].sound = NO_SOUND;
    return 0;
}

int init_input_state(world* w) {
    w->input_state = (input_state_type*) malloc(sizeof(input_state_type) * INPUT_TYPE_COUNT);
    for (int i=0; i<INPUT_TYPE_COUNT; i++) {
        w->input_state[i].down = false;
    }
    for (int i=0; i<INPUT_QUEUE_LENGTH; i++)
        w->input_queue.inputs[i] = NO_INPUT;
    w->input_queue.occupied = 0;
    return 0;
}

int init_world(world* w, u32 number) {
    world_freezeframe* frames = (world_freezeframe*) malloc(HISTORY_STEPS * sizeof(world_freezeframe));
    world_history history = {0, frames};
    level_select_struct level_select;
    load_levels_list(&level_select);
    level_select.moving = false;
    level_select.cx = level_select.levels[0].xpos;
    level_select.cy = level_select.levels[0].ypos;
    mouse_data mouse;
    mouse.l_pressed = false;
    mouse.r_pressed = false;
    world tmp;
    // TODO (12 Jun 2020 sam): See whether there is a cleaner way to set this all to 0;
    memset(&tmp, 0, sizeof(world));
    tmp.active_mode = MAIN_MENU;
    tmp.level_select = level_select;
    init_main_menu(&tmp);
    tmp.player = ALIVE;
    tmp.level_mode = NEUTRAL;
    tmp.editor.editor_enabled = false;
    tmp.currently_moving = false;
    tmp.win_scheduled = false;
    tmp.mouse = mouse;
    tmp.editor.z_level = 0;
    tmp.editor.active_type = GROUND;
    tmp.history = history;
    *w = tmp;
    u32 grid_data_size = sizeof(u32) * MAX_WORLD_ENTITIES;
    u32 entity_data_size = sizeof(entity_data) * MAX_WORLD_ENTITIES;
    u32 movement_data_size = sizeof(movement_state) * MAX_WORLD_ENTITIES;
    u32 animation_data_size = sizeof(animation_state) * MAX_WORLD_ENTITIES;
    w->data = (void*) calloc(grid_data_size+entity_data_size+movement_data_size+animation_data_size, sizeof(char));
    w->grid_data = w->data;
    w->entities = (char*) w->data+grid_data_size;
    w->movements = (char*)w->data+(grid_data_size+entity_data_size);
    w->animations = (char*)w->data+(grid_data_size+entity_data_size+movement_data_size);
    w->animation_seconds_update = 0.0;
    init_input_state(w);
    load_sounds(w);
    printf("loading game progress\n");
    load_game_progress(w);
    printf("saving lelve history\n");
    save_level_history(w);
    printf("loading level\n");
    load_level(w);
    printf("initted world\n");
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

int can_support_reflector(entity_type et) {
    if (et == GROUND ||
        et == SLIPPERY_GROUND)
        return 1;
    return 0;
}

int can_stop_cube_slide(entity_type et) {
    if (et == CUBE ||
        et == FURNITURE ||
        et == REFLECTOR ||
        et == WALL)
        return true;
    return false;
}

int set_entity_position(world* w, u32 index, int x, int y, int z) {
    w->entities[index].x = x;
    w->entities[index].y = y;
    w->entities[index].z = z;
    return 0;
}

int schedule_entity_removal(world*w, u32 index, int depth) {
    w->entities[index].removal_time = w->seconds + (3+depth)*ANIMATION_SINGLE_STEP_TIME;
    return 0;
}

int add_interpolation(world* w, u32 index, int x, int y, int z, int dx, int dy, int depth) {
    w->currently_moving = true;
    if (w->movements[w->entities[index].movement_index].currently_moving) {
        w->movements[w->entities[index].movement_index].duration += ANIMATION_SINGLE_STEP_TIME; // *pow(0.8, depth);
        w->movements[w->entities[index].movement_index].dx += dx;
        w->movements[w->entities[index].movement_index].dy += dy;
    } else {
        movement_state move_animation = {
            true,
            w->seconds+(depth*ANIMATION_SINGLE_STEP_TIME),
            ANIMATION_SINGLE_STEP_TIME,
            x, y, z, x, y, dx, dy};
        w->movements[w->entities[index].movement_index] = move_animation;
    }
    return 0;
}

int maybe_reflect_cube(world* w, int ogx, int ogy, int x, int y, int z, int dx, int dy, int dz, int depth) {
    // this is slightly different from the other functions because we need to be able to
    // chain the reflections. ogx and ogy are the original positions of the cube. x and
    // y are the positions of the reflector (not the cube).
    int r_index = get_position_index(w, x, y, z);
    int reflector_index = w->grid_data[r_index];
    int index = get_position_index(w, ogx, ogy, z);
    int tx, ty, ndx, ndy;
    if (w->entities[reflector_index].data == 0) {
        ndx = dy;
        ndy = dx;
        tx = x+ndx;
        ty = y+ndy;
    } else {
        ndx = -dy;
        ndy = -dx;
        tx = x+ndx;
        ty = y+ndy;
    }
    int target_pos_index = get_position_index(w, tx, ty, z);
    int et = get_entity_at(w, target_pos_index);
    if (can_stop_cube_slide(et) && et!=REFLECTOR)
        return 1;
    if (et == REFLECTOR) {
        maybe_reflect_cube(w, ogx, ogy, tx, ty, z, ndx, ndy, dz, depth+1);
        // does this need to return?
        return 1;
    }
    int cube_index = w->grid_data[index];
    // TODO (16 May 2020 sam): Check if there is ground available here or whatevs
    w->grid_data[target_pos_index] = cube_index;
    set_none(w, index);
    maybe_move_cube(w, tx, ty, z, ndx, ndy, dz, depth+1);
    return 0;
}

int maybe_move_cube(world* w, int x, int y, int z, int dx, int dy, int dz, int depth) {
    int on_index = get_position_index(w, x, y, z-1);
    if (get_entity_at(w, on_index) != HOT_TARGET)
        set_slippery(w, x, y, z-1, w->seconds+ANIMATION_SINGLE_STEP_TIME*depth);
    int index = get_position_index(w, x, y, z);
    int cube_index = w->grid_data[index];
    if ((x+dx < 0 || x+dx > w->x_size-1) ||
        (y+dy < 0 || y+dy > w->y_size-1) ||
        (z+dz < 0 || z+dz > w->z_size-1)) {
        // remove cube
        // TODO (07 May 2020 sam): Figure out how to handle animations here?
        add_interpolation(w, cube_index, x, y, z, dx, dy, depth);
        set_entity_position(w, cube_index, x+dx, y+dy, z+dz);
        schedule_entity_removal(w, cube_index, depth);
        set_none(w, index);
        return -1;
    }
    // see what's already in desired place
    int target_pos_index = get_position_index(w, x+dx, y+dy, z+dz);
    int target_on_index = get_position_index(w, x+dx, y+dy, z+dz-1);
    if (get_entity_at(w, target_pos_index) != NONE) {
        if (can_stop_cube_slide(get_entity_at(w, target_pos_index))) {
            sound_type st = CUBE_STOP;
            if (get_entity_at(w, target_pos_index) == CUBE) {
                maybe_move_cube(w, x+dx, y+dy, z+dz, dx, dy, dz, depth);
                st = CUBE_TO_CUBE;
            }
            if (get_entity_at(w, target_pos_index) == REFLECTOR)
                maybe_reflect_cube(w, x, y, x+dx, y+dy, z, dx, dy, dz, depth);
            if (get_entity_at(w, target_pos_index) == FURNITURE)
                maybe_move_furniture(w, x+dx, y+dy, z+dz, dx, dy, dz, depth);
            // check if win.
            if (get_entity_at(w, on_index) == HOT_TARGET) {
                schedule_entity_removal(w, cube_index, depth);
                schedule_entity_removal(w, w->grid_data[on_index], depth);
                set_entity_position(w, cube_index, x, y, z);
                set_none(w, index);
                set_cold_target(w, x, y, z-1, w->seconds+ANIMATION_SINGLE_STEP_TIME*depth);
                add_sound_to_queue(w, TARGET_FILLING, w->seconds + ANIMATION_SINGLE_STEP_TIME*(depth+1));
            }
            add_sound_to_queue(w, st, w->seconds + ANIMATION_SINGLE_STEP_TIME*depth);
            return 1;
        }
    }
    if (!can_support_cube(get_entity_at(w, target_on_index))) {
        // remove cube
        // TODO (07 May 2020 sam): Figure out how to handle animations here?
        schedule_entity_removal(w, cube_index, depth);
        add_interpolation(w, cube_index, x, y, z, dx, dy, depth);
        set_entity_position(w, cube_index, x+dx, y+dy, z+dz);
        set_none(w, index);
        return -2;
    }
    w->grid_data[target_pos_index] = cube_index;
    set_none(w, index);
    add_interpolation(w, cube_index, x, y, z, dx, dy, depth);
    maybe_move_cube(w, x+dx, y+dy, z+dz, dx, dy, dz, depth+1);
    if (depth == 0)
        add_sound_to_queue(w, CUBE_MOVE, w->seconds + ANIMATION_SINGLE_STEP_TIME*depth);
    return 0;
}

int maybe_move_furniture(world* w, int x, int y, int z, int dx, int dy, int dz, int depth) {
    int on_index = get_position_index(w, x, y, z-1);
    int index = get_position_index(w, x, y, z);
    int furniture_index = w->grid_data[index];
    if ((x+dx < 0 || x+dx > w->x_size-1) ||
        (y+dy < 0 || y+dy > w->y_size-1) ||
        (z+dz < 0 || z+dz > w->z_size-1)) {
        add_interpolation(w, furniture_index, x, y, z, dx, dy, depth);
        set_entity_position(w, furniture_index, x+dx, y+dy, z+dz);
        schedule_entity_removal(w, furniture_index, depth);
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
        if (get_entity_at(w, target_pos_index) == CUBE)
            return 1;
        // what if it's player?
    }
    int target_on_index = get_position_index(w, x+dx, y+dy, z+dz-1);
    if (!can_support_furniture(get_entity_at(w, target_on_index))) {
        add_interpolation(w, furniture_index, x, y, z, dx, dy, depth);
        set_entity_position(w, furniture_index, x+dx, y+dy, z+dz);
        schedule_entity_removal(w, furniture_index, depth);
        set_none(w, index);
        return -2;
    }
    w->grid_data[target_pos_index] = furniture_index;
    set_none(w, index);
    add_interpolation(w, furniture_index, x, y, z, dx, dy, depth);
    if (depth == 0) {
        add_sound_to_queue(w, FURN_MOVE, w->seconds + ANIMATION_SINGLE_STEP_TIME*depth);
    }
    if (get_entity_at(w, target_on_index) == SLIPPERY_GROUND)
        maybe_move_furniture(w, x+dx, y+dy, z+dz, dx, dy, dz, depth+1);
    return 0;
}

int maybe_move_reflector(world* w, int x, int y, int z, int dx, int dy, int dz, int depth) {
    int on_index = get_position_index(w, x, y, z-1);
    int index = get_position_index(w, x, y, z);
    int reflector_index = w->grid_data[index];
    if ((x+dx < 0 || x+dx > w->x_size-1) ||
        (y+dy < 0 || y+dy > w->y_size-1) ||
        (z+dz < 0 || z+dz > w->z_size-1)) {
        add_interpolation(w, reflector_index, x, y, z, dx, dy, depth);
        schedule_entity_removal(w, reflector_index, depth);
        set_entity_position(w, reflector_index, x+dx, y+dy, z+dz);
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
        if (get_entity_at(w, target_pos_index) == REFLECTOR)
            return 1;
        if (get_entity_at(w, target_pos_index) == CUBE) {
            return 1;
        }
        // what if it's player?
    }
    int target_on_index = get_position_index(w, x+dx, y+dy, z+dz-1);
    if (!can_support_reflector(get_entity_at(w, target_on_index))) {
        add_interpolation(w, reflector_index, x, y, z, dx, dy, depth);
        set_entity_position(w, reflector_index, x+dx, y+dy, z+dz);
        schedule_entity_removal(w, reflector_index, depth);
        set_none(w, index);
        return -2;
    }
    w->grid_data[target_pos_index] = reflector_index;
    set_none(w, index);
    add_interpolation(w, reflector_index, x, y, z, dx, dy, depth);
    if (get_entity_at(w, target_on_index) == SLIPPERY_GROUND)
        maybe_move_reflector(w, x+dx, y+dy, z+dz, dx, dy, dz, depth+1);
    return 0;
}

bool can_push_player_back(entity_type et) {
    if (et == CUBE ||
        et == FURNITURE ||
        et == REFLECTOR ||
        et == WALL)
        return true;
    return false;
}

int set_player_animation(world* w, u32 index, animations a) {
    // reset current playing animation to frame 0
    u32 anim_index = w->entities[index].animation_index;
    animation_state as = w->animations[anim_index];
    as.animation_data[as.current_animation_index].index = 0;
    as.current_animation_index = a;
    w->animations[anim_index] = as;
    return 0;
}

int queue_player_animation(world* w , u32 index, animations a) {
    u32 anim_index = w->entities[index].animation_index;
    animation_state as = w->animations[anim_index];
    u32 queue_index = as.queue_length;;
    as.queue_length++;
    as.queue[queue_index] = a;
    w->animations[anim_index] = as;
    return 0;
}

int schedule_player_win(world* w, int depth) {
    w->win_schedule_time = w->seconds + (depth+1)*ANIMATION_SINGLE_STEP_TIME;
    w->win_scheduled = true;
    return 0;
}

int maybe_move_player(world* w, int dx, int dy, int dz, bool force, int depth) {
    vec3i pos = w->player_position;
    int position_index = get_position_index(w, pos.x, pos.y, pos.z);
    int ground_index = get_position_index(w, pos.x, pos.y, pos.z-1);
    int target_index = get_position_index(w, pos.x+dx, pos.y+dy, pos.z+dz);
    int target_ground_index = get_position_index(w, pos.x+dx, pos.y+dy, pos.z+dz-1);
    int player_index = w->grid_data[position_index];
    bool should_move_player = true;
    bool should_call_maybe_move = false;
    bool should_schedule_removal = false;
    int new_dx, new_dy, new_dz;
    animations anim_to_queue = STATIC;
    sound_type sound_to_queue = NO_SOUND;
    // check out of bounds;
    if ((pos.x+dx < 0 || pos.x+dx > w->x_size-1) ||
        (pos.y+dy < 0 || pos.y+dy > w->y_size-1) ||
        (pos.z+dz < 0 || pos.z+dz > w->z_size-1)) {
        if (force) {
            should_schedule_removal = true;
        }
        return -1;
    }
    // see what's already in desired place
    if (get_entity_at(w, target_index) != NONE) {
        should_move_player = false;
        if (force) {
            anim_to_queue = STOPPING_HARD_LEFT;
            sound_to_queue = PLAYER_STOP;
        } else {
            anim_to_queue = PUSHING_LEFT;
            sound_to_queue = PLAYER_MOVE;
        }
        if (can_push_player_back(get_entity_at(w, target_index)) && !force) {
            if (get_entity_at(w, ground_index) == SLIPPERY_GROUND) {
                should_call_maybe_move = true;
                new_dx = -dx;
                new_dy = -dy;
                new_dz = -dz;
            }
        }
        if (get_entity_at(w, target_index) == CUBE)
            maybe_move_cube(w, pos.x+dx, pos.y+dy, pos.z+dz, dx, dy, dz, depth);
        if (get_entity_at(w, target_index) == FURNITURE)
            maybe_move_furniture(w, pos.x+dx, pos.y+dy, pos.z+dz, dx, dy, dz, depth);
        if (get_entity_at(w, target_index) == REFLECTOR)
            maybe_move_reflector(w, pos.x+dx, pos.y+dy, pos.z+dz, dx, dy, dz, depth);
    }
    // check if can stand
    if (!can_support_player(get_entity_at(w, target_ground_index))) {
        if (force)
            should_schedule_removal = true;
        else
            should_move_player = false;
    }
    if (get_entity_at(w, ground_index) == HOT_TARGET) {
        // for bugs where player input was being accepted before entity could be deleted
        should_move_player = false;
    } 
    if (get_entity_at(w, ground_index) == SLIPPERY_GROUND && !force && !should_call_maybe_move) {
        should_move_player = false;
        anim_to_queue = SLIPPING;
        sound_to_queue = PLAYER_SLIP;
    }
    if (get_entity_at(w, target_ground_index) == SLIPPERY_GROUND && should_move_player) {
        should_call_maybe_move = true;
        new_dx = dx;
        new_dy = dy;
        new_dz = dz;
    }
    if (get_entity_at(w, target_ground_index) == COLD_TARGET) {
        schedule_player_win(w, depth+1);
        anim_to_queue = JUMPING_LEFT;
    }
    if (anim_to_queue == STATIC && force && should_move_player) {
        anim_to_queue = SLIPPING;
        if (depth==0)
            sound_to_queue = PLAYER_SLIP;
    }
    if (anim_to_queue == STATIC && should_move_player) {
        sound_to_queue = PLAYER_MOVE;
        if (dx > 0)
            anim_to_queue = MOVING_RIGHT;
        else
            anim_to_queue = MOVING_LEFT;
    }
    queue_player_animation(w, player_index, anim_to_queue);
    if (sound_to_queue != NO_SOUND)
        add_sound_to_queue(w, sound_to_queue, w->seconds + depth*ANIMATION_SINGLE_STEP_TIME);
    if (should_schedule_removal)
        schedule_entity_removal(w, player_index, depth);
    if (should_move_player) {
        w->player_position.x += dx;
        w->player_position.y += dy;
        w->player_position.z += dz;
        w->grid_data[target_index] = player_index;
        set_entity_position(w, player_index, pos.x+dx, pos.y+dy, pos.z+dz);
        set_none(w, position_index);
        add_interpolation(w, player_index, pos.x, pos.y, pos.z, dx, dy, depth);
    }
    if (should_call_maybe_move)
        // We only have to call maybe move if there is force.
        maybe_move_player(w, new_dx, new_dy, new_dz, true, depth+1);
    return 0;
}

int copy_grid_data_to_entities(world* w) {
    // Along with trying to keep everything in sync, we explicitly update the position
    // values of all the entities every frame. So we only have to ensure correct position
    // for entities that are no longer in grid_data.
    for (int z=0; z<w->z_size; z++) {
        for (int y=0; y<w->y_size; y++) {
            for (int x=0; x<w->x_size; x++) {
                int index = get_position_index(w, x, y, z);
                w->entities[w->grid_data[index]].x = x;
                w->entities[w->grid_data[index]].y = y;
                w->entities[w->grid_data[index]].z = z;
            }
        }
    }
    return 0;
}

int remove_scheduled_entities(world* w) {
    for (int i=0; i<MAX_WORLD_ENTITIES; i++) {
        if (w->entities[i].removal_time > 0.0 && w->seconds > w->entities[i].removal_time) {
            w->entities[i].type = NONE;
            w->entities[i].removal_time = -1.0;
        }
    }
    return 0;
}

int set_input(world* w, input_type it) {
    if (w->currently_moving) {
        // we want to cancel the animations that are playing
        for (int i=0; i<w->movements_occupied; i++) {
            if (!w->movements[i].currently_moving)
                continue;
            float elapsed = 1.0;
            w->movements[i].currently_moving = false;
            w->movements[i].x = w->movements[i].start_x + w->movements[i].dx;
            w->movements[i].y = w->movements[i].start_y + w->movements[i].dy;
        }
    }
    if (w->active_mode == IN_GAME) {
        if (w->win_scheduled)
            return 0;
        if (it == MOVE_UP)
            maybe_move_player(w, 0, 1, 0, false, 0);
        if (it == MOVE_DOWN)
            maybe_move_player(w, 0, -1, 0, false, 0);
        if (it == MOVE_RIGHT)
            maybe_move_player(w, 1, 0, 0, false, 0);
        if (it == MOVE_LEFT)
            maybe_move_player(w, -1, 0, 0, false, 0);
        if (it == RESTART_LEVEL)
            load_level(w);
        if (it == UNDO_MOVE)
            load_previous_freezeframe(w);
        if (it == ESCAPE) {
            go_to_level_select(w);
            w->level_select.just_left_level = true;
        }
        if (it == MOVE_UP ||
            it == MOVE_DOWN ||
            it == MOVE_LEFT ||
            it == MOVE_RIGHT ||
            it == RESTART_LEVEL)
            save_freezeframe(w);
    } else if (w->active_mode == LEVEL_SELECT) {
        if (w->level_select.moving)
            return 0;
        if (it == MOVE_UP)
            set_next_level(w, LEVEL_UP);
        if (it == MOVE_DOWN)
            set_next_level(w, LEVEL_DOWN);
        if (it == MOVE_LEFT)
            set_next_level(w, LEVEL_LEFT);
        if (it == MOVE_RIGHT)
            set_next_level(w, LEVEL_RIGHT);
        if (it == UNDO_MOVE) {
            if (w->level_select.just_left_level)
                w->active_mode = IN_GAME;
            else
                load_previous_level_from_history(w);
        }
        if (it == ENTER)
            enter_active_level(w);
        if (it == ESCAPE)
            go_to_main_menu(w);
    } else if (w->active_mode == MAIN_MENU) {
        if (it == MOVE_UP)
            previous_option(w);
        if (it == MOVE_DOWN)
            next_option(w);
        if (it == ENTER)
            select_active_option(w);
        if (DEBUG_BUILD) {
            if (it  == ESCAPE)
                w->active_mode = EXIT;
        }
    }
    return 0;
}

int unlock_all_levels(world* w) {
    printf("unlocking levels...\n");
    for (int i=0; i<w->level_select.total_levels; i++) {
        w->level_select.levels[i].unlocked = true;
        w->level_select.levels[i].completed = true;
    }
    for (int i=0; i<w->level_select.total_connections; i++)
        w->level_select.connections[i].discovered = true;
    return 0;
}

int init_main_menu(world* w) {
    menu_option new_game = {string_from("Play")};
    menu_option fullscreen = {string_from("Toggle Fullscreen")};
    menu_option reset_progress = {string_from("Reset Progess")};
    menu_option exit = {string_from("Save and Quit")};
    w->main_menu.total_options = 4;
    w->main_menu.active_option = 0;
    w->main_menu.options[0] = new_game;
    w->main_menu.options[1] = fullscreen;
    w->main_menu.options[2] = reset_progress;
    w->main_menu.options[3] = exit;
    return 0;
}

int next_option(world* w) {
    w->main_menu.active_option++;
    if (w->main_menu.active_option >= w->main_menu.total_options)
        w->main_menu.active_option = 0;
    return 0;
}

int previous_option(world* w) {
    w->main_menu.active_option--;
    if (w->main_menu.active_option < 0)
        w->main_menu.active_option = w->main_menu.total_options-1;
    return 0;
}

int select_active_option(world* w) {
    // TODO (14 Jun 2020 sam): Figure out better way to link option with action?
    if (w->main_menu.active_option == 0)
        w->active_mode = LEVEL_SELECT;
    if (w->main_menu.active_option == 1)
        toggle_fullscreen(global_r, w);
    if (w->main_menu.active_option == 2)
        reset_game_progress(w);
    if (w->main_menu.active_option == 3) {
        save_game_progress(w);
        w->active_mode = EXIT;
    }
    return 0;
}

int go_to_next_level_mode(world* w) {
    w->level_mode++;
    if (w->level_mode == LEVEL_EDITOR_MODES_COUNT)
        w->level_mode = NEUTRAL;
    return 0;
}

int add_level_interpolation(world* w, int current, int next) {
    return 0;
}

int get_other_level(world* w, int index, int current) {
    if (index == -1)
        return -1;
    level_connection con = w->level_select.connections[index];
    if (con.head_index == current)
        return con.tail_index;
    return con.head_index;
}

int set_next_level(world* w, level_select_direction dir) {
    int next;
    level_connection con;
    int current_index = w->level_select.current_level;
    level_option current_level = w->level_select.levels[current_index];
    if (dir == LEVEL_UP) {
        next = get_other_level(w, current_level.up_index, current_index);
        con = w->level_select.connections[current_level.up_index];
    }
    if (dir == LEVEL_DOWN) {
        next = get_other_level(w, current_level.down_index, current_index);
        con = w->level_select.connections[current_level.down_index];
    }
    if (dir == LEVEL_LEFT) {
        next = get_other_level(w, current_level.left_index, current_index);
        con = w->level_select.connections[current_level.left_index];
    }
    if (dir == LEVEL_RIGHT) {
        next = get_other_level(w, current_level.right_index, current_index);
        con = w->level_select.connections[current_level.right_index];
    }
    if (next != -1) {
        level_option next_level = w->level_select.levels[next];
        if (next_level.unlocked && con.discovered) {
            if (dir == LEVEL_LEFT || dir == LEVEL_RIGHT)
                w->level_select.moving_left_right = true;
            else
                w->level_select.moving_left_right = false;
            w->level_select.moving = true;
            w->level_select.start_x = current_level.xpos;
            w->level_select.start_y = current_level.ypos;
            w->level_select.end_x = next_level.xpos;
            w->level_select.end_y = next_level.ypos;
            w->level_select.move_start = w->seconds;
            w->level_select.current_level = next;
            save_level_history(w);
        }
    }
    return 0;
}

int enter_active_level(world* w) {
    load_level(w);
    w->active_mode = IN_GAME;
    return 0;
}

void add_entity_at_mouse(world* w) {
    int x = get_world_x(w);
    int y = get_world_y(w);
    if (x < 0 || x >= w->x_size || y < 0 || y >= w->y_size)
        return;
    // TODO (19 Apr 2020 sam): Add check here to see whether the type is
    // on "correct z level"
    // TODO (03 Jul 2020 sam): Check if there is already an entity here
    // and remove in that case.
    u32 index = get_position_index(w, x, y, w->editor.z_level);
    entity_type et = get_entity_at(w, index);
    if (et != NONE)
        w->entities[w->grid_data[index]].type = NONE;
    add_entity(w, w->editor.active_type, x, y, w->editor.z_level);
    // Due to the way entities work now, we need to re initialise the
    // entities when we remove something. This way of doing it is obviously
    // not performant, but since this function is only called in the editor,
    // its fine.
    // change_world_xsize_right(w, 1);
    // change_world_xsize_right(w, -1);
}

void remove_entity_at_mouse(world* w) {
    int x = get_world_x(w);
    int y = get_world_y(w);
    if (x < 0 || x >= w->x_size || y < 0 || y >= w->y_size)
        return;
    u32 index = get_position_index(w, x, y, w->editor.z_level);
    entity_type et = get_entity_at(w, index);
    if (et != NONE) {
        w->editor.active_type = et;
        // TODO (03 Jul 2020 sam): This doesn't immediately get removed
        // and for some reason, the scheduling removal just acts really
        // wonky
        w->entities[w->grid_data[index]].type = NONE;
        // Due to the way entities work now, we need to re initialise the
        // entities when we remove something. This way of doing it is obviously
        // not performant, but since this function is only called in the editor,
        // its fine.
        // change_world_xsize_right(w, 1);
        // change_world_xsize_right(w, -1);
    }
    // save_freezeframe(w);
}

int process_mouse_motion(world* w, SDL_MouseMotionEvent event) {
    w->mouse.current_x = 1.0*event.x;
    w->mouse.current_y = 1.0*event.y;
    return 0;
}

int process_mouse_button(world* w, SDL_MouseButtonEvent event, u32 action) {
    int button = event.button;
    if (button == SDL_BUTTON_LEFT && action == SDL_MOUSEBUTTONDOWN) {
        w->mouse.l_pressed = true;
        w->mouse.l_down_x = w->mouse.current_x;
        w->mouse.l_down_y = w->mouse.current_y;
    }
    if (button == SDL_BUTTON_LEFT && action == SDL_MOUSEBUTTONUP) {
        w->mouse.l_pressed = false;
        w->mouse.l_released = true;
    }
    if (button == SDL_BUTTON_RIGHT && action == SDL_MOUSEBUTTONDOWN) {
        w->mouse.r_pressed = true;
        w->mouse.r_down_x = w->mouse.current_x;
        w->mouse.r_down_y = w->mouse.current_y;
    }
    if (button == SDL_BUTTON_RIGHT && action == SDL_MOUSEBUTTONUP) {
        w->mouse.r_pressed = false;
        w->mouse.r_released = true;
    }
    return 0;
}

int run_editor_functions(world* w) {
    if (w->mouse.l_pressed)
        add_entity_at_mouse(w);
    else if (w->mouse.r_pressed)
        remove_entity_at_mouse(w);
    return 0;
}

float get_linear_progress(float time_elapsed) {
    return time_elapsed;
}

float get_ease_in_out_progress(float time_elapsed) {
    // formulae from https://github.com/nicolausYes/easing-functions/blob/master/src/easing.cpp
    // we are using the easeInOutQuad
    // return t < 0.5 ? 2 * t * t : t * (4 - 2 * t) - 1;
    // return time_elapsed;
    if (time_elapsed < 0.5)
        return 2 * time_elapsed * time_elapsed;
    return time_elapsed * (4 - 2*time_elapsed)-1;
}

float get_sigmoid_progress(float time_elapsed) {
    // 1/(1+pow(2.718,5-x*10))
    return 1.0 / (1.0 + pow(2.718, 5-(10*time_elapsed)));
}

float get_anti_sigmoid_progress(float time_elapsed) {
    // basically a function that slows growth when sigmoid is growing more.
    if (time_elapsed < 0.5)
        return sin(time_elapsed*PI)/2;
    return 1.0 + sin((time_elapsed+1.0)*PI)/2.0;
}

int run_level_select_functions(world* w) {
    if (w->level_mode == SET_POSITION) {
        if (w->mouse.l_released) {
            u32 l = w->level_select.current_level;
            w->level_select.levels[l].xpos = w->mouse.current_x-WINDOW_WIDTH/2+w->level_select.cx;
            w->level_select.levels[l].ypos = w->mouse.current_y-WINDOW_HEIGHT/2+w->level_select.cy;
        }
    }
    return 0;
}

int simulate_level_select(world* w) {
    if (!w->level_select.moving)
        return 0;
    float elapsed = (w->seconds-w->level_select.move_start) / ANIMATION_SINGLE_STEP_TIME/2.0;
    if (elapsed >= 1.0) {
        w->level_select.cx = w->level_select.end_x;
        w->level_select.cy = w->level_select.end_y;
        w->level_select.moving = false;
    } else {
        float progress = get_sigmoid_progress(elapsed);    
        float anti = get_anti_sigmoid_progress(elapsed);
        float dx, dy;
        if (w->level_select.moving_left_right) {
            dx = anti;
            dy = progress;
        } else {
            dx = progress;
            dy = anti;
        }
        w->level_select.cx = w->level_select.start_x + dx*(w->level_select.end_x-w->level_select.start_x);
        w->level_select.cy = w->level_select.start_y + dy*(w->level_select.end_y-w->level_select.start_y);
    }
    return 0;
}

int simulate_world(world* w, float seconds) {
    w->seconds = seconds;
    if (w->active_mode == LEVEL_SELECT)
        simulate_level_select(w);
    if (w->player == WIN && w->active_mode==IN_GAME) {
        w->win_scheduled = false;
        go_to_level_select(w);
        complete_current_level(w);
    }
    if (w->currently_moving) {
        if (w->seconds - w->animation_seconds_update > 1.0 / (float) ANIMATION_FRAMES_PER_SECOND) {
            bool still_moving = false;
            for (int i=0; i<w->movements_occupied; i++) {
                if (!w->movements[i].currently_moving)
                    continue;
                float elapsed = (seconds-w->movements[i].start_time) / w->movements[i].duration;
                if (elapsed < 0.0)
                    elapsed = 0.0;
                if (elapsed > 1.0)
                    w->movements[i].currently_moving = false;
                still_moving = still_moving || w->movements[i].currently_moving;
                float progress = get_linear_progress(elapsed);
                w->movements[i].x = w->movements[i].start_x + progress*w->movements[i].dx;
                w->movements[i].y = w->movements[i].start_y + progress*w->movements[i].dy;
            }
            w->currently_moving = still_moving;
        }
    }
    if (w->seconds - w->animation_seconds_update > 1.0 / (float) ANIMATION_FRAMES_PER_SECOND) {
        w->animation_seconds_update = w->seconds;
        for (int i=0; i<w->animations_occupied; i++) {
            if (w->animations[i].currently_animating) {
                animation_state as = w->animations[i];
                as.animation_data[as.current_animation_index].index++;
                if (as.animation_data[as.current_animation_index].index ==
                        as.animation_data[as.current_animation_index].length) {
                    as.animation_data[as.current_animation_index].index = 0;
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
                w->animations[i] = as;
            }
        }
    }
    if (w->win_scheduled && w->seconds > w->win_schedule_time) {
        w->player = WIN;
        add_sound_to_queue(w, PLAYER_WIN, w->seconds);
        clear_world_history(w);
    }
    if (w->editor.editor_enabled)
        run_editor_functions(w);
    if (w->active_mode == LEVEL_SELECT)
        run_level_select_functions(w);
    for (int i=0; i<SOUND_QUEUE_LENGTH; i++) {
        sound_queue_item sqi = w->sound_queue[i];
        if (sqi.sound != NO_SOUND && w->seconds > sqi.seconds) {
            Mix_PlayChannel(-1, w->sounds[sqi.sound].data, 0);
            w->sound_queue[i].sound = NO_SOUND;
        }
    }
    copy_grid_data_to_entities(w);
    remove_scheduled_entities(w);
    return 0;
}

int reset_inputs(world* w) {
    // called at the end of each tick to clear out state that was valid for
    // the length of the tick.
    w->mouse.l_released = false;
    w->mouse.r_released = false;
    return 0;
}

int change_world_xsize_right(world* w, int sign) {
    int ogx = w->x_size;
    u32 old_grid[MAX_WORLD_ENTITIES];
    memcpy(old_grid, w->grid_data, w->size*sizeof(u32));
    entity_data old_entities[MAX_WORLD_ENTITIES];
    memcpy(old_entities, w->entities, w->size*sizeof(entity_data));
    sign = sign / abs(sign);
    w->x_size = w->x_size + sign;
    w->size = w->x_size * w->y_size * w->z_size;
    init_entities(w);
    for (int z=0; z<w->z_size; z++) {
        for (int y=0; y<w->y_size; y++) {
            for (int x=0; x<w->x_size; x++) {
                int index = get_position_index_sizes(ogx, w->y_size, w->z_size, x, y, z);
                if (x==w->x_size-1 && sign>0)
                    add_entity(w, NONE, x, y, z);
                else
                    add_entity(w, old_entities[old_grid[index]].type, x, y, z);
            }
        }
    }
    return 0;
}

int change_world_xsize_left(world* w, int sign) {
    int ogx = w->x_size;
    u32 old_grid[MAX_WORLD_ENTITIES];
    memcpy(old_grid, w->grid_data, w->size*sizeof(u32));
    entity_data old_entities[MAX_WORLD_ENTITIES];
    memcpy(old_entities, w->entities, w->size*sizeof(entity_data));
    sign = sign / abs(sign);
    w->x_size = w->x_size + sign;
    w->size = w->x_size * w->y_size * w->z_size;
    init_entities(w);
    for (int z=0; z<w->z_size; z++) {
        for (int y=0; y<w->y_size; y++) {
            for (int x=0; x<w->x_size; x++) {
                if (sign>0){
                    int index = get_position_index_sizes(ogx, w->y_size, w->z_size, x-1, y, z);
                    if (x==0)
                        add_entity(w, NONE, 0, y, z);
                    else
                        add_entity(w, old_entities[old_grid[index]].type, x, y, z);
                }
                else {
                    int index = get_position_index_sizes(ogx, w->y_size, w->z_size, x+1, y, z);
                    add_entity(w, old_entities[old_grid[index]].type, x, y, z);
                }
            }
        }
    }
    return 0;
}

int change_world_ysize_top(world *w, int sign) {
    int ogy = w->y_size;
    u32 old_grid[MAX_WORLD_ENTITIES];
    memcpy(old_grid, w->grid_data, w->size*sizeof(u32));
    entity_data old_entities[MAX_WORLD_ENTITIES];
    memcpy(old_entities, w->entities, w->size*sizeof(entity_data));
    sign = sign / abs(sign);
    w->y_size = w->y_size + sign;
    w->size = w->x_size * w->y_size * w->z_size;
    init_entities(w);
    for (int z=0; z<w->z_size; z++) {
        for (int y=0; y<w->y_size; y++) {
            for (int x=0; x<w->x_size; x++) {
                int index = get_position_index_sizes(w->x_size, ogy, w->z_size, x, y, z);
                if (y==w->y_size-1 && sign>0)
                    add_entity(w, NONE, x, y, z);
                else
                    add_entity(w, old_entities[old_grid[index]].type, x, y, z);
            }
        }
    }
    return 0;
}

int change_world_ysize_bottom(world *w, int sign) {
    int ogy = w->y_size;
    u32 old_grid[MAX_WORLD_ENTITIES];
    memcpy(old_grid, w->grid_data, w->size*sizeof(u32));
    entity_data old_entities[MAX_WORLD_ENTITIES];
    memcpy(old_entities, w->entities, w->size*sizeof(entity_data));
    sign = sign / abs(sign);
    w->y_size = w->y_size + sign;
    w->size = w->x_size * w->y_size * w->z_size;
    init_entities(w);
    for (int z=0; z<w->z_size; z++) {
        for (int y=0; y<w->y_size; y++) {
            for (int x=0; x<w->x_size; x++) {
                if (sign>0){
                    int index = get_position_index_sizes(w->x_size, ogy, w->z_size, x, y-1, z);
                    if (y==0)
                        add_entity(w, NONE, x, 0, z);
                    else
                        add_entity(w, old_entities[old_grid[index]].type, x, y, z);
                }
                else {
                    int index = get_position_index_sizes(w->x_size, ogy, w->z_size, x, y+1, z);
                    add_entity(w, old_entities[old_grid[index]].type, x, y, z);
                }
            }
        }
    }
    return 0;
}

int change_world_xsize(world* w, int direction, int sign) {
    if (direction == 1)
        change_world_xsize_right(w, sign);
    if (direction == -1)
        change_world_xsize_left(w, sign);
    return 0;
}

int change_world_ysize(world* w, int direction, int sign) {
    if (direction == 1)
        change_world_ysize_top(w, sign);
    else
        change_world_ysize_bottom(w, sign);
    return 0;
}

int set_key_down(world* w, input_type it) {
    if (!w->input_state[it].down) {
        w->input_state[it].down = true;
        w->input_state[it].count = 0;
        w->input_state[it].seconds = w->seconds;
    }
    return 0;
}

int set_key_up(world* w, input_type it) {
    w->input_state[it].down = false;
    return 0;
}

int process_input_event(world* w, SDL_Event event) {
    if (event.type == SDL_MOUSEMOTION)
        process_mouse_motion(w, event.motion);
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
        process_mouse_button(w, event.button, event.type);
    if (event.type == SDL_KEYDOWN) {
        SDL_Keycode key = event.key.keysym.sym;
        if (key == SDLK_w)
            set_key_down(w, MOVE_UP);
        if (key == SDLK_a)
            set_key_down(w, MOVE_LEFT);
        if (key == SDLK_s)
            set_key_down(w, MOVE_DOWN);
        if (key == SDLK_d)
            set_key_down(w, MOVE_RIGHT);
        if (key == SDLK_z)
            set_key_down(w, UNDO_MOVE);
        if (key == SDLK_r)
            set_key_down(w, RESTART_LEVEL);
    }
    if (event.type == SDL_KEYUP) {
        SDL_Keycode key = event.key.keysym.sym;
        if (key == SDLK_ESCAPE)
            set_input(w, ESCAPE);
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
            set_input(w, ENTER);
        if (key == SDLK_w)
            set_key_up(w, MOVE_UP);
        if (key == SDLK_a)
            set_key_up(w, MOVE_LEFT);
        if (key == SDLK_s)
            set_key_up(w, MOVE_DOWN);
        if (key == SDLK_d)
            set_key_up(w, MOVE_RIGHT);
        if (key == SDLK_z)
            set_key_up(w, UNDO_MOVE);
        if (key == SDLK_r)
            set_key_up(w, RESTART_LEVEL);
        if (DEBUG_BUILD) {
            if (key == SDLK_e)
                w->editor.editor_enabled =  !w->editor.editor_enabled;
            if (key == SDLK_c)
                complete_current_level(w);
            if (key == SDLK_q)
                unlock_all_levels(w);
            if (key == SDLK_i)
                w->currently_moving = false;
            if (key == SDLK_f)
                toggle_fullscreen(global_r, w);
            if (key == SDLK_g)
                load_shaders(global_r);
            if (key == SDLK_n)
                load_next_level(w);
            if (key == SDLK_p)
                load_previous_level(w);
        }
    }
    return 0;
}

int handle_input_queue(world* w) {
    if (w->win_scheduled)
        return 0;
    for (int i=1; i<INPUT_TYPE_COUNT; i++) {
        if (w->input_state[i].down) {
            input_state_type ist = w->input_state[i];
            if ( (int)((w->seconds-ist.seconds)/1.0/INPUT_DOWN_REPEAT)+1 > ist.count) {
                if (ist.count > 0) {
                    printf("repeat %i\n", ist.count);
                }
                w->input_state[i].count++;
                set_input(w, i);
            }
        }
    }
}

int handle_input_state(world* w, SDL_GameController* controller) {
    // keyboard
    const Uint8* keyboard = SDL_GetKeyboardState(NULL);
    if (keyboard[SDL_SCANCODE_W] || keyboard[SDL_SCANCODE_UP])
        set_input(w, MOVE_UP);
    if (keyboard[SDL_SCANCODE_S] || keyboard[SDL_SCANCODE_DOWN])
        set_input(w, MOVE_DOWN);
    if (keyboard[SDL_SCANCODE_A] || keyboard[SDL_SCANCODE_LEFT])
        set_input(w, MOVE_LEFT);
    if (keyboard[SDL_SCANCODE_D] || keyboard[SDL_SCANCODE_RIGHT])
        set_input(w, MOVE_RIGHT);
    if (keyboard[SDL_SCANCODE_Z])
        set_input(w, UNDO_MOVE);
    if (keyboard[SDL_SCANCODE_R])
        set_input(w, RESTART_LEVEL);
    if (keyboard[SDL_SCANCODE_RETURN] || keyboard[SDL_SCANCODE_KP_ENTER] || keyboard[SDL_SCANCODE_SPACE] || keyboard[SDL_SCANCODE_X])
        set_input(w, ENTER);
    // controller buttons
    if (controller != NULL) {
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP))
            set_input(w, MOVE_UP);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
            set_input(w, MOVE_DOWN);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
            set_input(w, MOVE_LEFT);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
            set_input(w, MOVE_RIGHT);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))
            set_input(w, ENTER);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B))
            set_input(w, UNDO_MOVE);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y))
            set_input(w, RESTART_LEVEL);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START))
            set_input(w, ESCAPE);
        if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) > 16384)
            set_input(w, MOVE_RIGHT);
        if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) < -16384)
            set_input(w, MOVE_LEFT);
        if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) > 16384)
            set_input(w, MOVE_DOWN);
        if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) < -16384)
            set_input(w, MOVE_UP);
    }
    return 0;
}


/*
int set_callbacks(GLFWwindow* window) {
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    return 0;
}
*/
