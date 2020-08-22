#version 330 core
in vec3 fragCoord;
in vec3 texCoord;

out vec4 fragColor;
uniform sampler2D spritesheet;
uniform sampler2D sdfsheet;

uniform float time;
uniform float opacity;
float PI = 3.141596;
float NUMBER_OF_SPRITES = 15.0;
float STROKE_THICKNESS = 4.0;

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

void main() {
    vec4 col;
    vec2 tx = texCoord.xy;
    int sno = int(texCoord.z);
    if (texCoord.z < -0.2) 
        tx = texCoord.xy;
    else
        tx = vec2((texCoord.z/NUMBER_OF_SPRITES)+(tx.x/NUMBER_OF_SPRITES), tx.y);
    if (sno == 2 || sno == 7 || sno == 8 || sno == 9 || sno == 10 || sno == 11 ||
                    sno == 12 || sno == 13 || sno == 14)
        tx.y = 0.5 + tx.y/2;
    if (texCoord.z < 0.0) 
        col = texture(spritesheet, tx);
    else
        col = texture(spritesheet, tx);
    if (col.a < 0.5)
        discard;
    fragColor = col;
    return;

    /*
    // vec2 tx = texCoord.xy;
    // tx = tx + 0.004*cnoise(fragCoord.xy*15.0)*sin(time/3.5);
    // tx = tx + 0.008*cnoise(fragCoord.xy*2.4)*sin(time/1.8);
    // tx.x = clamp(tx.x, 0.01, 0.99);
    // tx.y = clamp(tx.y, 0.01, 0.99);
    // if (texCoord.z < -0.2) 
    //     tx = texCoord.xy;
    // else
    //     tx = vec2((texCoord.z/NUMBER_OF_SPRITES)+(tx.x/NUMBER_OF_SPRITES), tx.y);
    // int sno = int(texCoord.z);
    // if (sno == 2 || sno == 7 || sno == 8 || sno == 9 || sno == 10 || sno == 11 ||
    //                 sno == 12 || sno == 13 || sno == 14)
    //     tx.y = 0.5 + tx.y/2;
    // vec2 txc;
    // vec4 col = texture(spritesheet, tx);
    // // if (col.a < 0.5) {
    // //     discard;
    // // }
    // vec4 shadow = texture(sdfsheet, tx);
    // shadow.g += 0.02*cnoise(fragCoord.xy*10.0)*sin(time/5.5);
    // float fill_dist = min(shadow.r, shadow.g);
    // fill_dist = min(shadow.b, fill_dist);
    // if (fill_dist > 0.499)
    //     discard;
    // // if (shadow.a > 0.4)
    // //     col = mix(col, vec4(0.0, 0.0, 0.0, 0.22), shadow.a);
    // float shadow_dist = smoothstep(0.6, 0.4, shadow.g);
    // col = mix(col, vec4(0.0, 0.0, 0.0, 1.0), shadow_dist*(0.4));//* 0.06*cnoise(fragCoord.xy*100.0))); 
    // if (opacity > 0.01)
    //     fragColor = vec4(col.xyz, opacity*col.w);
    // col += 0.003*vec4(cnoise(fragCoord.xy*40.0))*sin(time/5.0);
    // col += 0.002*vec4(cnoise(fragCoord.xy*400.0))*sin(time/3.0);
    // col += 0.003*vec4(cnoise(fragCoord.xy*4.0))*sin(time/2.0);
    // col += 0.002*vec4(cnoise(fragCoord.xy*2000.0))*sin(time/4.0);
    // if (col.a >0.9)
    //     col.a = smoothstep(0.65, 0.459, fill_dist);
    // if (fill_dist > 0.32) {
    //     float bleed = min(0.52, fill_dist);
    //     bleed = smoothstep(0.32, 0.499, bleed);
    //     bleed *= bleed*0.5;
    //     bleed -= 0.5;
    //     bleed = sin(bleed*5)/bleed;
    //     col.a -= bleed*0.02;
    // }
    // float line = max(0.0, shadow.b-0.5);
    // line = smoothstep(0.019, 0.0, line) * abs(cnoise(fragCoord.xy*01.0*sin(time/10.0)));
    // col = mix(col, vec4(0.1, 0.1, 0.1, 1.0), line);
    // fragColor = col;
    // return;
    //
    */
}
