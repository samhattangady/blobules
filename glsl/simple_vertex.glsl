#version 330 core
layout (location=0) in vec3 position;
layout (location=1) in vec2 tex;
out vec3 fragCoord;
out vec2 texCoord;

uniform float time;

void main() {
    fragCoord = position;
    texCoord = tex;
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}
