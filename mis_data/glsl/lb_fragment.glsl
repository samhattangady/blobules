#version 330 core
in vec3 fragCoord;
in vec3 texCoord;

layout(location = 0) out vec4 fragColor;
uniform sampler2D spritesheet;
uniform sampler2D sdfsheet;
// mode 1 is for the sdf rendering to texture
// mode 2 is for the rendering to screen
uniform int mode;

uniform float time;
uniform float opacity;
float PI = 3.141596;
float STROKE_THICKNESS = 4.0;
vec4 bg = vec4(0.94, 0.94, 0.92, 1.0);

void main() {
    vec2 tx = texCoord.xy;
    vec4 col = vec4(0.0);
    if (mode == 1) {
        col.a = 1.0;
        col.b = 0.0;
        if (texCoord.z < 1.5) { // circle
            float dist = (0.5 - distance(tx, vec2(0.5))) * 2.0;
            col.r = dist;
            col.a = dist;
        } else if (texCoord.z < 2.5) { // level connection
            col.g = fragCoord.z;
        } else if (texCoord.z < 3.5) { // level connection
            // for lines to be color of paper.
            col.b = 1.0;
        }
    } else if (mode == 2) {
        col = texture2D(spritesheet, tx);
        vec4 sdf = texture2D(sdfsheet, texCoord.xy);
        col.a *= smoothstep(0.0, 0.5, sdf.r);
        col = mix(col,vec4(0.1, 0.1, 0.1, 1.0), sdf.g);
        col = mix(col, bg, sdf.b);
    } else if (mode == 3) {
        col = texture2D(spritesheet, tx);
        vec4 sdf = texture2D(sdfsheet, texCoord.xy);
        col.a *= smoothstep(0.0, 0.5, sdf.r);
        col = mix(col,vec4(0.1, 0.1, 0.1, 1.0), sdf.g);
        col = mix(col, bg, sdf.b);
        // TODO (23 Aug 2020 sam: Figure out how to blur here instead of reducing alpha
        col.a *= 0.5;
    }
    fragColor = col;
    return;
}
