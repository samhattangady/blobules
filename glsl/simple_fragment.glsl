#version 330 core
in vec3 fragCoord;
out vec4 fragColor;

uniform float time;

vec4 get_color(int material) {
    if (material == 1)   // player
        return vec4(0.6, 0.3, 0.3, 1.0);
    if (material == 2)   // icecube
        return vec4(0.7, 0.7, 0.8, 1.0);
    if (material == 3)   // blocker
        return vec4(0.2, 0.2, 0.2, 1.0);
    if (material == 4)   // ground
        return vec4(0.3, 0.6, 0.5, 1.0);
    if (material == 5)   // none
        return vec4(0.1, 0.1, 0.1, 0.0);
    if (material == 6)   // slippery
        return vec4(0.4, 0.4, 0.6, 1.0);
    if (material == 7)   // hot target
        return vec4(0.7 + 0.05 * sin(time*8.0), 0.4 + 0.05*sin(time*8.0), 0.2, 1.0);
    if (material == 8)   // cold target
        return vec4(0.7, 0.7, 0.4, 1.0);
    if (material == 9)   // furniture
        return vec4(0.9, 0.7, 0.4, 1.0);
}

void main() {
    fragColor = get_color(int(fragCoord.z));
}
