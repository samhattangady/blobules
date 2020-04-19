#include "ui.h"
#include "cb_lib/cb_string.h"
#include "cb_lib/cb_types.h"
#include "game_settings.h"
#define DEFAULT_WIDGET_NUMBER 64
#define PIXEL_SIZE 16
#define BUTTON_PADDING 4

int cb_ui_test_shader_compilation(uint shader, char* type) {
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    uint text_vertex_shader;
    uint text_fragment_shader;
    uint text_shader_program;
    compile_and_link_text_shader(&text_vertex_shader, &text_fragment_shader, &text_shader_program);
    gl_values values = {VAO, VBO, text_vertex_shader, text_fragment_shader, text_shader_program};
    state->values = values;
    return 0;
}

int init_character_glyphs(cb_ui_state* state) {
    FT_Library ft_library;
    FT_Face ft_face;
    if (FT_Init_FreeType(&ft_library)) return -1;
    FT_New_Face(ft_library,
                "fonts/mono.ttf",
                0,
                &ft_face);
    FT_Set_Pixel_Sizes(ft_face, 0, 16);
    {
        // TODO (05 Apr 2020 sam): Figure out how to control i and c within the for loop
        uint i = 0;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (GLubyte c=0; c<128; c++) {
            FT_Load_Char(ft_face, c, FT_LOAD_RENDER);
            uint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RED,
                ft_face->glyph->bitmap.width,
                ft_face->glyph->bitmap.rows,
                0, GL_RED, GL_UNSIGNED_BYTE,
                ft_face->glyph->bitmap.buffer
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // TODO (05 Apr 2020 sam): Figure out how to declare and insert in one step;
            ft_char current_char = {
                texture,
                ft_face->glyph->bitmap.width,
                ft_face->glyph->bitmap.rows,
                ft_face->glyph->bitmap_left,
                ft_face->glyph->bitmap_top,
                ft_face->glyph->advance.x
            };
            state->glyphs[i] = current_char;
            i++;
        }
    }
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_library);
    return 0;
}

int init_ui(cb_ui_state* state) {
    init_character_glyphs(state);
    init_gl_values(state);
    mouse_state_struct m = {false, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    state->mouse = m;
    return 0;
}

int cb_ui_render_rectangle(cb_ui_state* state, float xpos, float ypos, float w, float h) {
    glUseProgram(state->values.shader_program);
    glLinkProgram(state->values.shader_program);
    glUniform2f(glGetUniformLocation(state->values.shader_program, "window_size"), 1536, 864);
    glUniform4f(glGetUniformLocation(state->values.shader_program, "textColor"), 0.2, 0.2, 0.4,0.4);
    glUniform1i(glGetUniformLocation(state->values.shader_program, "mode"), 2);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(state->values.vao);
    // we want text coordinates to be passed with top left of window as (0,0)
    ypos = WINDOW_HEIGHT-ypos;
    GLfloat ui_vertices[6][4] = {
        { xpos,   ypos,   0.0, 0.0 },
        { xpos,   ypos-h,       1.0, 1.0 },
        { xpos+w, ypos,   0.0, 0.0 },

        { xpos,   ypos-h,       1.0, 1.0 },
        { xpos+w,   ypos-h,   0.0, 0.0 },
        { xpos+w, ypos,   0.0, 0.0 },
    };
    glBindBuffer(GL_ARRAY_BUFFER, state->values.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ui_vertices), ui_vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return 0;
}

int cb_ui_render_text(cb_ui_state* state, char* text, float x, float y) {
    glUseProgram(state->values.shader_program);
    glLinkProgram(state->values.shader_program);
    glUniform2f(glGetUniformLocation(state->values.shader_program, "window_size"), 1536, 864);
    glUniform4f(glGetUniformLocation(state->values.shader_program, "textColor"), 1, 1, 1, 1);
    glUniform1i(glGetUniformLocation(state->values.shader_program, "mode"), 1);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(state->values.vao);
    // we want text coordinates to be passed with top left of window as (0,0)
    y = WINDOW_HEIGHT-y;
    for (int i=0; i<strlen(text); i++) {
        // TODO (05 Apr 2020 sam): Convert to using a single bitmap texture for this
        char c = text[i];
        ft_char current = state->glyphs[c];
        float xpos = x + current.bearing_x;
        float ypos = y - (current.size_y - current.bearing_y);
        float w = current.size_x;
        float h = current.size_y;
        GLfloat ui_vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        glBindTexture(GL_TEXTURE_2D, current.texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, state->values.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ui_vertices), ui_vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (current.advance >> 6);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return 0;
}

int init_cb_window(cb_window* w, char* title, uint position[2], uint size[2]) {
    cb_widget* widget_mem = (cb_widget*) malloc(DEFAULT_WIDGET_NUMBER * sizeof(cb_widget));
    cb_widget_array widgets = {DEFAULT_WIDGET_NUMBER, 0, widget_mem};
    cb_window window = {title, {position[0], position[1]}, {size[0], size[1]}, 0, TEXT_HEIGHT, widgets};
    *w = window;
    return 0;
}

float get_text_width(cb_ui_state* state, char* text) {
    float w = 0.0;
    for (int i=0; i<strlen(text); i++) {
        // TODO (05 Apr 2020 sam): Convert to using a single bitmap texture for this
        char c = text[i];
        ft_char current = state->glyphs[c];
        w += (current.advance >> 6);
    }
    return w;
}

int append_widget(cb_widget_array* array, cb_widget w) {
    if (array->allotted == array->used) {
        // realloc here
        float c = 1.0/0.0;
    }
    array->widgets[array->used] = w;
    array->used++;
}

int add_text(cb_ui_state* state, cb_window* window, char* text, bool newline) {
    float width = get_text_width(state, text);
    cb_widget b = {text, CB_TEXT, {window->current_x, window->current_y}, {width, TEXT_HEIGHT}};
    window->current_x += width;
    append_widget(&window->widgets, b);
    if (newline)
        new_line(state, window, false);
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
}

int clear_window(cb_ui_state* state, cb_window* window) {
    window->current_x = 0;
    window->current_y = TEXT_HEIGHT;
    window->widgets.used = 0;
    return 0;
}

int cb_render_window(cb_ui_state* state, cb_window* window) {
    float width = get_text_width(state, window->title);
    cb_ui_render_rectangle(state, window->position[0]-UI_PADDING, window->position[1]-TEXT_HEIGHT-UI_PADDING, window->size[0]+2*UI_PADDING, window->size[1]+2*UI_PADDING+TEXT_HEIGHT);
    cb_ui_render_text(state, window->title,
            window->position[0]+(window->size[0]/2.0)-(width/2.0),
            window->position[1]);
    for (int i=0; i<window->widgets.used; i++) {
        cb_widget w = window->widgets.widgets[i];
        float text_x = window->position[0]+w.position[0];
        float text_y = window->position[1]+w.position[1];
        if (w.type == CB_BUTTON) {
            cb_ui_render_rectangle(state, text_x, text_y, w.size[0], w.size[1]);
            text_x += BUTTON_PADDING;
            text_y += BUTTON_PADDING;
        }
        cb_ui_render_text(state, w.title, text_x, text_y);
    }
    clear_window(state, window);
    state->mouse.l_released = false;
    state->mouse.r_released = false;
}

