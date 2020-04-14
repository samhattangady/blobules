#version 330 core
in vec3 position;
out vec3 fragCoord;

void main()
{
    fragCoord = position;
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}
