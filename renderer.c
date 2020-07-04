#include <stdio.h>
#include <stdlib.h>
//#include <sys/time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "renderer.h"
#include "game_settings.h"


static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Errors: %s\n", description);
}

int test_shader_compilation(uint shader, char* type) {
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        fprintf(stderr, "%s shader failed to compile... %s\n", type, buffer);
        return -1;
    }
    return 0;
}

int get_buffer_size() {
    return sizeof(float) * MAX_WORLD_ENTITIES * 36;
}

int load_shaders(renderer* r) {
    string vertex_source = read_file("glsl/simple_vertex.glsl");
    string fragment_source = read_file("glsl/simple_fragment.glsl");
    //printf("%s\n", vertex_source.text);
    //printf("%s\n", fragment_source.text);
    glShaderSource(r->shader.vertex_shader, 1, &vertex_source.text, NULL);
    glCompileShader(r->shader.vertex_shader);
    test_shader_compilation(r->shader.vertex_shader, "vertex");
    glShaderSource(r->shader.fragment_shader, 1, &fragment_source.text, NULL);
    glCompileShader(r->shader.fragment_shader);
    test_shader_compilation(r->shader.fragment_shader, "frag");
    glAttachShader(r->shader.shader_program, r->shader.vertex_shader);
    glAttachShader(r->shader.shader_program, r->shader.fragment_shader);
    glBindFragDataLocation(r->shader.shader_program, 0, "fragColor");
    glLinkProgram(r->shader.shader_program);
    glUseProgram(r->shader.shader_program);
    dispose_string(&vertex_source);
    dispose_string(&fragment_source);
    return 0;
}

int init_renderer(renderer* r, char* window_name) {
	printf("init renderer\n");
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* window;
    int window_height = WINDOW_HEIGHT;
    int window_width = WINDOW_WIDTH;
    printf("trying to create window\n");

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, window_name, NULL, NULL);
    if (!window) {
        printf("GLFW could not create a window...\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("GLEW could not initiate.\n");
        return -1;
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

    uint vao;
    uint vbo;
    uint vertex_shader;
    uint fragment_shader;
    uint shader_program;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    shader_program = glCreateProgram();

    int width, height, nrChannels;
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    uint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("static/spritesheet.png",&width,&height,&nrChannels,0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    glGenerateMipmap(GL_TEXTURE_2D);
    renderer r_ = { window, { (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT },
        { vao, vbo, vertex_shader, fragment_shader, shader_program, texture},
        0, 0, NULL };
    *r = r_;
    r->buffer_size = get_buffer_size();
    r->buffer_occupied = 0;
    glBufferData(GL_ARRAY_BUFFER, r->buffer_size, NULL, GL_DYNAMIC_DRAW);
    printf("mallocing... vertex_buffer\n");
    r->vertex_buffer = (float*) malloc(r->buffer_size);
    load_shaders(r);
	printf("renderer has been initted\n");
    return 0;
}

float get_slippery_sprite_position(world* w, int x, int y, int z) {
    // TODO (23 Jun 2020 sam): I don't know the best way to do something like this really
    // TODO (23 Jun 2020 sam): Logic is a little hard here. Don't know if we can do it
    // procedurally. I want it to look like the ice is pushed off the board, but in many
    // places the logic of checking none as well makes it look messed up. We probably will
    // have to manually decide what each one should be in some way.
    // TODO (23 Jun 2020 sam): Need to add checks for top_right, top_left etc as well.
    // Right now if there is a bunch of blocks together, they dont combine, and create
    // an ugly grid shape. Yuck. But that might need upto 64(?) more sprites.
    bool top = get_entity_at(w, get_position_index(w, x, y+1, z)) == SLIPPERY_GROUND;
    bool bottom = get_entity_at(w, get_position_index(w, x, y-1, z)) == SLIPPERY_GROUND;
    bool left = get_entity_at(w, get_position_index(w, x-1, y, z)) == SLIPPERY_GROUND;
    bool right = get_entity_at(w, get_position_index(w, x+1, y, z)) == SLIPPERY_GROUND;
    // bool top = (get_entity_at(w, get_position_index(w, x, y+1, z)) == SLIPPERY_GROUND) ||
    //            (get_entity_at(w, get_position_index(w, x, y+1, z)) == NONE);
    // bool bottom = (get_entity_at(w, get_position_index(w, x, y-1, z)) == SLIPPERY_GROUND) ||
    //             (get_entity_at(w, get_position_index(w, x, y-1, z)) == NONE);
    // bool left = (get_entity_at(w, get_position_index(w, x-1, y, z)) == SLIPPERY_GROUND) ||
    //             (get_entity_at(w, get_position_index(w, x-1, y, z)) == NONE);
    // bool right = (get_entity_at(w, get_position_index(w, x+1, y, z)) == SLIPPERY_GROUND) ||
    //             (get_entity_at(w, get_position_index(w, x+1, y, z)) == NONE);
    if (!top && !bottom && !left && !right)
        return 0.0;
    if ( top && !bottom && !left && !right)
        return 1.0;
    if (!top &&  bottom && !left && !right)
        return 2.0;
    if (!top && !bottom && !left &&  right)
        return 3.0;
    if (!top && !bottom &&  left && !right)
        return 4.0;
    if ( top &&  bottom && !left && !right)
        return 5.0;
    if (!top && !bottom &&  left &&  right)
        return 6.0;
    if ( top && !bottom &&  left && !right)
        return 7.0;
    if ( top && !bottom && !left &&  right)
        return 8.0;
    if (!top &&  bottom &&  left && !right)
        return 9.0;
    if (!top &&  bottom && !left &&  right)
        return 10.0;
    if ( top &&  bottom &&  left && !right)
        return 11.0;
    if ( top &&  bottom && !left &&  right)
        return 12.0;
    if ( top && !bottom &&  left &&  right)
        return 13.0;
    if (!top &&  bottom &&  left &&  right)
        return 14.0;
    if ( top &&  bottom &&  left &&  right)
        return 15.0;
    return 0.0;
}

float get_player_animation_frame(world* w, entity_data ed) {
    uint anim_index = ed.animation_index;
    animation_state as = w->animations[anim_index];
    animation_frames_data afd = as.animation_data[as.current_animation_index];
    int f = afd.frame_list[afd.index];
    return (float) f;
}

float get_entity_sprite_position(entity_data ed, world* w) {
    entity_type type = ed.type;
    if (type == PLAYER)
        return 7.0 + get_player_animation_frame(w, ed);
    if (type == CUBE)
        return 0.0;
    if (type == WALL)
        return 1.0;
    if (type == GROUND)
        return 2.0;
    if (type == SLIPPERY_GROUND)
        return 19.0 +18.0 +get_slippery_sprite_position(w, ed.x, ed.y, ed.z);
    if (type == HOT_TARGET)
        return 3.0;
    if (type == COLD_TARGET)
        return 4.0;
    if (type == FURNITURE)
        return 5.0;
    if (type == REFLECTOR)
        return 6.0;
    return -1.0;
}

float get_block_size_x(entity_type type) {
    // if (type == PLAYER)
    //     return 0.8;
    // if (type == CUBE)
    //     return 0.8;
    // if (type == FURNITURE)
    //     return 0.6;
    return 1.0;
}

float get_block_size_y(entity_type type) {
    if (type == NONE)
        return 1.0;
    // if (type==SLIPPERY_GROUND)
    //     return 1.0;
    if (type == GROUND || type == SLIPPERY_GROUND)
        return 1.0;
    return 2.0;
}

float get_x_pos(world* w, entity_data ed) {
    if (!w->currently_moving)
        return ed.x;
    int movement_index = ed.movement_index;
    if (!w->movements[movement_index].currently_moving)
        return ed.x;
    return w->movements[movement_index].x;
}

float get_y_pos(world* w, entity_data ed) {
    if (!w->currently_moving)
        return ed.y;
    int movement_index = ed.movement_index;
    if (!w->movements[movement_index].currently_moving)
        return ed.y;
    return w->movements[movement_index].y;
}

int get_vertex_buffer_index(world* w, int x, int y, int z) {
    // We need to arrange the vertices in the buffer such that higher the y
    // value, lower the index value, as we want the things with lower y value
    // to be drawn after the higher y values so that they overlap nicely and
    // look correct in that regard.
    return (z * w->x_size *w-> y_size) + ((w->y_size-1-y) *w->x_size) + x;
}

bool draw_ground_under(entity_type et) {
    return (et == SLIPPERY_GROUND || et == HOT_TARGET || et == COLD_TARGET);
}

void add_vertex_to_buffer(renderer* r, world* w, float xpos, float ypos, float x_size, float y_size, float depth, float sprite_position) {
    float blockx = BLOCK_WIDTH*1.0 / WINDOW_WIDTH*1.0;
    float blocky = BLOCK_HEIGHT*1.0 / WINDOW_HEIGHT*1.0;
    int j = r->buffer_occupied;
    r->buffer_occupied++;
    r->vertex_buffer[(36*j)+ 0] = X_PADDING + (blockx * xpos);
    r->vertex_buffer[(36*j)+ 1] = Y_PADDING + (blocky * ypos);
    r->vertex_buffer[(36*j)+ 2] = depth;
    r->vertex_buffer[(36*j)+ 3] = 0.0;
    r->vertex_buffer[(36*j)+ 4] = 0.0;
    r->vertex_buffer[(36*j)+ 5] = sprite_position;
    r->vertex_buffer[(36*j)+ 6] = X_PADDING + (blockx * xpos) + (x_size*blockx);
    r->vertex_buffer[(36*j)+ 7] = Y_PADDING + (blocky * ypos);
    r->vertex_buffer[(36*j)+ 8] = depth;
    r->vertex_buffer[(36*j)+ 9] = 1.0;
    r->vertex_buffer[(36*j)+10] = 0.0;
    r->vertex_buffer[(36*j)+11] = sprite_position;
    r->vertex_buffer[(36*j)+12] = X_PADDING + (blockx * xpos);
    r->vertex_buffer[(36*j)+13] = Y_PADDING + (blocky * ypos) + (y_size*blocky);
    r->vertex_buffer[(36*j)+14] = depth;
    r->vertex_buffer[(36*j)+15] = 0.0;
    r->vertex_buffer[(36*j)+16] = 1.0;
    r->vertex_buffer[(36*j)+17] = sprite_position;
    r->vertex_buffer[(36*j)+18] = X_PADDING + (blockx * xpos);
    r->vertex_buffer[(36*j)+19] = Y_PADDING + (blocky * ypos) + (y_size*blocky);
    r->vertex_buffer[(36*j)+20] = depth;
    r->vertex_buffer[(36*j)+21] = 0.0;
    r->vertex_buffer[(36*j)+22] = 1.0;
    r->vertex_buffer[(36*j)+23] = sprite_position;
    r->vertex_buffer[(36*j)+24] = X_PADDING + (blockx * xpos) + (x_size*blockx);
    r->vertex_buffer[(36*j)+25] = Y_PADDING + (blocky * ypos);
    r->vertex_buffer[(36*j)+26] = depth;
    r->vertex_buffer[(36*j)+27] = 1.0;
    r->vertex_buffer[(36*j)+28] = 0.0;
    r->vertex_buffer[(36*j)+29] = sprite_position;
    r->vertex_buffer[(36*j)+30] = X_PADDING + (blockx * xpos) + (x_size*blockx);
    r->vertex_buffer[(36*j)+31] = Y_PADDING + (blocky * ypos) + (y_size*blocky);
    r->vertex_buffer[(36*j)+32] = depth;
    r->vertex_buffer[(36*j)+33] = 1.0;
    r->vertex_buffer[(36*j)+34] = 1.0;
    r->vertex_buffer[(36*j)+35] = sprite_position;
}

void add_ground_at_pos(renderer* r, world* w, int x, int y) {
    float depth = 0;    
    depth += (float) y/100.0;
    float xpos = x;
    float ypos = y;
    float x_size = get_block_size_x(GROUND);
    float y_size = get_block_size_y(GROUND);
    // FIXME (26 Jun 2020 sam): Hardcoded value. Fix.
    float sprite_position = 2.0;
    add_vertex_to_buffer(r, w, xpos, ypos, x_size, y_size, depth, sprite_position);
} 

void render_entity(renderer* r, world* w, entity_data ed) {
    entity_type et = ed.type;
    if (et == NONE)
        return;
    // TODO (20 Jun 2020 sam): See the performance implications of blockx and blocky
    // This needs to be calculated for every single vertex added to the buffer.
    // But it's such a simple calculation that I don't know whether the memory overhead
    // more or less performant...
    float depth = (float)-ed.z;
    // we want the depth buffer to include some data about the y position as well so that
    // things closer to us are drawn first.
    depth += (float) ed.y/100.0;
    if (et==SLIPPERY_GROUND)
        depth -= 0.1;
    if (et==HOT_TARGET || et==COLD_TARGET)
        depth -= 1.0;
    float sprite_position = get_entity_sprite_position(ed, w);
    if (sprite_position < 0.0)
        return;
    float x_size = get_block_size_x(et);
    float y_size = get_block_size_y(et);
    float xpos = get_x_pos(w, ed);
    float ypos = get_y_pos(w, ed);
    add_vertex_to_buffer(r, w, xpos, ypos, x_size, y_size, depth, sprite_position);
}

int update_vertex_buffer(renderer* r, world* w) {
    for (int i=0; i<w->entities_occupied; i++) {
        entity_data ed = w->entities[i];
        if (ed.z==0 && draw_ground_under(ed.type))
            add_ground_at_pos(r, w, ed.x, ed.y);
        render_entity(r, w, ed);
    }
    return 0;
}

int render_game_scene(renderer* r, world* w) {
    update_vertex_buffer(r, w);
    glLinkProgram(r->shader.shader_program);
    glUseProgram(r->shader.shader_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->shader.texture);
    glBindVertexArray(r->shader.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->shader.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->buffer_size, r->vertex_buffer);
    int position_attribute = glGetAttribLocation(r->shader.shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
    int tex_attribute = glGetAttribLocation(r->shader.shader_program, "tex");
    glEnableVertexAttribArray(tex_attribute);
    glVertexAttribPointer(tex_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*) (3*sizeof(float)));
    int uni_ybyx = glGetUniformLocation(r->shader.shader_program, "ybyx");
    int uni_time = glGetUniformLocation(r->shader.shader_program, "time");
    glUniform1f(uni_ybyx, WINDOW_HEIGHT*1.0/WINDOW_WIDTH);
    glUniform1f(uni_time, w->seconds);
    glViewport(0, 0, r->size[0], r->size[1]);
    glDrawArrays(GL_TRIANGLES, 0, r->buffer_size);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    memset(r->vertex_buffer, 0, r->buffer_size);
    r->buffer_occupied = 0;
    return 0;
}

int render_menu_scene(renderer* r, world* w) {
    // TODO (14 Jun 2020 sam): Keep the ui state in a more accessible location
    for (int i=0; i<w->main_menu.total_options; i++) {
        cb_ui_render_text(w->editor.ui_state, w->main_menu.options[i].text.text,
                          100, 100+(i*30));
    }
    cb_ui_render_rectangle(w->editor.ui_state, 95, 95+(w->main_menu.active_option*30), 100, 21, 0.5);
    return 0;
}

char* get_level_mode(world* w) {
    if (w->level_mode == NEUTRAL)
        return "level_mode: neutral";
    if (w->level_mode == SET_POSITION)
        return "level_mode: set_position";
    if (w->level_mode == SET_LEFT)
        return "level_mode: set_left";
    if (w->level_mode == SET_RIGHT)
        return "level_mode: set_right";
    if (w->level_mode == SET_BOTTOM)
        return "level_mode: set_bottom";
    if (w->level_mode == SET_TOP)
        return "level_mode: set_top";
    return "NA";
}

int render_level_select(renderer* r, world* w) {
    // TODO (14 Jun 2020 sam): Keep the ui state in a more accessible location
    cb_ui_render_text(w->editor.ui_state, get_level_mode(w), 20, 20);
    for (int i=0; i<w->level_select.total_levels; i++) {
        level_option level = w->level_select.levels[i];
        cb_ui_render_text(w->editor.ui_state, level.name.text, level.xpos, level.ypos, 1.0);
        cb_ui_render_rectangle(w->editor.ui_state, level.xpos-10, level.ypos-5, 200, 21, 0.5);
        int l_index;
        l_index = w->level_select.levels[i].up_index;
        if (l_index > 0) {
            level_option l = w->level_select.levels[l_index];
            cb_ui_render_line(w->editor.ui_state, level.xpos+95, level.ypos, l.xpos+95, l.ypos, 1.0);
        }
        // l_index = w->level_select.levels[i].down_index;
        // if (l_index > 0) {
        //     level_option l = w->level_select.levels[l_index];
        //     cb_ui_render_line(w->editor.ui_state, level.xpos+95, level.ypos+16, l.xpos+95, l.ypos, 1.0);
        // }
        l_index = w->level_select.levels[i].left_index;
        if (l_index > 0) {
            level_option l = w->level_select.levels[l_index];
            cb_ui_render_line(w->editor.ui_state, level.xpos, level.ypos, l.xpos, l.ypos, 1.0);
        }
        // l_index = w->level_select.levels[i].right_index;
        // if (l_index > 0) {
        //     level_option l = w->level_select.levels[l_index];
        //     cb_ui_render_line(w->editor.ui_state, level.xpos, level.ypos, l.xpos, l.ypos, 1.0);
        // }
    }
    level_option active_level = w->level_select.levels[w->level_select.current_level];
    cb_ui_render_rectangle(w->editor.ui_state, active_level.xpos-10, active_level.ypos-5, 200, 21, 0.5);
    return 0;
}

int render_scene(renderer* r, world* w) {
    glClearColor(0.1, 0.1, 0.101, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    if (w->active_mode == IN_GAME)
        render_game_scene(r, w);
    if (w->active_mode == MAIN_MENU)
        render_menu_scene(r, w);
    if (w->active_mode == LEVEL_SELECT)
        render_level_select(r, w);
    return 0;
}

