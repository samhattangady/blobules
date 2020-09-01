#version 330 core
layout (location=0) in vec4 position;
out vec2 fragCoord;
out vec2 textCoord;

uniform float time;

void main() {
    fragCoord = position.xy;
    textCoord = position.zw;
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}
