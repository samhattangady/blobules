
vec3 calcNormal(vec3 p) {
    // We calculate the normal by finding the gradient of the field at the
    // point that we are interested in. We can find the gradient by getting
    // the difference in field at that point and a point slighttly away from it.
    const float h = 0.0001;
    return normalize( vec3(
        			       -distanceField(p).x+ distanceField(p+vec3(h,0.0,0.0)).x,
                           -distanceField(p).x+ distanceField(p+vec3(0.0,h,0.0)).x,
                           -distanceField(p).x+ distanceField(p+vec3(0.0,0.0,h)).x
    				 ));
}

vec4 raymarch(vec3 direction, vec3 start) {
    // We need to cast out a ray in the given direction, and see which is
    // the closest object that we hit. We then move forward by that distance,
    // and continue the same process. We terminate when we hit an object
    // (distance is very small) or at some predefined distance.
    float far = 550.0;
    vec3 pos = start;
    float d = 0.0;
    vec4 obj = vec4(0.0, 0.0, 0.0, 0.0);
    for (int i=0; i<100; i++) {
    	obj = distanceField(pos);
        float dist = obj.x;
        pos += dist*direction;
        d += dist;
        if (dist < 0.01) {
        	break;
        }
        if (d > far) {
        	break;
        }
    }
    return vec4(d, obj.yzw);
}

void main() {
    vec2 pos = fragCoord;
    pos.y *= ybyx;

    vec3 cameraPosition = vec3(0.0) + rotate3D(vec3(0.0, 11.7, -15.0), vec3(0.0, 0.0, 0.0));
    vec3 planePosition = rotate3D(vec3(pos, 1.0), vec3(-0.6, 0.0, 0.0)) + cameraPosition;

    float xRotate = 0.0;
    float yRotate = 0.0;
    float zRotate = 0.0;

    // TODO (02 Apr 2020 sam): Figure out the camera orientation stuff here
    // cameraPosition = ...
    // planePosition = ...

    vec3 lookingDirection = (planePosition - cameraPosition);

    vec3 lightFacing = vec3(1.0, 1.0, -0.3) - vec3(0.0);

    // raymarch to check for colissions.
    vec4 obj = raymarch(lookingDirection, planePosition);
    float dist = obj.x;
    vec3 color = vec3(0.01);
    if (dist < 550.0) {
        vec3 world_pos = planePosition+ dist*lookingDirection;
        vec3 normal = calcNormal(world_pos);
        float light = dot(lightFacing, normal);
        light = max(light, 0.0);
        if (obj.y < 1.5) {
            // player
        	color = vec3(0.205, 0.205, 0.505);
            color += 0.3* smoothstep(0.1, 1.0, light);
        } else if (obj.y < 2.5) {
            // icecube
        	color = vec3(0.75, 0.75, 0.85);
            color += 0.3* smoothstep(0.1, 1.0, light);
        } else if (obj.y < 3.5) {
            // blocker
        	color = vec3(0.35, 0.35, 0.35);
            color += 0.3* smoothstep(0.1, 1.0, light);
        } else if (obj.y < 4.5) {
            // ground
        	color = vec3(0.35, 0.75, 0.35);
            color += 0.3* smoothstep(0.1, 1.0, light);
        } else if (obj.y < 5.5) {
            // none
        }
    }
    // gamma correction
    color = pow( color, vec3(1.0/2.2) );
    fragColor = vec4(color,1.0);
}
