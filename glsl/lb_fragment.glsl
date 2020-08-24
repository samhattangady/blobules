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

//	Classic Perlin 2D Noise 
//	by Stefan Gustavson
//
vec2 fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}

float cnoise(vec2 P){
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;
  vec4 i = permute(permute(ix) + iy);
  vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
  vec4 gy = abs(gx) - 0.5;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;
  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);
  vec4 norm = 1.79284291400159 - 0.85373472095314 * 
    vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;
  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));
  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}

// SDF for line segment
// by Inigo Quillez
//
float sdSegment(vec2 p, vec2 a, vec2 b ) {
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

float normpdf(in float x, in float sigma) {
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

void main() {
    vec2 tx = texCoord.xy + 0.0004*cnoise(fragCoord.xy*15.0)*sin(time/3.5);
    tx = tx + 0.008*cnoise(fragCoord.xy*2.4)*sin(time/1.8);
    tx = texCoord.xy;
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
