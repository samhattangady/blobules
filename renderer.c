#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <sys/time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "renderer.h"
#include "game_settings.h"

#define SPRITE_DATA "static/sprite_data.txt"
#define SPRITE_HEIGHT 200
#define SPRITE_WIDTH 280

float ICE_EDGE_TOP = 12;
float ICE_EDGE_LEFT = 13;
float ICE_EDGE_CORNER = 14;
float GROUND_EDGE_TOP = 8;
float GROUND_EDGE_LEFT = 9;
float GROUND_EDGE_CORNER = 11;
float GROUND_EDGE_LEVEL_BELOW = 10;
float LS_WIDTH = 4000.0;
float LS_HEIGHT = 4000.0;
float LEVEL_SPRITE_WIDTH = 90.0;
float LEVEL_SPRITE_HEIGHT = 90.0;
float LEVEL_BACKGROUND_SDF_WIDTH = 256.0;
float LEVEL_BACKGROUND_SDF_HEIGHT = 256.0;

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

int get_ingame_buffer_size() {
    return sizeof(float) * MAX_WORLD_ENTITIES * 36;
}

int get_level_buffer_size() {
    return sizeof(float) * MAX_WORLD_ENTITIES * 36;
}

int load_shader(shader_data* shader, char* vertex_path, char* fragment_path) {
    string vertex_source = read_file(vertex_path);
    string fragment_source = read_file(fragment_path);
    glShaderSource(shader->vertex_shader, 1, &vertex_source.text, NULL);
    glCompileShader(shader->vertex_shader);
    test_shader_compilation(shader->vertex_shader, "vertex");
    glShaderSource(shader->fragment_shader, 1, &fragment_source.text, NULL);
    glCompileShader(shader->fragment_shader);
    test_shader_compilation(shader->fragment_shader, "frag");
    glAttachShader(shader->shader_program, shader->vertex_shader);
    glAttachShader(shader->shader_program, shader->fragment_shader);
    glBindFragDataLocation(shader->shader_program, 0, "fragColor");
    glLinkProgram(shader->shader_program);
    dispose_string(&vertex_source);
    dispose_string(&fragment_source);
    return 0;
}

int load_shaders(renderer* r) {
    load_shader(&r->ingame_shader, "glsl/simple_vertex.glsl", "glsl/simple_fragment.glsl");
    load_shader(&r->level_background_shader, "glsl/simple_vertex.glsl", "glsl/lb_fragment.glsl");
    load_shader(&r->level_shader, "glsl/simple_vertex.glsl", "glsl/simple_fragment.glsl");
    return 0;
}

int init_shader_data(shader_data* shader, char* fill_path, char* sdf_path) {
    u32 vertex_shader;
    u32 fragment_shader;
    u32 shader_program;

    int width, height, nrChannels;
    int width1, height1, nrChannels1;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *fill_data = stbi_load(fill_path,&width,&height,&nrChannels,0);
    unsigned char *sdf_data = stbi_load(sdf_path,&width1,&height1,&nrChannels1,0);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    shader_program = glCreateProgram();

    u32 fill_texture;
    glGenTextures(1, &fill_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fill_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fill_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    u32 sdf_texture;
    glGenTextures(1, &sdf_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sdf_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, sdf_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(fill_data);
    stbi_image_free(sdf_data);

    u32 framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    u32 rendered_texture;
    unsigned char* temp_bitmap = (unsigned char*) malloc(width*height*4);
    memset(temp_bitmap, 16, width*height*4);
    glGenTextures(1, &rendered_texture);
    glBindTexture(GL_TEXTURE_2D, rendered_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendered_texture, 0);
    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "could not generate the framebuffer");
    free(temp_bitmap);

    shader->vertex_shader = vertex_shader;
    shader->fragment_shader = fragment_shader;
    shader->shader_program = shader_program;
    shader->fill_texture = fill_texture;
    shader->sdf_texture = sdf_texture;
    shader->framebuffer = framebuffer;
    shader->rendered_texture = rendered_texture;
    return 0;
}

int init_buffer_data(buffer_data* buffer, int buffer_size) {
    u32 vao;
    u32 vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    buffer->vao = vao;
    buffer->vbo = vbo;
    buffer->buffer_size = buffer_size;
    buffer->buffer_occupied = 0;
    glBindVertexArray(buffer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, buffer->buffer_size, NULL, GL_DYNAMIC_DRAW);
    buffer->vertex_buffer = (float*) malloc(buffer->buffer_size);
    return 0;
}

int init_sprite_data(renderer* r) {
    int n = 0;
    FILE* sprite_datafile = fopen(SPRITE_DATA, "r");
    fscanf(sprite_datafile, "%i\n", &n);
    r->sprites = (sprite_data*) malloc(n*sizeof(sprite_data));
    for (int i=0; i<n; i++) {
        sprite_data sd;
        fscanf(sprite_datafile, "%i %i %f %f %f %f\n", &sd.w, &sd.h, &sd.x1, &sd.y1, &sd.x2, &sd.y2);
        r->sprites[i] = sd;
    }
    fclose(sprite_datafile);
    return 0;
}

int init_renderer(renderer* r, char* window_name) {
	printf("init renderer\n");
    SDL_Window *window;
    int window_height = WINDOW_HEIGHT;
    int window_width = WINDOW_WIDTH;
    printf("trying to create window\n");

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
    window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              window_width, window_height, SDL_WINDOW_OPENGL);
    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("GLEW could not initiate.\n");
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    shader_data ingame_shader;
    shader_data level_shader;
    shader_data level_background_shader;
    buffer_data ingame_buffer;
    buffer_data level_buffer;
    renderer r_ = { false, window, { (int)window_width, (int)window_height },
                    ingame_shader, level_background_shader, level_shader, ingame_buffer, level_buffer, NULL };
    *r = r_;
    init_shader_data(&r->ingame_shader, "static/fillsheet.png", "static/sdfsheet.png");
    init_shader_data(&r->level_background_shader, "static/brani2.png", "static/ls_sdf.png");
    init_shader_data(&r->level_shader, "static/lev1.png", "static/lev1.png");
    u32 framebuffer;

    init_buffer_data(&r->ingame_buffer, get_ingame_buffer_size());
    init_buffer_data(&r->level_buffer, get_level_buffer_size());
    r->ground_entities = (entity_type*) malloc(sizeof(entity_type)*MAX_WORLD_ENTITIES/2);
    load_shaders(r);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    init_sprite_data(r);
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

int get_sprite_position(entity_type et)  {
    switch (et) {
        case CUBE : return 0;
        case WALL : return 1;
        case GROUND : return 2;
        case HOT_TARGET : return 3;
        case COLD_TARGET : return 4;
        case FURNITURE : return 5;
        case PLAYER : return 6;
        case SLIPPERY_GROUND : return 7;
    }
    return 0;
}

int get_entity_sprite_position(entity_data ed, world* w) {
    entity_type type = ed.type;
    if (type == PLAYER)
        return 6; // + get_player_animation_frame(w, ed);
    if (type == CUBE)
        return 0;
    if (type == WALL)
        return 1;
    if (type == GROUND)
        return 2;
    if (type == SLIPPERY_GROUND)
        return 7;
    if (type == HOT_TARGET)
        return 3;
    if (type == COLD_TARGET)
        return 4;
    if (type == FURNITURE)
        return 5;
    return 0;
}


float get_block_size_x(renderer* r, entity_type type) {
    int pos = get_sprite_position(type);
    return 1.0 * r->sprites[pos].w / SPRITE_WIDTH;
}

float get_block_size_y(renderer* r, entity_type type) {
    int pos = get_sprite_position(type);
    return 1.0 * r->sprites[pos].h / SPRITE_HEIGHT;
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
    return xpos;
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

void add_single_vertex_to_buffer(buffer_data* buffer, float vx1, float vy1, float vx2, float vy2, float tx1, float ty1, float tx2, float ty2, float depth, float sprite) {
    int j = buffer->buffer_occupied;
    buffer->buffer_occupied++;
    buffer->vertex_buffer[(36*j)+ 0] = vx1;
    buffer->vertex_buffer[(36*j)+ 1] = vy1;
    buffer->vertex_buffer[(36*j)+ 2] = depth;
    buffer->vertex_buffer[(36*j)+ 3] = tx1;
    buffer->vertex_buffer[(36*j)+ 4] = ty1;
    buffer->vertex_buffer[(36*j)+ 5] = sprite;
    buffer->vertex_buffer[(36*j)+ 6] = vx2;
    buffer->vertex_buffer[(36*j)+ 7] = vy1;
    buffer->vertex_buffer[(36*j)+ 8] = depth;
    buffer->vertex_buffer[(36*j)+ 9] = tx2;
    buffer->vertex_buffer[(36*j)+10] = ty1;
    buffer->vertex_buffer[(36*j)+11] = sprite;
    buffer->vertex_buffer[(36*j)+12] = vx1;
    buffer->vertex_buffer[(36*j)+13] = vy2;
    buffer->vertex_buffer[(36*j)+14] = depth;
    buffer->vertex_buffer[(36*j)+15] = tx1;
    buffer->vertex_buffer[(36*j)+16] = ty2;
    buffer->vertex_buffer[(36*j)+17] = sprite;
    buffer->vertex_buffer[(36*j)+18] = vx1;
    buffer->vertex_buffer[(36*j)+19] = vy2;
    buffer->vertex_buffer[(36*j)+20] = depth;
    buffer->vertex_buffer[(36*j)+21] = tx1;
    buffer->vertex_buffer[(36*j)+22] = ty2;
    buffer->vertex_buffer[(36*j)+23] = sprite;
    buffer->vertex_buffer[(36*j)+24] = vx2;
    buffer->vertex_buffer[(36*j)+25] = vy1;
    buffer->vertex_buffer[(36*j)+26] = depth;
    buffer->vertex_buffer[(36*j)+27] = tx2;
    buffer->vertex_buffer[(36*j)+28] = ty1;
    buffer->vertex_buffer[(36*j)+29] = sprite;
    buffer->vertex_buffer[(36*j)+30] = vx2;
    buffer->vertex_buffer[(36*j)+31] = vy2;
    buffer->vertex_buffer[(36*j)+32] = depth;
    buffer->vertex_buffer[(36*j)+33] = tx2;
    buffer->vertex_buffer[(36*j)+34] = ty2;
    buffer->vertex_buffer[(36*j)+35] = sprite;
    return;
}

void add_vertex_to_buffer(renderer* r, world* w, float xpos, float ypos, float x_size, float y_size, float depth, int sprite_position, int orientation) {
    float window_width = r->size[0];
    float block_width = window_width*BLOCK_WIDTH;
    float block_height = window_width*BLOCK_HEIGHT;
    float blockx = block_width*1.0 / r->size[0]*1.0;
    float blocky = block_height*1.0 / r->size[1]*1.0;
    sprite_data sd = r->sprites[sprite_position];
    float tx1, ty1, tx2, ty2;
    float x_padding = get_x_padding(w);
    float y_padding = get_y_padding(w);
    float vx1 = x_padding + (blockx * xpos);
    float vy1 = y_padding + (blocky * ypos);
    float vx2 = x_padding + (blockx * xpos) + (x_size*blockx);
    float vy2 = y_padding + (blocky * ypos) + (y_size*blocky);
    if (orientation == 0) {
        tx1 = sd.x1; ty1 = sd.y1; tx2 = sd.x2; ty2 = sd.y2;
    } else if (orientation == 1) {
        tx1 = sd.x2; ty1 = sd.y2; tx2 = sd.x1; ty2 = sd.y2;
    } else if (orientation == 2) {
        tx1 = sd.x2; ty1 = sd.y2; tx2 = sd.x1; ty2 = sd.y1;
        vy1 = y_padding + (blocky * ypos) + ((1.0-y_size)*blocky);
        vy2 = y_padding + (blocky * ypos) + (1.0*blocky);
    } else if (orientation == 3) {
        tx1 = sd.x1; ty1 = sd.y2; tx2 = sd.x2; ty2 = sd.y1;
        // TODO (22 Aug 2020 sam): Add the x reversal stuff as well.
        vy1 = y_padding + (blocky * ypos) + ((1.0-y_size)*blocky);
        vy2 = y_padding + (blocky * ypos) + (1.0*blocky);
    }
    add_single_vertex_to_buffer(&r->ingame_buffer, vx1, vy1, vx2, vy2, tx1, ty1, tx2, ty2, depth, sprite_position);
}

void add_ground_at_pos(renderer* r, world* w, int x, int y) {
    float depth = 0;    
    depth += (float) y/100.0;
    float xpos = x;
    float ypos = y;
    float x_size = get_block_size_x(r, GROUND);
    float y_size = get_block_size_y(r, GROUND);
    // FIXME (26 Jun 2020 sam): Hardcoded value. Fix.
    int sprite_position = 2;
    add_vertex_to_buffer(r, w, xpos, ypos, x_size, y_size, depth, sprite_position, 0);
} 

entity_type get_ground_et_at(renderer* r, world* w, int x, int y) {
    if (x >= w->x_size || y >= w->y_size)
       return NONE; 
    int index = x + y*w->x_size;
    return r->ground_entities[index];
}

void draw_additional_sprite(renderer* r, world* w, int x, int y, int sprite_position, int orientation, float depth) {
    depth += (float) y/100.0;
    float xpos = x;
    float ypos = y;
    // TODO (22 Aug 2020 sam): cleanup the functions so that we don't have to do this calculation here.
    float x_size = 1.0 * r->sprites[sprite_position].w / SPRITE_WIDTH;
    float y_size = 1.0 * r->sprites[sprite_position].h / SPRITE_HEIGHT;
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
                draw_additional_sprite(r, w, x, y, ICE_EDGE_CORNER, 0, -0.1);
            if (n3 && !n2 && !n6)
                draw_additional_sprite(r, w, x, y, ICE_EDGE_CORNER, 1, -0.1);
            if (n7 && !n8 && !n4)
                draw_additional_sprite(r, w, x, y, ICE_EDGE_CORNER, 3, -0.1);
            if (n9 && !n8 && !n6)
                draw_additional_sprite(r, w, x, y, ICE_EDGE_CORNER, 2, -0.1);
            if (n2)
                draw_additional_sprite(r, w, x, y, ICE_EDGE_TOP, 0, -0.1);
            if (n8)
                draw_additional_sprite(r, w, x, y, ICE_EDGE_TOP, 2, -0.1);
            if (n4)
                draw_additional_sprite(r, w, x, y, ICE_EDGE_LEFT, 0, -0.1);
            if (n6)
                draw_additional_sprite(r, w, x, y, ICE_EDGE_LEFT, 2, -0.1);
        }
    }
}

void add_ground_edges(renderer* r, world* w) {
    int x, y;
    for (int x=0; x<w->x_size; x++) {
        for (int y=0; y<w->y_size; y++) {
            entity_type et = get_ground_et_at(r, w, x, y);
            if (et!=NONE)
                continue;
            // So we assign neighbours like a numpad. 1 at top left
            bool n1, n2, n3, n4, n6, n7, n8, n9;
            n1 = get_ground_et_at(r, w, x-1, y+1) != NONE;
            n2 = get_ground_et_at(r, w, x+0, y+1) != NONE;
            n3 = get_ground_et_at(r, w, x+1, y+1) != NONE;
            n4 = get_ground_et_at(r, w, x-1, y+0) != NONE;
            n6 = get_ground_et_at(r, w, x+1, y+0) != NONE;
            n7 = get_ground_et_at(r, w, x-1, y-1) != NONE;
            n8 = get_ground_et_at(r, w, x+0, y-1) != NONE;
            n9 = get_ground_et_at(r, w, x+1, y-1) != NONE;
            if (n1 && !n2 && !n4)
                draw_additional_sprite(r, w, x, y, GROUND_EDGE_CORNER, 2, -0.1);
            if (n3 && !n2 && !n6)
                draw_additional_sprite(r, w, x, y, GROUND_EDGE_CORNER, 3, -0.1);
            if (n7 && !n8 && !n4)
                draw_additional_sprite(r, w, x, y, GROUND_EDGE_CORNER, 1, -0.1);
            if (n9 && !n8 && !n6)
                draw_additional_sprite(r, w, x, y, GROUND_EDGE_CORNER, 0, -0.1);
            if (n2) {
                // draw_additional_sprite(r, w, x, y, GROUND_EDGE_LEVEL_BELOW, 0, -0.08);
                draw_additional_sprite(r, w, x, y, GROUND_EDGE_TOP, 3, -0.1);
            }
            if (n8)
                draw_additional_sprite(r, w, x, y, GROUND_EDGE_TOP, 0, -0.1);
            if (n4)
                draw_additional_sprite(r, w, x, y, GROUND_EDGE_LEFT, 2, -0.1);
            if (n6)
                draw_additional_sprite(r, w, x, y, GROUND_EDGE_LEFT, 0, -0.1);
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
        depth -= 0.8;
    int sprite_position = get_entity_sprite_position(ed, w);
    if (sprite_position < 0.0)
        return;
    float x_size = get_block_size_x(r, et);
    float y_size = get_block_size_y(r, et);
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
    add_ground_edges(r, w);
    return 0;
}

int render_game_scene(renderer* r, world* w) {
    update_vertex_buffer(r, w);
    glLinkProgram(r->ingame_shader.shader_program);
    glUseProgram(r->ingame_shader.shader_program);
    glViewport(0, 0, r->size[0], r->size[1]);
    glUniform1i(glGetUniformLocation(r->ingame_shader.shader_program, "spritesheet"), 0);
    glUniform1i(glGetUniformLocation(r->ingame_shader.shader_program, "sdfsheet"), 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->ingame_shader.fill_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, r->ingame_shader.sdf_texture);
    glBindVertexArray(r->ingame_buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->ingame_buffer.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->ingame_buffer.buffer_size, r->ingame_buffer.vertex_buffer);
    int position_attribute = glGetAttribLocation(r->ingame_shader.shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
    int tex_attribute = glGetAttribLocation(r->ingame_shader.shader_program, "tex");
    glEnableVertexAttribArray(tex_attribute);
    glVertexAttribPointer(tex_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*) (3*sizeof(float)));
    int uni_ybyx = glGetUniformLocation(r->ingame_shader.shader_program, "ybyx");
    int uni_time = glGetUniformLocation(r->ingame_shader.shader_program, "time");
    glUniform1f(uni_ybyx, r->size[1]*1.0/r->size[0]);
    glUniform1f(uni_time, w->seconds);
    glDrawArrays(GL_TRIANGLES, 0, r->ingame_buffer.buffer_size);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0+0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    memset(r->ingame_buffer.vertex_buffer, 0, r->ingame_buffer.buffer_size);
    r->ingame_buffer.buffer_occupied = 0;
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

int update_level_vertex_background_buffer(renderer* r, world* w) {
    float vx1 = -1.0;
    float vy1 = -1.0;
    float vx2 = 1.0;
    float vy2 = 1.0;
    float depth = 0.1;
    float tx1, ty1, tx2, ty2;
    float cx = w->level_select.cx;
    float cy = w->level_select.cy;
    tx1 = (cx - r->size[0]/2) / LS_WIDTH;
    tx2 = (cx + r->size[0]/2) / LS_WIDTH;
    ty1 = -(cy + r->size[1]/2) / LS_HEIGHT;
    ty2 = -(cy - r->size[1]/2) / LS_HEIGHT;
    add_single_vertex_to_buffer(&r->level_buffer, vx1, vy1, vx2, vy2, tx1, ty1, tx2, ty2, depth, 0.1);
    return 0;
}

float screen_pixel_to_vertex_x(renderer* r, float x) {
    float window_width = (float)r->size[0];
    float vertex_x = (x/window_width) * 2.0;
    vertex_x -= 1.0;
    return vertex_x;
}

float screen_pixel_to_vertex_y(renderer* r, float y) {
    float window_height = (float)r->size[1];
    float vertex_y = (y/window_height) * 2.0;
    vertex_y -= 1.0;
    return vertex_y;
}

float bg_texture_pixel_to_vertex_x(renderer* r, float x) {
    float window_height = LS_WIDTH;
    float vertex_x = (x/window_height) * 2.0;
    vertex_x -= 1.0;
    return vertex_x;
}

float bg_texture_pixel_to_vertex_y(renderer* r, float y) {
    float window_height = LS_HEIGHT;
    float vertex_y = (y/window_height) * 2.0;
    vertex_y -= 1.0;
    return vertex_y;
}

float my_min(float a, float b) {
    if (a<b) return a;
    return b;
}

float my_max(float a, float b) {
    if (a>b) return a;
    return b;
}

float my_abs(float a) {
    if (a < 0.0)
        return -a;
    return a;
}

int draw_single_line(renderer* r, float x1, float y1, float x2, float y2, float opacity) {
    // only meant for lines that are vertical/horizontal. (just fills full box)
    float half_line_thickness = 3.0;
    if (x1 == x2)  {
        // line is vertical
        x1 += half_line_thickness;
        x2 -= half_line_thickness;
    } else {
        y1 -= half_line_thickness;
        y2 += half_line_thickness;
    }
    float vx1 = bg_texture_pixel_to_vertex_x(r, x1);
    float vy1 = bg_texture_pixel_to_vertex_y(r, y1);
    float vx2 = bg_texture_pixel_to_vertex_x(r, x2);
    float vy2 = bg_texture_pixel_to_vertex_y(r, y2);
    add_single_vertex_to_buffer(&r->level_buffer, vx1, vy1, vx2, vy2, 0.0, 0.0, 1.0, 1.0, opacity, 3.0);
    return 0;
}

int draw_line_connections(renderer* r, world* w, level_connection con) {
    level_option level = w->level_select.levels[con.head_index];
    level_option next_level = w->level_select.levels[con.tail_index];
    float lx1 = (level.xpos);
    float ly1 = LS_HEIGHT-(level.ypos);
    float lx2 = (next_level.xpos);
    float ly2 = LS_HEIGHT-(next_level.ypos);
    if (con.is_left_right) {
        lx1 += 20.0;
        lx2 -= 20.0;
    } else {
        ly1 -= 20.0;
        ly2 += 20.0;
    }
    float opacity = 0.5 * pow(my_min(1.0, (w->seconds - con.draw_time)/2.0), 0.7);
    float extent = 3.0 * pow(my_min((w->seconds - level.complete_time) / 2.0, 1.0), 0.7);
    // we want 3 lines, to form like a staircase between levels.
    if (con.is_left_right) {
        float mid_x = (lx1 + lx2) / 2.0;
        draw_single_line(r, lx1, ly1, mid_x, ly1, opacity);
        draw_single_line(r, mid_x, ly1, mid_x, ly2, opacity);
        draw_single_line(r, mid_x, ly2, lx2, ly2, opacity);
    } else {
        float mid_y = (ly1 + ly2) / 2.0;
        draw_single_line(r, lx1, ly1, lx1, mid_y, opacity);
        draw_single_line(r, lx1, mid_y, lx2, mid_y, opacity);
        draw_single_line(r, lx2, mid_y, lx2, ly2, opacity);
    }
    return 0;
}

float get_connection_length(world* w, int con_index) {
    level_connection con = w->level_select.connections[con_index];
    level_option l1 = w->level_select.levels[con.head_index];
    level_option l2 = w->level_select.levels[con.tail_index];
    return sqrt(pow(l1.xpos-l2.xpos, 2.0) + pow(l1.ypos-l2.ypos, 2.0));
}

float get_min_connection_length(world* w, level_option lev) {
    float radius = LEVEL_BACKGROUND_SDF_WIDTH;
    level_connection con;
    int next;
    next = lev.up_index;
    if (next != -1)
        radius = min(radius, get_connection_length(w, next));
    next = lev.left_index;
    if (next != -1)
        radius = min(radius, get_connection_length(w, next));
    next = lev.down_index;
    if (next != -1)
        radius = min(radius, get_connection_length(w, next));
    next = lev.right_index;
    if (next != -1)
        radius = min(radius, get_connection_length(w, next));
    return radius;
}

int update_level_vertex_background_sdf_buffer(renderer* r, world* w) {
    level_option active_level = w->level_select.levels[w->level_select.current_level];
    float circle_sprite = 1.0;
    float line_sprite = 2.0;
    // render the background sdf for color fill
    for (int i=0; i<w->level_select.total_levels; i++) {
        level_option level = w->level_select.levels[i];
        if (!level.completed)
            continue;
        float radius = get_min_connection_length(w, level);
        radius = LEVEL_BACKGROUND_SDF_WIDTH;
        float lx = (level.xpos);
        float ly = LS_HEIGHT-(level.ypos);
        float extent = 3.0 * pow(my_min((w->seconds - level.complete_time) / 2.0, 1.0), 0.7);
        float vx1 = bg_texture_pixel_to_vertex_x(r, lx-radius*extent/2.0);
        float vy1 = bg_texture_pixel_to_vertex_y(r, ly-radius*extent/2.0);
        float vx2 = bg_texture_pixel_to_vertex_x(r, lx+radius*extent/2.0);
        float vy2 = bg_texture_pixel_to_vertex_y(r, ly+radius*extent/2.0);
        add_single_vertex_to_buffer(&r->level_buffer, vx1, vy1, vx2, vy2, 0.0, 0.0, 1.0, 1.0, 0.0, circle_sprite);
    }
    // render the lines sdf for lines connecting levels
    // currently we are only drawing the left, up links (as a left will have a right, that will be same)
    for (int i=0; i<w->level_select.total_connections; i++) {
        level_connection con = w->level_select.connections[i];
        if (!con.discovered)
            continue;
        draw_line_connections(r, w, con);
    }
    return 0;
}

int update_level_select_vertex_buffer(renderer* r, world* w) {
    // we want a single smaller circle in middle of screen always
    float vx1 = screen_pixel_to_vertex_x(r, r->size[0]/2-LEVEL_SPRITE_WIDTH/4.0);
    float vy1 = screen_pixel_to_vertex_y(r, r->size[1]/2-LEVEL_SPRITE_HEIGHT/4.0);
    float vx2 = screen_pixel_to_vertex_x(r, r->size[0]/2+LEVEL_SPRITE_WIDTH/4.0);
    float vy2 = screen_pixel_to_vertex_y(r, r->size[1]/2+LEVEL_SPRITE_HEIGHT/4.0);
    add_single_vertex_to_buffer(&r->level_buffer, vx1, vy1, vx2, vy2, 0.0, 0.0, 1.0, 1.0, -0.1, -1.0);
    level_option active_level = w->level_select.levels[w->level_select.current_level];
    float cx = w->level_select.cx - r->size[0]/2;
    float cy = w->level_select.cy - r->size[1]/2;
    for (int i=0; i<w->level_select.total_levels; i++) {
        level_option level = w->level_select.levels[i];
        if (!level.unlocked)
            continue;
        float lx = (level.xpos-cx);
        float ly = r->size[1]-(level.ypos-cy);
        float vx1 = screen_pixel_to_vertex_x(r, lx-LEVEL_SPRITE_WIDTH/2.0);
        float vy1 = screen_pixel_to_vertex_y(r, ly-LEVEL_SPRITE_HEIGHT/2.0);
        float vx2 = screen_pixel_to_vertex_x(r, lx+LEVEL_SPRITE_WIDTH/2.0);
        float vy2 = screen_pixel_to_vertex_y(r, ly+LEVEL_SPRITE_HEIGHT/2.0);
        add_single_vertex_to_buffer(&r->level_buffer, vx1, vy1, vx2, vy2, 0.0, 0.0, 1.0, 1.0, -0.1, -1.0);
        if (i == w->level_select.current_level) {
        }
    }
    return 0;
}

int render_level_select(renderer* r, world* w) {
    int position_attribute, tex_attribute, uni_ybyx, uni_time;
    // background sdf -> drawing is done in the shader. We just want to pass in the coords
    glBindFramebuffer(GL_FRAMEBUFFER, r->level_background_shader.framebuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    update_level_vertex_background_sdf_buffer(r, w);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT); 
    glLinkProgram(r->level_background_shader.shader_program);
    glUseProgram(r->level_background_shader.shader_program);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->level_background_shader.sdf_texture);
    glBindVertexArray(r->level_buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->level_buffer.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->level_buffer.buffer_size, r->level_buffer.vertex_buffer);
    position_attribute = glGetAttribLocation(r->level_background_shader.shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
    tex_attribute = glGetAttribLocation(r->level_background_shader.shader_program, "tex");
    glEnableVertexAttribArray(tex_attribute);
    glVertexAttribPointer(tex_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*) (3*sizeof(float)));
    uni_ybyx = glGetUniformLocation(r->level_background_shader.shader_program, "ybyx");
    uni_time = glGetUniformLocation(r->level_background_shader.shader_program, "time");
    glUniform1f(uni_ybyx, LS_HEIGHT/LS_WIDTH);
    glUniform1f(uni_time, w->seconds);
    glUniform1i(glGetUniformLocation(r->level_background_shader.shader_program, "mode"), 1);
    glViewport(0, 0, LS_WIDTH, LS_HEIGHT);
    glDrawArrays(GL_TRIANGLES, 0, r->level_buffer.buffer_size);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    memset(r->level_buffer.vertex_buffer, 0, r->level_buffer.buffer_size);
    r->level_buffer.buffer_occupied = 0;
    // background
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    update_level_vertex_background_buffer(r, w);
    glLinkProgram(r->level_background_shader.shader_program);
    glUseProgram(r->level_background_shader.shader_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->level_background_shader.fill_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, r->level_background_shader.rendered_texture);
    glBindVertexArray(r->level_buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->level_buffer.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->level_buffer.buffer_size, r->level_buffer.vertex_buffer);
    position_attribute = glGetAttribLocation(r->level_background_shader.shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
    tex_attribute = glGetAttribLocation(r->level_background_shader.shader_program, "tex");
    glEnableVertexAttribArray(tex_attribute);
    glVertexAttribPointer(tex_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*) (3*sizeof(float)));
    uni_ybyx = glGetUniformLocation(r->level_background_shader.shader_program, "ybyx");
    uni_time = glGetUniformLocation(r->level_background_shader.shader_program, "time");
    glUniform1i(glGetUniformLocation(r->level_background_shader.shader_program, "spritesheet"), 0);
    glUniform1i(glGetUniformLocation(r->level_background_shader.shader_program, "sdfsheet"), 1);
    glUniform1i(glGetUniformLocation(r->level_background_shader.shader_program, "mode"), 2);
    glUniform1f(uni_ybyx, r->size[1]*1.0/r->size[0]);
    glUniform1f(uni_time, w->seconds);
    glViewport(0, 0, r->size[0], r->size[1]);
    glDrawArrays(GL_TRIANGLES, 0, r->level_buffer.buffer_size);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    memset(r->level_buffer.vertex_buffer, 0, r->level_buffer.buffer_size);
    r->level_buffer.buffer_occupied = 0;
    // level options
    update_level_select_vertex_buffer(r, w);
    glLinkProgram(r->level_shader.shader_program);
    glUseProgram(r->level_shader.shader_program);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->level_shader.sdf_texture);
    glBindVertexArray(r->level_buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->level_buffer.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->level_buffer.buffer_size, r->level_buffer.vertex_buffer);
    position_attribute = glGetAttribLocation(r->level_shader.shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
    tex_attribute = glGetAttribLocation(r->level_shader.shader_program, "tex");
    glEnableVertexAttribArray(tex_attribute);
    glVertexAttribPointer(tex_attribute, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*) (3*sizeof(float)));
    uni_ybyx = glGetUniformLocation(r->level_shader.shader_program, "ybyx");
    uni_time = glGetUniformLocation(r->level_shader.shader_program, "time");
    glUniform1f(uni_ybyx, r->size[1]*1.0/r->size[0]);
    glUniform1f(uni_time, w->seconds);
    glViewport(0, 0, r->size[0], r->size[1]);
    glDrawArrays(GL_TRIANGLES, 0, r->level_buffer.buffer_size);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    memset(r->level_buffer.vertex_buffer, 0, r->level_buffer.buffer_size);
    r->level_buffer.buffer_occupied = 0;
    // TODO (14 Jun 2020 sam): Keep the ui state in a more accessible location
    // cb_ui_render_text(w->editor.ui_state, get_level_mode(w), 20, 20);
    // level_option active_level = w->level_select.levels[w->level_select.current_level];
    // float cx = w->level_select.cx - r->size[0]/2;
    // float cy = w->level_select.cy - r->size[1]/2;
    // for (int i=0; i<w->level_select.total_levels; i++) {
    //     level_option level = w->level_select.levels[i];
    //     if (!level.unlocked)
    //         continue;
    //     float lx = (level.xpos-cx);
    //     float ly = (level.ypos-cy);

    //     cb_ui_render_text(w->editor.ui_state, level.name.text, lx, ly);
    //     cb_ui_render_rectangle(w->editor.ui_state, lx-10, ly-5, 200, 21, 0.5);
    // }
    // cb_ui_render_rectangle(w->editor.ui_state, r->size[0]/2-10, r->size[1]/2-5, 200, 21, 0.5);

    return 0;
}

int render_scene(renderer* r, world* w) {
    glClearColor(0.1, 0.1, 0.101, 1.0);
    glClearColor(0.94, 0.94, 0.92, 1.0);
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

int set_fullscreen(renderer* r, bool flag) {
    r->fullscreen = flag;
    int mode;
    if (flag)
        mode = SDL_WINDOW_FULLSCREEN_DESKTOP;
    else
        mode = 0;
    SDL_SetWindowFullscreen(r->window, mode);
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    if (flag) {
        r->size[0] = DM.w;
        r->size[1] = DM.h;
    } else {
        r->size[0] = WINDOW_WIDTH;
        r->size[1] = WINDOW_HEIGHT;
    }
    return 0;
}

int toggle_fullscreen(renderer* r) {
    set_fullscreen(r, !r->fullscreen);
    return 0;
}
