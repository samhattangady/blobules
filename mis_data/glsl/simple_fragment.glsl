#version 330 core
in vec3 fragCoord;
in vec3 texCoord;

out vec4 fragColor;
uniform sampler2D spritesheet;
uniform sampler2D sdfsheet;

uniform float time;
uniform float opacity;

void main() {
    vec4 col;
    vec2 tx = texCoord.xy;
    col = texture(spritesheet, tx);
    if (col.a < 0.1)
        discard;
    if (texCoord.z > 0.0) {
        // col.g += 0.02*texCoord.z;
    }
    fragColor = col;
    return;
}
