#version 330 core
in vec3 fragCoord;
in vec3 texCoord;

out vec4 fragColor;
uniform sampler2D spritesheet;

uniform float time;
float PI = 3.141596;
float NUMBER_OF_SPRITES = 8.0;
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
        return vec4(1.0, 1.0, 1.0, 0.00);
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
    if (material == 11)   // reflector
        return vec4(0.1, 0.4, 0.7, 1.0);
}

int get_rand_0_to_3() {
    float r = rand(vec2(time));
    r*= 3.0;
    r+= 0.5;
    return int(r);
}

void main() {
    float noise = pNoise(gl_FragCoord.xy, 3, 300.3);
    // if (noise > 0.5)
    //     fragColor = vec4(0.0, 0.0, 0.3, 1.0);
    // else
    //     fragColor = vec4(0.3, 0.0, 0.3, 1.0);
    // fragColor = vec4(vec3(noise), 1.0);
    // return;
    vec4 colour = get_color(int(fragCoord.z));
    if (int(fragCoord.z) == 1) {
        vec2 tex = texCoord.xy;
        tex += 0.10*vec2(pNoise(gl_FragCoord.xy,3, 50.3), pNoise(gl_FragCoord.yx,3, 50.3));
        vec4 col = texture(spritesheet, vec2((0.0/NUMBER_OF_SPRITES)+(tex.x/NUMBER_OF_SPRITES), tex.y));
        // fragColor = col;
        if (col.w > 0.9) {
            float pixel_distance = col.x - 0.5;
            pixel_distance *= 255.0;
            // pixel_distance += 2.5 * (noise-0.2);
            float pencil = smoothstep(1.2, 15.8, pixel_distance);
            pencil = pow(pencil, 0.01);
            vec3 colour = vec3(0.0, 0.04, 0.1);
            float alpha = mix(1.0, 0.0, pencil);
            fragColor = vec4(colour, alpha);
        } else
            fragColor = vec4(0.0);
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
    fragColor = colour;
    // fragColor = vec4(vec3(noise), 1.0);
}
