#version 330 core
in vec2 fragCoord;
out vec4 fragColor;
uniform float ybyx;
uniform float time;
uniform int number_of_entities;
uniform vec3 entity_positions[128];
uniform int entity_types[128];

float PI = 3.14159;
vec3 rotate3D(vec3 point, vec3 rotation) {
    vec3 r = rotation;
	mat3 rz = mat3(cos(r.z), -sin(r.z), 0,
                   sin(r.z),  cos(r.z), 0,
                   0,         0,        1);
    mat3 ry = mat3( cos(r.y), 0, sin(r.y),
                    0       , 1, 0       ,
                   -sin(r.y), 0, cos(r.y));
    mat3 rx = mat3(1, 0       , 0        ,
                   0, cos(r.x), -sin(r.x),
                   0, sin(r.x),  cos(r.x));
    return rx * ry * rz * point;
}

vec3 moveAndRotate(vec3 position, vec3 center, vec3 rotation) {
    position -= center;
    position = rotate3D(position, rotation);
    return position;
}

float sdfSphere(vec3 position, float radius) {
    return length(position) - radius;
}

float sdfRoundedBox(vec3 position, vec3 box, float radius) {
    vec3 q = abs(position) - box;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - radius;
}

float smin(float d1, float d2, float k) {
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
}

float smax(float d1, float d2, float k) {
    if (k < 0.001) return max(d1, d2);
    float h = max(k-abs(d1-d2),0.0);
    return max(d1, d2) + h*h*0.25/k;
}

vec3 get_entity_size(int type) {
    if (type == 1)   // player
        return vec3(0.1);
    if (type == 2)   // icecube
        return vec3(0.20);
    if (type == 3)   // blocker
        return vec3(0.30);
    if (type == 4)   // ground
        return vec3(0.45);
    if (type == 5)   // none
        return vec3(0.02);
}

float get_entity_material(int type) {
    if (type == 1)   // player
        return 1.0;
    if (type == 2)   // icecube
        return 2.0;
    if (type == 3)   // blocker
        return 3.0;
    if (type == 4)   // ground
        return 4.0;
    if (type == 5)   // none
        return 5.0;
}
