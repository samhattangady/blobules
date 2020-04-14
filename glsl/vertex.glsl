#version 330 core
in vec2 position;
out vec2 fragCoord;

void main()
{
    fragCoord = position;
    gl_Position = vec4(position.x, position.y+0.2, 0.0, 1.0);
}
