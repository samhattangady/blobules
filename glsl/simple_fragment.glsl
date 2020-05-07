#version 330 core
in vec3 fragCoord;
in vec3 texCoord;

out vec4 fragColor;
uniform sampler2D ground_texture;

uniform float time;
float PI = 3.141596;

float rand(vec2 c) {
	return fract(sin(dot(c.xy ,vec2(15.868,781.233))) * 4378.5453);
}

float noise(vec2 p, float freq ){
	float unit = 1536.0/freq;
	vec2 ij = floor(p/unit);
	vec2 xy = mod(p,unit)/unit;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5*(1.-cos(PI*xy));
	float a = rand((ij+vec2(0.,0.)));
	float b = rand((ij+vec2(1.,0.)));
	float c = rand((ij+vec2(0.,1.)));
	float d = rand((ij+vec2(1.,1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}

float pNoise(vec2 p, int res){
	float persistance = .5;
	float n = 0.;
	float normK = 0.;
	float f = 30.3;
	float amp = 1.;
	int iCount = 0;
	for (int i = 0; i<50; i++){
		n+=amp*noise(p, f);
		f*=2.;
		normK+=amp;
		amp*=persistance;
		if (iCount == res) break;
		iCount++;
	}
	float nf = n/normK;
	return nf*nf*nf*nf;
}

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
        return vec4(1.0, 1.0, 1.0, 0.01);
    if (material == 6)   // slippery
        return vec4(0.4, 0.4, 0.6, 1.0);
    if (material == 7)   // hot target
        return vec4(0.7 + 0.05 * sin(time*8.0), 0.4 + 0.05*sin(time*8.0), 0.2, 1.0);
    if (material == 8)   // cold target
        return vec4(0.7, 0.7, 0.4, 1.0);
    if (material == 9)   // furniture
        return vec4(0.9, 0.7, 0.4, 1.0);
    if (material == 10)   // destroyed target
        return vec4(0.7, 0.0, 0.0, 1.0);
}

int get_rand_0_to_3() {
    float r = rand(vec2(time));
    r*= 3.0;
    r+= 0.5;
    return int(r);
}

void main() {
    vec4 colour = get_color(int(fragCoord.z));
    if (colour.w < 0.1) {
        fragColor = colour;
        return;
    }
    if (int(fragCoord.z) == 1) {
        fragColor = texture(ground_texture, vec2(0.25*texCoord.z+(texCoord.x/4.0), texCoord.y));
        return;
    }
    if (int(fragCoord.z) == 4) {
        fragColor = texture(ground_texture, vec2(texCoord.x/4.0, texCoord.y));
        return;
    }
    float noise = pNoise(gl_FragCoord.xy+vec2(5.0)*fragCoord.z, 3)*(2.0);
    fragColor = vec4(colour.xyz, 1.0);
}
