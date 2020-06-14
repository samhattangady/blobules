#include <stdio.h>
#include <stdlib.h>
#include "ui.h"
#include "cb_lib/cb_string.h"
#include "cb_lib/cb_types.h"
#include "game_settings.h"
#define DEFAULT_WIDGET_NUMBER 64
#define PIXEL_SIZE 16
#define BUTTON_PADDING 4
#define CHAR_BUFFER_SIZE 512

int cb_ui_test_shader_compilation(uint shader, char* type) {
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

int compile_and_link_text_shader(uint* vertex_shader, uint* fragment_shader, uint* shader_program) {
    string vertex_source = read_file("glsl/text_vertex.glsl");
    *vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(*vertex_shader, 1, &vertex_source.text, NULL);
    glCompileShader(*vertex_shader);
    cb_ui_test_shader_compilation(*vertex_shader, "text vertex");
    
    string fragment_source = read_file("glsl/text_fragment.glsl");
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

int init_gl_values(cb_ui_state* state) {
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    uint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4 * CHAR_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    uint text_vertex_shader;
    uint text_fragment_shader;
    uint text_shader_program;
    compile_and_link_text_shader(&text_vertex_shader, &text_fragment_shader, &text_shader_program);
    printf("mallocing... text vertex buffer\n");
    float* vb = (float*) malloc(sizeof(float) * CHAR_BUFFER_SIZE * 24);
    char_vertex_buffer_data vertex_buffer = {0, vb};
    gl_values values = {VAO, VBO, text_vertex_shader, text_fragment_shader, text_shader_program, vertex_buffer};
    state->values = values;
    return 0;
}

int init_character_glyphs(cb_ui_state* state) {
    printf("initting character glyphs...\t");
    unsigned char* ttf_buffer = (unsigned char*) malloc(150000);
    unsigned char* temp_bitmap = (unsigned char*) malloc(512*512);
    printf("1...\t");
    FILE* handler = fopen("fonts/mono.ttf", "rb");
    if (!handler)
        printf("error opening font file I guess...\n");
    fread(ttf_buffer, 1, 150000, handler);
    memset(temp_bitmap, 42, 512*512);
    // TODO (12 May 2020 sam): Don't load from 0 to 128. Figure out the correct
    // range for all chars. Note that you will also have to load the correct char
    // accordingly.
    printf("2... \t");
    int loaded = stbtt_BakeFontBitmap(ttf_buffer,0, 16.0, temp_bitmap,512,512, 0,126, state->glyphs);
    
    printf("loaded bitmap = %i...\t", loaded);
    uint font_texture;
    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    state->font_texture = font_texture;
    // state->glyphs = cdata;
    free(ttf_buffer);
    free(temp_bitmap);
    return 0;
}

int render_chars(cb_ui_state* state) {
    glUseProgram(state->values.shader_program);
    glLinkProgram(state->values.shader_program);
    glUniform2f(glGetUniformLocation(state->values.shader_program, "window_size"), WINDOW_WIDTH, WINDOW_HEIGHT);
    glUniform4f(glGetUniformLocation(state->values.shader_program, "textColor"), 1, 1, 1, 1);
    glUniform1i(glGetUniformLocation(state->values.shader_program, "mode"), 1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, state->font_texture);
    glBindVertexArray(state->values.vao);
    glBindBuffer(GL_ARRAY_BUFFER, state->values.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*CHAR_BUFFER_SIZE*24, state->values.vertex_buffer.vertex_buffer, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, state->values.vertex_buffer.chars_occupied*6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    state->values.vertex_buffer.chars_occupied = 0;
    memset(state->values.vertex_buffer.vertex_buffer, 0, sizeof(float)*CHAR_BUFFER_SIZE*24);
    return 0;
}

int init_ui(cb_ui_state* state) {
    init_gl_values(state);
    init_character_glyphs(state);
    mouse_state_struct m = {false, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    state->mouse = m;
    return 0;
}

int cb_ui_render_rectangle(cb_ui_state* state, float xpos, float ypos, float w, float h, float opacity) {
    // TODO (12 May 2020 sam): Don't use multiple draw calls for each rect. Add it to
    // the same buffer as the chars if required.
    glUseProgram(state->values.shader_program);
    glLinkProgram(state->values.shader_program);
    glUniform2f(glGetUniformLocation(state->values.shader_program, "window_size"), WINDOW_WIDTH, WINDOW_HEIGHT);
    glUniform4f(glGetUniformLocation(state->values.shader_program, "textColor"), 0.2, 0.2, 0.23, opacity);
    glUniform1i(glGetUniformLocation(state->values.shader_program, "mode"), 2);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(state->values.vao);
    // we want text coordinates to be passed with top left of window as (0,0)
    ypos = WINDOW_HEIGHT-ypos;
    GLfloat ui_vertices[6][4] = {
        { xpos,   ypos,   0.0, 0.0 },
        { xpos,   ypos-h,       1.0, 1.0 },
        { xpos+w, ypos,   1.0, 0.0 },
        
        { xpos,   ypos-h,       1.0, 1.0 },
        { xpos+w,   ypos-h,   0.0, 0.0 },
        { xpos+w, ypos,   0.0, 1.0 },
    };
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, state->values.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ui_vertices), ui_vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return 0;
}

int cb_ui_render_text(cb_ui_state* state, char* text, float x, float y) {
    // we want text coordinates to be passed with top left of window as (0,0)
    y = WINDOW_HEIGHT-y;
    y -= PIXEL_SIZE - BUTTON_PADDING;
    for (int i=0; i<strlen(text); i++) {
        char c = text[i];
        float yoff = state->glyphs[c].yoff;
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(state->glyphs, 512,512, c, &x, &y, &q, 1);
        /*
                ft_char current = state->glyphs[c];
                float xpos = x + current.bearing_x;
                float ypos = y - (current.size_y - current.bearing_y);
                float w = current.size_x;
                float h = current.size_y;
                float l = current.x;
                float b = current.y;
                float dx = current.dx;
                float dy = current.dy;
        */
        uint index = state->values.vertex_buffer.chars_occupied;
        if (index >= CHAR_BUFFER_SIZE*24) {
            render_chars(state);
            index = 0;
        }
        state->values.vertex_buffer.chars_occupied += 24;
        state->values.vertex_buffer.vertex_buffer[index + 0] = q.x0;
        state->values.vertex_buffer.vertex_buffer[index + 1] = q.y1-yoff;
        state->values.vertex_buffer.vertex_buffer[index + 2] = q.s0;
        state->values.vertex_buffer.vertex_buffer[index + 3] = q.t0;
        state->values.vertex_buffer.vertex_buffer[index + 4] = q.x0;
        state->values.vertex_buffer.vertex_buffer[index + 5] = q.y0-yoff;
        state->values.vertex_buffer.vertex_buffer[index + 6] = q.s0;
        state->values.vertex_buffer.vertex_buffer[index + 7] = q.t1;
        state->values.vertex_buffer.vertex_buffer[index + 8] = q.x1;
        state->values.vertex_buffer.vertex_buffer[index + 9] = q.y0-yoff;
        state->values.vertex_buffer.vertex_buffer[index + 10] = q.s1;
        state->values.vertex_buffer.vertex_buffer[index + 11] = q.t1;
        state->values.vertex_buffer.vertex_buffer[index + 12] = q.x0;
        state->values.vertex_buffer.vertex_buffer[index + 13] = q.y1-yoff;
        state->values.vertex_buffer.vertex_buffer[index + 14] = q.s0;
        state->values.vertex_buffer.vertex_buffer[index + 15] = q.t0;
        state->values.vertex_buffer.vertex_buffer[index + 16] = q.x1;
        state->values.vertex_buffer.vertex_buffer[index + 17] = q.y0-yoff;
        state->values.vertex_buffer.vertex_buffer[index + 18] = q.s1;
        state->values.vertex_buffer.vertex_buffer[index + 19] = q.t1;
        state->values.vertex_buffer.vertex_buffer[index + 20] = q.x1;
        state->values.vertex_buffer.vertex_buffer[index + 21] = q.y1-yoff;
        state->values.vertex_buffer.vertex_buffer[index + 22] = q.s1;
        state->values.vertex_buffer.vertex_buffer[index + 23] = q.t0;
        /*
        state->values.vertex_buffer.vertex_buffer[index + 0] = 0.0;
        state->values.vertex_buffer.vertex_buffer[index + 1] = WINDOW_HEIGHT;
        state->values.vertex_buffer.vertex_buffer[index + 2] = 0.0;
        state->values.vertex_buffer.vertex_buffer[index + 3] = 0.0;
        
        state->values.vertex_buffer.vertex_buffer[index + 4] = 0.0;
        state->values.vertex_buffer.vertex_buffer[index + 5] = 0.0;
        state->values.vertex_buffer.vertex_buffer[index + 6] = 0.0;
        state->values.vertex_buffer.vertex_buffer[index + 7] = 1.0;
        
        state->values.vertex_buffer.vertex_buffer[index + 8] = WINDOW_WIDTH;
        state->values.vertex_buffer.vertex_buffer[index + 9] = 0.0;
        state->values.vertex_buffer.vertex_buffer[index + 10] = 1.0;
        state->values.vertex_buffer.vertex_buffer[index + 11] = 1.0;
        
        state->values.vertex_buffer.vertex_buffer[index + 12] = 0.0;
        state->values.vertex_buffer.vertex_buffer[index + 13] = WINDOW_HEIGHT;
        state->values.vertex_buffer.vertex_buffer[index + 14] = 0.0;
        state->values.vertex_buffer.vertex_buffer[index + 15] = 0.0;
        
        state->values.vertex_buffer.vertex_buffer[index + 16] = WINDOW_WIDTH;
        state->values.vertex_buffer.vertex_buffer[index + 17] = 0.0;
        state->values.vertex_buffer.vertex_buffer[index + 18] = 1.0;
        state->values.vertex_buffer.vertex_buffer[index + 19] = 1.0;
        
        state->values.vertex_buffer.vertex_buffer[index + 20] = WINDOW_WIDTH;
        state->values.vertex_buffer.vertex_buffer[index + 21] = WINDOW_HEIGHT;
        state->values.vertex_buffer.vertex_buffer[index + 22] = 1.0;
        state->values.vertex_buffer.vertex_buffer[index + 23] = 0.0;
        */
        //x += (current.advance >> 6);
    }
    return 0;
}

int init_cb_window(cb_window* w, char* title, uint position[2], uint size[2]) {
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
        char c = text[i];
        stbtt_aligned_quad q;
        x = 0.0;
        stbtt_GetBakedQuad(state->glyphs, 512,512, c, &x, &y, &q, 1);
        w += x;
    }
    return w;
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

bool mouse_in(cb_ui_state* state, cb_window* window, uint pos[2], uint size[2]) {
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
    render_chars(state);
    clear_window(state, window);
    state->mouse.l_released = false;
    state->mouse.r_released = false;
    return 0;
}

