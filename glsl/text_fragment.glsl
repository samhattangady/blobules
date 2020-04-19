#version 330 core
in vec2 text_coords;
out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;
uniform int mode;

void main() {
    if (mode == 1) {
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, text_coords).r);
        color = vec4(textColor) * sampled;
    } else if (mode == 2) {
        color = vec4(textColor);    
    }
}
