#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 text_coords;

uniform vec2 window_size;

void main()
{
    vec4 pos = vec4(0.0, 0.0, 0.0, 1.0);
    pos.x = ((vertex.x/window_size.x) * 2.0) - 1.0;
    pos.y = ((vertex.y/window_size.y) * 2.0) - 1.0;
    gl_Position = pos;
    text_coords = vertex.zw;
}  

