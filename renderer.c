#include <stdio.h>
#include <stdlib.h>
//#include <sys/time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "renderer.h"
#include "game_settings.h"

u32 textures[2];
float ICE_EDGE_TOP = 38;
float ICE_EDGE_LEFT = 39;
float ICE_EDGE_CORNER = 40;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Errors: %s\n", description);
}

int test_shader_compilation(u32 shader, char* type) {
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

    glfwWindowHint(GLFW_SAMPLES, 16);
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    u32 vao;
    u32 vbo;
    u32 vertex_shader;
    u32 fragment_shader;
    u32 shader_program;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    shader_program = glCreateProgram();

    int width, height, nrChannels;
    int width1, height1, nrChannels1;
    int width2, height2, nrChannels2;
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("static/spritesheet.png",&width,&height,&nrChannels,0);
    unsigned char *data1 = stbi_load("static/m1.png",&width1,&height1,&nrChannels1,0);
    unsigned char *data2 = stbi_load("test_png.png",&width2,&height2,&nrChannels2,0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenTextures(2, &textures);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, textures[0]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, textures[1]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    stbi_image_free(data1);
    stbi_image_free(data2);
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
    r->ground_entities = (entity_type*) malloc(sizeof(entity_type)*MAX_WORLD_ENTITIES/2);
    load_shaders(r);
	printf("renderer has been initted\n");
    return 0;
}

float get_player_animation_frame(world* w, entity_data ed) {
    u32 anim_index = ed.animation_index;
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
        return 37.0;
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
    float xpos;
    if (!w->currently_moving)
        xpos = ed.x;
    else {
        int movement_index = ed.movement_index;
        if (!w->movements[movement_index].currently_moving)
            xpos = ed.x;
        else
            xpos = w->movements[movement_index].x;
    }
    if (ed.animation_index > 0) {
        animation_state as = w->animations[ed.animation_index];
        animation_frames_data afd = as.animation_data[as.current_animation_index];
        xpos += 2.0*afd.frame_positions[afd.index].x;
    }
    return xpos;
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

void add_vertex_to_buffer(renderer* r, world* w, float xpos, float ypos, float x_size, float y_size, float depth, float sprite_position, int orientation) {
    float blockx = BLOCK_WIDTH*1.0 / WINDOW_WIDTH*1.0;
    float blocky = BLOCK_HEIGHT*1.0 / WINDOW_HEIGHT*1.0;
    float tx1, ty1, tx2, ty2;
    if (orientation == 0) {
        tx1 = 0.0; ty1 = 0.0; tx2 = 1.0; ty2 = 1.0;
    } else if (orientation == 1) {
        tx1 = 1.0; ty1 = 0.0; tx2 = 0.0; ty2 = 1.0;
    } else if (orientation == 2) {
        tx1 = 1.0; ty1 = 1.0; tx2 = 0.0; ty2 = 0.0;
    } else if (orientation == 3) {
        tx1 = 0.0; ty1 = 1.0; tx2 = 1.0; ty2 = 0.0;
    }
    int j = r->buffer_occupied;
    r->buffer_occupied++;
    r->vertex_buffer[(36*j)+ 0] = X_PADDING + (blockx * xpos);
    r->vertex_buffer[(36*j)+ 1] = Y_PADDING + (blocky * ypos);
    r->vertex_buffer[(36*j)+ 2] = depth;
    r->vertex_buffer[(36*j)+ 3] = tx1;
    r->vertex_buffer[(36*j)+ 4] = ty1;
    r->vertex_buffer[(36*j)+ 5] = sprite_position;
    r->vertex_buffer[(36*j)+ 6] = X_PADDING + (blockx * xpos) + (x_size*blockx);
    r->vertex_buffer[(36*j)+ 7] = Y_PADDING + (blocky * ypos);
    r->vertex_buffer[(36*j)+ 8] = depth;
    r->vertex_buffer[(36*j)+ 9] = tx2;
    r->vertex_buffer[(36*j)+10] = ty1;
    r->vertex_buffer[(36*j)+11] = sprite_position;
    r->vertex_buffer[(36*j)+12] = X_PADDING + (blockx * xpos);
    r->vertex_buffer[(36*j)+13] = Y_PADDING + (blocky * ypos) + (y_size*blocky);
    r->vertex_buffer[(36*j)+14] = depth;
    r->vertex_buffer[(36*j)+15] = tx1;
    r->vertex_buffer[(36*j)+16] = ty2;
    r->vertex_buffer[(36*j)+17] = sprite_position;
    r->vertex_buffer[(36*j)+18] = X_PADDING + (blockx * xpos);
    r->vertex_buffer[(36*j)+19] = Y_PADDING + (blocky * ypos) + (y_size*blocky);
    r->vertex_buffer[(36*j)+20] = depth;
    r->vertex_buffer[(36*j)+21] = tx1;
    r->vertex_buffer[(36*j)+22] = ty2;
    r->vertex_buffer[(36*j)+23] = sprite_position;
    r->vertex_buffer[(36*j)+24] = X_PADDING + (blockx * xpos) + (x_size*blockx);
    r->vertex_buffer[(36*j)+25] = Y_PADDING + (blocky * ypos);
    r->vertex_buffer[(36*j)+26] = depth;
    r->vertex_buffer[(36*j)+27] = tx2;
    r->vertex_buffer[(36*j)+28] = ty1;
    r->vertex_buffer[(36*j)+29] = sprite_position;
    r->vertex_buffer[(36*j)+30] = X_PADDING + (blockx * xpos) + (x_size*blockx);
    r->vertex_buffer[(36*j)+31] = Y_PADDING + (blocky * ypos) + (y_size*blocky);
    r->vertex_buffer[(36*j)+32] = depth;
    r->vertex_buffer[(36*j)+33] = tx2;
    r->vertex_buffer[(36*j)+34] = ty2;
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
    add_vertex_to_buffer(r, w, xpos, ypos, x_size, y_size, depth, sprite_position, 0);
} 

entity_type get_ground_et_at(renderer* r, world* w, int x, int y) {
    int index = x + y*w->x_size;
    return r->ground_entities[index];
}

void draw_ice_edge(renderer* r, world* w, int x, int y, float sprite_position, int orientation) {
    float depth = -0.1;    
    depth += (float) y/100.0;
    float xpos = x;
    float ypos = y;
    float x_size = get_block_size_x(GROUND);
    float y_size = get_block_size_y(GROUND);
    add_vertex_to_buffer(r, w, xpos, ypos, x_size, y_size, depth, sprite_position, orientation);
    return;
}

void add_ice_edges(renderer* r, world* w) {
    int x, y;
    for (int x=0; x<w->x_size; x++) {
        for (int y=0; y<w->y_size; y++) {
            entity_type et = get_ground_et_at(r, w, x, y);
            if (et==SLIPPERY_GROUND || et==NONE)
                continue;
            // So we assign neighbours like a numpad. 1 at top left
            bool n1, n2, n3, n4, n6, n7, n8, n9;
            n1 = get_ground_et_at(r, w, x-1, y+1) == SLIPPERY_GROUND;
            n2 = get_ground_et_at(r, w, x+0, y+1) == SLIPPERY_GROUND;
            n3 = get_ground_et_at(r, w, x+1, y+1) == SLIPPERY_GROUND;
            n4 = get_ground_et_at(r, w, x-1, y+0) == SLIPPERY_GROUND;
            n6 = get_ground_et_at(r, w, x+1, y+0) == SLIPPERY_GROUND;
            n7 = get_ground_et_at(r, w, x-1, y-1) == SLIPPERY_GROUND;
            n8 = get_ground_et_at(r, w, x+0, y-1) == SLIPPERY_GROUND;
            n9 = get_ground_et_at(r, w, x+1, y-1) == SLIPPERY_GROUND;
            if (n1 && !n2 && !n4)
                draw_ice_edge(r, w, x, y, ICE_EDGE_CORNER, 0);
            if (n3 && !n2 && !n6)
                draw_ice_edge(r, w, x, y, ICE_EDGE_CORNER, 1);
            if (n7 && !n8 && !n4)
                draw_ice_edge(r, w, x, y, ICE_EDGE_CORNER, 3);
            if (n9 && !n8 && !n6)
                draw_ice_edge(r, w, x, y, ICE_EDGE_CORNER, 2);
            if (n2)
                draw_ice_edge(r, w, x, y, ICE_EDGE_TOP, 0);
            if (n8)
                draw_ice_edge(r, w, x, y, ICE_EDGE_TOP, 2);
            if (n4)
                draw_ice_edge(r, w, x, y, ICE_EDGE_LEFT, 0);
            if (n6)
                draw_ice_edge(r, w, x, y, ICE_EDGE_LEFT, 2);
        }
    }
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
    add_vertex_to_buffer(r, w, xpos, ypos, x_size, y_size, depth, sprite_position, 0);
}

int update_vertex_buffer(renderer* r, world* w) {
    // memset(r->ground_entities, NONE, MAX_WORLD_ENTITIES/2);
    for (int i=0; i<MAX_WORLD_ENTITIES/2; i++)
        r->ground_entities[i] = NONE;
    for (int i=0; i<w->entities_occupied; i++) {
        entity_data ed = w->entities[i];
        if (ed.z==0 && draw_ground_under(ed.type))
            add_ground_at_pos(r, w, ed.x, ed.y);
        if (ed.z==0)
            r->ground_entities[ed.y*w->x_size+ed.x] = ed.type;
        render_entity(r, w, ed);
    }
    add_ice_edges(r, w);
    return 0;
}

int render_game_scene(renderer* r, world* w) {
    update_vertex_buffer(r, w);
    glLinkProgram(r->shader.shader_program);
    glUseProgram(r->shader.shader_program);
    // glUniform1i(glGetUniformLocation(r->shader.shader_program, "spritesheet"), 0);
    // glUniform1i(glGetUniformLocation(r->shader.shader_program, "sdfsheet"), 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->shader.texture);
    // glBindTexture(GL_TEXTURE_2D, textures[0]);
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, textures[1]);
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
    // glActiveTexture(GL_TEXTURE0+0);
    // glBindTexture(GL_TEXTURE_2D, 0);
    // glActiveTexture(GL_TEXTURE1);
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
    level_option active_level = w->level_select.levels[w->level_select.current_level];
    float cx = w->level_select.cx - WINDOW_WIDTH/2;
    float cy = w->level_select.cy - WINDOW_HEIGHT/2;
    for (int i=0; i<w->level_select.total_levels; i++) {
        level_option level = w->level_select.levels[i];
        float lx = (level.xpos-cx);
        float ly = (level.ypos-cy);
        cb_ui_render_text(w->editor.ui_state, level.name.text, lx, ly);
        cb_ui_render_rectangle(w->editor.ui_state, lx-10, ly-5, 200, 21, 0.5);
        int l_index;
        l_index = w->level_select.levels[i].up_index;
        if (l_index > 0) {
            level_option l = w->level_select.levels[l_index];
            float lnx = (l.xpos-cx);
            float lny = (l.ypos-cy);
            cb_ui_render_line(w->editor.ui_state, lx+95, ly, lnx+95, lny, 1.0);
        }
        // l_index = w->level_select.levels[i].down_index;
        // if (l_index > 0) {
        //     level_option l = w->level_select.levels[l_index];
        //     cb_ui_render_line(w->editor.ui_state, level.xpos+95, level.ypos+16, l.xpos+95, l.ypos, 1.0);
        // }
        l_index = w->level_select.levels[i].left_index;
        if (l_index > 0) {
            level_option l = w->level_select.levels[l_index];
            float lnx = (l.xpos-cx);
            float lny = (l.ypos-cy);
            cb_ui_render_line(w->editor.ui_state, lx, ly, lnx, lny, 1.0);
        }
        // l_index = w->level_select.levels[i].right_index;
        // if (l_index > 0) {
        //     level_option l = w->level_select.levels[l_index];
        //     cb_ui_render_line(w->editor.ui_state, level.xpos, level.ypos, l.xpos, l.ypos, 1.0);
        // }
    }
    cb_ui_render_rectangle(w->editor.ui_state, WINDOW_WIDTH/2-10, WINDOW_HEIGHT/2-5, 200, 21, 0.5);
    return 0;
}

int render_scene(renderer* r, world* w) {
    glClearColor(0.1, 0.1, 0.101, 1.0);
    // glClearColor(0.94, 0.94, 0.92, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_MULTISAMPLE_ARB);
    if (w->active_mode == IN_GAME)
        render_game_scene(r, w);
    if (w->active_mode == MAIN_MENU)
        render_menu_scene(r, w);
    if (w->active_mode == LEVEL_SELECT)
        render_level_select(r, w);
    return 0;
}

