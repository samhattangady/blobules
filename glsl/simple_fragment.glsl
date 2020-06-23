#version 330 core
in vec3 fragCoord;
in vec3 texCoord;

out vec4 fragColor;
uniform sampler2D spritesheet;

uniform float time;
float PI = 3.141596;
float NUMBER_OF_SPRITES = 24.0;
float STROKE_THICKNESS = 4.0;

float rand(vec2 c) {
	return fract(sin(dot(c.xy ,vec2(1.868,7.233))) * 42349.53);
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

float pNoise(vec2 p, int res, float f){
	float persistance = .5;
	float n = 0.;
	float normK = 0.;
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

void main() {
    fragColor = texture(spritesheet, vec2((fragCoord.z/NUMBER_OF_SPRITES)+(texCoord.x/NUMBER_OF_SPRITES), texCoord.y));
    return;
    if (int(fragCoord.z) == 1) {
        fragColor = texture(spritesheet, vec2((0.0/NUMBER_OF_SPRITES)+(texCoord.x/NUMBER_OF_SPRITES), texCoord.y));
        return;
    }
    if (int(fragCoord.z) == 2) {
        fragColor = texture(spritesheet, vec2((1.0/NUMBER_OF_SPRITES)+(texCoord.x/NUMBER_OF_SPRITES), texCoord.y));
        return;
    }
    if (int(fragCoord.z) == 3) {
        fragColor = texture(spritesheet, vec2((2.0/NUMBER_OF_SPRITES)+(texCoord.x/NUMBER_OF_SPRITES), texCoord.y));
        return;
    }
    if (int(fragCoord.z) == 4) {
        fragColor = texture(spritesheet, vec2((3.0/NUMBER_OF_SPRITES)+(texCoord.x/NUMBER_OF_SPRITES), texCoord.y/2.0));
        return;
    }
    if (int(fragCoord.z) == 6) {
        fragColor = texture(spritesheet, vec2((4.0/NUMBER_OF_SPRITES)+(texCoord.x/NUMBER_OF_SPRITES), texCoord.y/2.0));
        return;
    }
    if (int(fragCoord.z) == 7) {
        fragColor = texture(spritesheet, vec2((5.0/NUMBER_OF_SPRITES)+(texCoord.x/NUMBER_OF_SPRITES), texCoord.y));
        return;
    }
    if (int(fragCoord.z) == 8) {
        fragColor = texture(spritesheet, vec2((6.0/NUMBER_OF_SPRITES)+(texCoord.x/NUMBER_OF_SPRITES), texCoord.y));
        return;
    }
    if (int(fragCoord.z) == 9) {
        fragColor = texture(spritesheet, vec2((7.0/NUMBER_OF_SPRITES)+(texCoord.x/NUMBER_OF_SPRITES), texCoord.y));
        return;
    }
    if (int(fragCoord.z) == 11) {
        fragColor = texture(spritesheet, vec2((8.0/NUMBER_OF_SPRITES)+(texCoord.x/NUMBER_OF_SPRITES), texCoord.y));
        return;
    }
}
