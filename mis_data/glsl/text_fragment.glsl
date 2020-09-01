#version 330 core
in vec2 text_coords;
out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;
uniform int mode;

void main() {
    if (mode == 1) {
        vec4 sampled = vec4(0.2, 0.2, 0.22, texture(text, text_coords).r);
        color = textColor*sampled;
		//color = sampled;
    } else if (mode == 2) {
        color = vec4(textColor);    
    }
}
