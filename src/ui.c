#include <stdio.h>
#include <stdlib.h>
#include "ui.h"
#include "renderer.h"
#include "cb_lib/cb_string.h"
#include "cb_lib/cb_types.h"
#include "game_settings.h"
#define DEFAULT_WIDGET_NUMBER 64
#define PIXEL_SIZE 16
#define BUTTON_PADDING 4
// TODO (19 Jun 2020 sam): There is a lot of confusion in the code here in the code.
// CHAR_BUFFER_SIZE is the number of chars I want to have in the buffer. But each char
// needs 24 floats. So there tends to be a bunch of confusion about that. Needs to be
// standardized.
#define CHAR_BUFFER_SIZE 2048

renderer* global_r;

int set_global_renderer(void* r) {
    global_r = (renderer*) r;
    return 0;
}

int get_window_width() {
    return global_r->size[0];
}

int get_window_height() {
    return global_r->size[1];
}

int cb_ui_test_shader_compilation(u32 shader, char* type) {
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        // fprintf(stderr, "%s shader failed to compile... %s\n", type, buffer);
        return -1;
    }
    return 0;
}

int compile_and_link_text_shader(u32* vertex_shader, u32* fragment_shader, u32* shader_program) {
    string vertex_source = read_file("mis_data/glsl/text_vertex.glsl");
    *vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(*vertex_shader, 1, &vertex_source.text, NULL);
    glCompileShader(*vertex_shader);
    cb_ui_test_shader_compilation(*vertex_shader, "text vertex");

    string fragment_source = read_file("mis_data/glsl/text_fragment.glsl");
    *fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*fragment_shader, 1, &fragment_source.text, NULL);
    glCompileShader(*fragment_shader);
    cb_ui_test_shader_compilation(*fragment_shader, "text fragment");

    *shader_program = glCreateProgram();
    glAttachShader(*shader_program, *vertex_shader);
    glAttachShader(*shader_program, *fragment_shader);
    glBindFragDataLocation(*shader_program, 0, "color");
    glLinkProgram(*shader_program);
    glUseProgram(*shader_program);
    dispose_string(&vertex_source);
    dispose_string(&fragment_source);
    return 0;
}

int clear_buffers(cb_ui_state* state) {
    state->values.rect_buffer.occupied = 0;
    memset(state->values.rect_buffer.vertex_buffer, 0, sizeof(float)*CHAR_BUFFER_SIZE*24);
    state->values.char_buffer.occupied = 0;
    memset(state->values.char_buffer.vertex_buffer, 0, sizeof(float)*CHAR_BUFFER_SIZE*24);
    state->values.line_buffer.occupied = 0;
    memset(state->values.line_buffer.vertex_buffer, 0, sizeof(float)*CHAR_BUFFER_SIZE*24);
    return 0;
}

int init_gl_values(cb_ui_state* state) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // TODO (19 Jun 2020 sam): I'm not sure if we need 2 vaos here. But when I tried to use just
    // one, the rects stopped working. So I'm not entirely sure what should be done in that case.
    u32 char_vao, char_vbo, rect_vao, rect_vbo, line_vao, line_vbo;

    glGenVertexArrays(1, &rect_vao);
    glBindVertexArray(rect_vao);
    glGenBuffers(1, &rect_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4 * CHAR_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &char_vao);
    glBindVertexArray(char_vao);
    glGenBuffers(1, &char_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, char_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4 * CHAR_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &line_vao);
    glBindVertexArray(line_vao);
    glGenBuffers(1, &line_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4 * CHAR_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    u32 text_vertex_shader;
    u32 text_fragment_shader;
    u32 text_shader_program;
    compile_and_link_text_shader(&text_vertex_shader, &text_fragment_shader, &text_shader_program);
    printf("mallocing... text vertex buffer\n");
    // TODO (03 Jul 2020 sam): Malloc single contiguous array.
    float* cb = (float*) malloc(sizeof(float) * CHAR_BUFFER_SIZE * 24);
    vertex_buffer_data char_buffer = {0, cb};
    float* rb = (float*) malloc(sizeof(float) * CHAR_BUFFER_SIZE * 24);
    vertex_buffer_data rect_buffer = {0, rb};
    float* lb = (float*) malloc(sizeof(float) * CHAR_BUFFER_SIZE * 24);
    vertex_buffer_data line_buffer = {0, lb};
    gl_values values = {char_vao, char_vbo, rect_vao, rect_vbo, line_vao, line_vbo, text_vertex_shader, text_fragment_shader, text_shader_program, rect_buffer, char_buffer, line_buffer};
    state->values = values;
    clear_buffers(state);
    return 0;
}

int init_character_glyphs(cb_ui_state* state) {
    printf("initting character glyphs...\t");
    unsigned char* ttf_buffer = (unsigned char*) malloc(150000);
    unsigned char* temp_bitmap = (unsigned char*) malloc(512*512);
    FILE* handler = fopen("mis_data/fonts/shortstack.ttf", "rb");
    if (!handler)
        printf("error opening font file I guess...\n");
    fread(ttf_buffer, 1, 150000, handler);
    stbtt_BakeFontBitmap(ttf_buffer,0, 32, temp_bitmap, 512, 512, 32, 95, state->glyphs);
    u32 font_texture;
    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    state->font_texture = font_texture;
    free(ttf_buffer);
    free(temp_bitmap);
    fclose(handler);
    return 0;
}

int render_chars(cb_ui_state* state) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_MULTISAMPLE_ARB);
    glLinkProgram(state->values.shader_program);
    glUseProgram(state->values.shader_program);
    int window_height = get_window_height();
    int window_width = get_window_width();
    glUniform2f(glGetUniformLocation(state->values.shader_program, "window_size"), window_width, window_height);
    glUniform4f(glGetUniformLocation(state->values.shader_program, "textColor"), 1, 1, 1, 1);
    glUniform1i(glGetUniformLocation(state->values.shader_program, "mode"), 1);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state->font_texture);
    glBindVertexArray(state->values.char_vao);
    glBindBuffer(GL_ARRAY_BUFFER, state->values.char_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*CHAR_BUFFER_SIZE*24, state->values.char_buffer.vertex_buffer);
    glDrawArrays(GL_TRIANGLES, 0, state->values.char_buffer.occupied*6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    state->values.char_buffer.occupied = 0;
    memset(state->values.char_buffer.vertex_buffer, 0, sizeof(float)*CHAR_BUFFER_SIZE*24);
    return 0;
}

int render_rectangles(cb_ui_state* state) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_MULTISAMPLE_ARB);
    glDepthFunc(GL_GEQUAL);
    glLinkProgram(state->values.shader_program);
    glUseProgram(state->values.shader_program);
    int window_height = get_window_height();
    int window_width = get_window_width();
    glUniform2f(glGetUniformLocation(state->values.shader_program, "window_size"), window_width, window_height);
    glUniform4f(glGetUniformLocation(state->values.shader_program, "textColor"), 0.2, 0.2, 0.23, 0.2);
    glUniform1i(glGetUniformLocation(state->values.shader_program, "mode"), 2);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(state->values.rect_vao);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, state->values.rect_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*CHAR_BUFFER_SIZE*24, state->values.rect_buffer.vertex_buffer);
    glDrawArrays(GL_TRIANGLES, 0, state->values.rect_buffer.occupied*6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    state->values.rect_buffer.occupied = 0;
    memset(state->values.rect_buffer.vertex_buffer, 0, sizeof(float)*CHAR_BUFFER_SIZE*24);
    return 0;
}

int render_lines(cb_ui_state* state) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_GEQUAL);
    glLinkProgram(state->values.shader_program);
    glUseProgram(state->values.shader_program);
    int window_height = get_window_height();
    int window_width = get_window_width();
    glUniform2f(glGetUniformLocation(state->values.shader_program, "window_size"), window_width, window_height);
    glUniform4f(glGetUniformLocation(state->values.shader_program, "textColor"), 0.5, 0.5, 0.63, 0.4);
    glUniform1i(glGetUniformLocation(state->values.shader_program, "mode"), 2);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(state->values.line_vao);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, state->values.line_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*CHAR_BUFFER_SIZE*24, state->values.line_buffer.vertex_buffer);
    glDrawArrays(GL_TRIANGLES, 0, state->values.line_buffer.occupied*6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    state->values.line_buffer.occupied = 0;
    memset(state->values.line_buffer.vertex_buffer, 0, sizeof(float)*CHAR_BUFFER_SIZE*24);
    return 0;
}

int init_ui(cb_ui_state* state) {
    init_gl_values(state);
    init_character_glyphs(state);
    return 0;
}

int cb_ui_render_rectangle(cb_ui_state* state, float xpos, float ypos, float w, float h, float opacity) {
    // TODO (18 Jun 2020 sam): We have removed the opacity functionality when we started
    // batch drawing the rectangles. Might need to be fixed.
    // we want text coordinates to be passed with top left of window as (0,0)
    int window_height = get_window_height();
    ypos = window_height-ypos;
    u32 index = state->values.rect_buffer.occupied;
    if (index >= CHAR_BUFFER_SIZE*24) {
        render_rectangles(state);
        index = 0;
    }
    state->values.rect_buffer.occupied += 24;
    state->values.rect_buffer.vertex_buffer[index +  0] = xpos;
    state->values.rect_buffer.vertex_buffer[index +  1] = ypos;
    state->values.rect_buffer.vertex_buffer[index +  2] = 0.0;
    state->values.rect_buffer.vertex_buffer[index +  3] = 0.0;
    state->values.rect_buffer.vertex_buffer[index +  4] = xpos;
    state->values.rect_buffer.vertex_buffer[index +  5] = ypos-h;
    state->values.rect_buffer.vertex_buffer[index +  6] = 1.0;
    state->values.rect_buffer.vertex_buffer[index +  7] = 1.0;
    state->values.rect_buffer.vertex_buffer[index +  8] = xpos+w;
    state->values.rect_buffer.vertex_buffer[index +  9] = ypos;
    state->values.rect_buffer.vertex_buffer[index + 10] = 1.0;
    state->values.rect_buffer.vertex_buffer[index + 11] = 0.0;
    state->values.rect_buffer.vertex_buffer[index + 12] = xpos;
    state->values.rect_buffer.vertex_buffer[index + 13] = ypos-h;
    state->values.rect_buffer.vertex_buffer[index + 14] = 1.0;
    state->values.rect_buffer.vertex_buffer[index + 15] = 1.0;
    state->values.rect_buffer.vertex_buffer[index + 16] = xpos+w;
    state->values.rect_buffer.vertex_buffer[index + 17] = ypos-h;
    state->values.rect_buffer.vertex_buffer[index + 18] = 0.0;
    state->values.rect_buffer.vertex_buffer[index + 19] = 0.0;
    state->values.rect_buffer.vertex_buffer[index + 20] = xpos+w;
    state->values.rect_buffer.vertex_buffer[index + 21] = ypos;
    state->values.rect_buffer.vertex_buffer[index + 22] = 0.0;
    state->values.rect_buffer.vertex_buffer[index + 23] = 1.0;
    return 0;
}

int cb_ui_render_line(cb_ui_state* state, float xpos1, float ypos1, float xpos2, float ypos2, float opacity) {
    // TODO (18 Jun 2020 sam): We have removed the opacity functionality when we started
    // batch drawing the rectangles. Might need to be fixed.
    // we want text coordinates to be passed with top left of window as (0,0)
    int window_height = get_window_height();
    ypos1 = window_height-ypos1;
    ypos2 = window_height-ypos2;
    float x1, y1, x2, y2;
    if (xpos1>xpos2) {
        x1 = xpos1;//min(xpos1, xpos2);
        x2 = xpos2;//max(xpos1, xpos2);
        y1 = ypos1;//max(ypos1, ypos2);
        y2 = ypos2;//min(ypos1, ypos2);
    } else {
        x2 = xpos1;//min(xpos1, xpos2);
        x1 = xpos2;//max(xpos1, xpos2);
        y2 = ypos1;//max(ypos1, ypos2);
        y1 = ypos2;//min(ypos1, ypos2);
    }
    u32 index = state->values.line_buffer.occupied;
    if (index >= CHAR_BUFFER_SIZE*24) {
        render_lines(state);
        index = 0;
    }
    state->values.line_buffer.occupied += 24;
    state->values.line_buffer.vertex_buffer[index +  0] = x1;
    state->values.line_buffer.vertex_buffer[index +  1] = y1;
    state->values.line_buffer.vertex_buffer[index +  2] = 0.0;
    state->values.line_buffer.vertex_buffer[index +  3] = 0.0;
    state->values.line_buffer.vertex_buffer[index +  4] = x1+4.0;
    state->values.line_buffer.vertex_buffer[index +  5] = y1-4.0;
    state->values.line_buffer.vertex_buffer[index +  6] = 0.0;
    state->values.line_buffer.vertex_buffer[index +  7] = 0.0;
    state->values.line_buffer.vertex_buffer[index +  8] = x2;
    state->values.line_buffer.vertex_buffer[index +  9] = y2;
    state->values.line_buffer.vertex_buffer[index + 10] = 0.0;
    state->values.line_buffer.vertex_buffer[index + 11] = 0.0;

    state->values.line_buffer.vertex_buffer[index + 12] = x1;
    state->values.line_buffer.vertex_buffer[index + 13] = y1;
    state->values.line_buffer.vertex_buffer[index + 14] = 0.0;
    state->values.line_buffer.vertex_buffer[index + 15] = 0.0;
    state->values.line_buffer.vertex_buffer[index + 16] = x2-4.0;
    state->values.line_buffer.vertex_buffer[index + 17] = y2+4.0;
    state->values.line_buffer.vertex_buffer[index + 18] = 0.0;
    state->values.line_buffer.vertex_buffer[index + 19] = 0.0;
    state->values.line_buffer.vertex_buffer[index + 20] = x2;
    state->values.line_buffer.vertex_buffer[index + 21] = y2;
    state->values.line_buffer.vertex_buffer[index + 22] = 0.0;
    state->values.line_buffer.vertex_buffer[index + 23] = 0.0;
    return 0;
}

void get_baked_quad(const stbtt_bakedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q) {
    // This function is a copy of stbtt_GetBakedQuad, but modified to support opengl y grows up.
    float ipw = 1.0f / pw, iph = 1.0f / ph;
    const stbtt_bakedchar *b = chardata + char_index;
    int round_x = floor((*xpos + b->xoff) + 0.5f);
    int round_y = floor((*ypos - b->yoff) + 0.5f);
    
    q->x0 = round_x;
    q->y0 = round_y;
    q->x1 = round_x + b->x1 - b->x0;
    q->y1 = round_y - b->y1 + b->y0;
    
    q->s0 = b->x0 * ipw;
    q->t0 = b->y0 * iph;
    q->s1 = b->x1 * ipw;
    q->t1 = b->y1 * iph;
    
    *xpos += b->xadvance;
}

int cb_ui_render_text(cb_ui_state* state, char* text, float x, float y) {
    // we want text coordinates to be passed with top left of window as (0,0)
    int window_height = get_window_height();
    y = window_height-y;
    y -= PIXEL_SIZE - BUTTON_PADDING;
    for (int i=0; i<strlen(text); i++) {
        char c = text[i] - 32;
        float yoff = state->glyphs[c].yoff;
        // TODO (17 Jun 2020 sam): Move to stb_tt release version. GetBakedQuad is meant
        // for test/dev. They have some other sets of API for release versions.
        stbtt_aligned_quad q;
        get_baked_quad(state->glyphs, 512,512, c, &x, &y, &q);
        u32 index = state->values.char_buffer.occupied;
        if (index >= CHAR_BUFFER_SIZE*24) {
            render_chars(state);
            index = 0;
        }
        state->values.char_buffer.occupied += 24;
        state->values.char_buffer.vertex_buffer[index + 0] = q.x0;
        state->values.char_buffer.vertex_buffer[index + 1] = q.y0;
        state->values.char_buffer.vertex_buffer[index + 2] = q.s0;
        state->values.char_buffer.vertex_buffer[index + 3] = q.t0;
        state->values.char_buffer.vertex_buffer[index + 4] = q.x1;
        state->values.char_buffer.vertex_buffer[index + 5] = q.y0;
        state->values.char_buffer.vertex_buffer[index + 6] = q.s1;
        state->values.char_buffer.vertex_buffer[index + 7] = q.t0;
        state->values.char_buffer.vertex_buffer[index + 8] = q.x0;
        state->values.char_buffer.vertex_buffer[index + 9] = q.y1;
        state->values.char_buffer.vertex_buffer[index + 10] = q.s0;
        state->values.char_buffer.vertex_buffer[index + 11] = q.t1;
        state->values.char_buffer.vertex_buffer[index + 12] = q.x0;
        state->values.char_buffer.vertex_buffer[index + 13] = q.y1;
        state->values.char_buffer.vertex_buffer[index + 14] = q.s0;
        state->values.char_buffer.vertex_buffer[index + 15] = q.t1;
        state->values.char_buffer.vertex_buffer[index + 16] = q.x1;
        state->values.char_buffer.vertex_buffer[index + 17] = q.y0;
        state->values.char_buffer.vertex_buffer[index + 18] = q.s1;
        state->values.char_buffer.vertex_buffer[index + 19] = q.t0;
        state->values.char_buffer.vertex_buffer[index + 20] = q.x1;
        state->values.char_buffer.vertex_buffer[index + 21] = q.y1;
        state->values.char_buffer.vertex_buffer[index + 22] = q.s1;
        state->values.char_buffer.vertex_buffer[index + 23] = q.t1;
    }
    return 0;
}

int init_cb_window(cb_window* w, char* title, u32 position[2], u32 size[2]) {
    printf("mallocing... init_cb_window\n");
    cb_widget* widget_mem = (cb_widget*) malloc(DEFAULT_WIDGET_NUMBER * sizeof(cb_widget));
    cb_widget_array widgets = {DEFAULT_WIDGET_NUMBER, 0, widget_mem};
    cb_window window = {title, {position[0], position[1]}, {size[0], size[1]}, 0, TEXT_HEIGHT, widgets};
    *w = window;
    return 0;
}

float get_text_width(cb_ui_state* state, char* text) {
    float w = 0.0;
    float x = 0.0;
    float y = 0.0;
    for (int i=0; i<strlen(text); i++) {
        char c = text[i] - 32;
        stbtt_aligned_quad q;
        x = 0.0;
        get_baked_quad(state->glyphs, 512,512, c, &x, &y, &q);
        w += x;
    }
    return w;
}

int cb_ui_render_text_centered_x(cb_ui_state* state, char* text, float x, float y) {
    float width = get_text_width(state, text);
    cb_ui_render_text(state, text, x-width/2.0, y);
    return 0;
}

int append_widget(cb_widget_array* array, cb_widget w) {
    if (array->allotted == array->used) {
        // realloc here
        // float c = 1.0/0.0;
    }
    array->widgets[array->used] = w;
    array->used++;
    return 0;
}

int add_text(cb_ui_state* state, cb_window* window, char* text, bool newline) {
    float width = get_text_width(state, text);
    cb_widget b = {text, CB_TEXT, {window->current_x, window->current_y}, {width, TEXT_HEIGHT}};
    window->current_x += width;
    append_widget(&window->widgets, b);
    if (newline)
        new_line(state, window, false);
    return 0;
}

bool mouse_in(cb_ui_state* state, cb_window* window, u32 pos[2], u32 size[2]) {
    float x1 = window->position[0]+pos[0];
    float x2 = window->position[0]+pos[0]+size[0];
    float y1 = window->position[1]+pos[1];
    float y2 = window->position[1]+pos[1]+size[1];
    float cx = state->mouse.current_x;
    float cy = state->mouse.current_y;
    float dx = state->mouse.l_down_x;
    float dy = state->mouse.l_down_y;
    return ( (cx > x1 && cx < x2) && (cy > y1 && cy < y2) && (dx > x1 && dx < x2) && (dy > y1 && dy < y2) );
}

bool add_button(cb_ui_state* state, cb_window* window, char* text, bool newline) {
    float width = get_text_width(state, text);
    cb_widget b = {text, CB_BUTTON, {window->current_x+BUTTON_PADDING, window->current_y+BUTTON_PADDING}, {width+2*BUTTON_PADDING, TEXT_HEIGHT+2*BUTTON_PADDING}};
    window->current_x += width+2*BUTTON_PADDING;
    append_widget(&window->widgets, b);
    if (newline)
        new_line(state, window, true);
    if (state->mouse.l_released && mouse_in(state, window, b.position, b.size)) {
        return true;
    }
    return false;
}

int new_line(cb_ui_state* state, cb_window* window, bool padding) {
    window->current_x = 0;
    window->current_y += TEXT_HEIGHT;
    if (padding) {
        window->current_y += BUTTON_PADDING;
    }
    return 0;
}

int vert_spacer(cb_ui_state* state, cb_window* window, bool padding) {
    window->current_x += BUTTON_PADDING;
    if (padding) {
        window->current_x += BUTTON_PADDING;
    }
    return 0;
}

int clear_window(cb_ui_state* state, cb_window* window) {
    window->current_x = 0;
    window->current_y = TEXT_HEIGHT;
    window->widgets.used = 0;
    return 0;
}

int cb_render_window(cb_ui_state* state, cb_window* window) {
    float width = get_text_width(state, window->title);
    cb_ui_render_rectangle(state, window->position[0]-UI_PADDING, window->position[1]-TEXT_HEIGHT-UI_PADDING, window->size[0]+2*UI_PADDING, window->size[1]+2*UI_PADDING+TEXT_HEIGHT, 0.4);
    cb_ui_render_text(state, window->title,
                      window->position[0]+(window->size[0]/2.0)-(width/2.0),
                      window->position[1]);
    for (int i=0; i<window->widgets.used; i++) {
        cb_widget w = window->widgets.widgets[i];
        float text_x = window->position[0]+w.position[0];
        float text_y = window->position[1]+w.position[1];
        if (w.type == CB_BUTTON) {
            float bg = 0.4;
            if (mouse_in(state, window, w.position, w.size))
                bg = 0.8;
            cb_ui_render_rectangle(state, text_x, text_y, w.size[0], w.size[1], bg);
            text_x += BUTTON_PADDING;
            text_y += BUTTON_PADDING;
        }
        cb_ui_render_text(state, w.title, text_x, text_y);
    }
    clear_window(state, window);
    return 0;
}

int render_ui(cb_ui_state* state) {
    render_rectangles(state);
    render_lines(state);
    render_chars(state);
    clear_buffers(state);
    return 0;
}
