#version 330 core
in vec2 fragCoord;
in vec2 textCoord;

out vec4 fragColor;
uniform sampler2D body;
uniform sampler2D shad;
uniform sampler2D line;
uniform vec2 size;
int MAX_PIXEL_DISTANCE = 127;
float MIN_ALPHA = 0.7;

int pixel_inside(sampler2D img, int x, int y) {
    if (x<0 || y<0 || x>=size.x || y>=size.y) {
        return 0;
    }
    vec4 p = texture(img, vec2(x/size.x, y/size.y));   
    if (p.a > MIN_ALPHA) {
        return 1;
    }
    return 0;
}

ivec2 update_point_count(ivec2 p, int inside) {
    if (inside==1)
        p[0] += 1;
    else
        p[1] += 1;
    return p;
}

ivec2 get_points_at_radius(sampler2D img, int xc, int yc, int r) {
    int x = r;
    int y = 0;
    int err = 0;
    ivec2 p = ivec2(0);
    while (x >= y) {
        p = update_point_count(p, pixel_inside(img, xc+x, yc+y)); 
        p = update_point_count(p, pixel_inside(img, xc+y, yc+x)); 
        p = update_point_count(p, pixel_inside(img, xc-x, yc+y)); 
        p = update_point_count(p, pixel_inside(img, xc-y, yc+x)); 
        p = update_point_count(p, pixel_inside(img, xc+x, yc-y)); 
        p = update_point_count(p, pixel_inside(img, xc+y, yc-x)); 
        p = update_point_count(p, pixel_inside(img, xc-x, yc-y)); 
        p = update_point_count(p, pixel_inside(img, xc-y, yc-x)); 
        if (err <= 0) {
            y += 1;
            err += 2*y - 1;
        } else {
            x -= 1;
            err -= 2*x + 1;
        }
    }
    return p;
}

void main() {
    vec4 body_col = texture(body, textCoord);
    vec4 shad_col = texture(shad, textCoord);
    vec4 line_col = texture(line, textCoord);
    vec4 col = vec4(1.0);
    for (int r=0; r<MAX_PIXEL_DISTANCE; r++) {
        int xc = int(textCoord.x * size.x);
        int yc = int(textCoord.y * size.y);
        ivec2 p = get_points_at_radius(body, xc, yc, r);
        int inside = pixel_inside(body, xc, yc);
        if (inside==1)
            col.r = 0.0;
        if (inside==1 && p[1]>1) {
            col.r = 0.5 - (r/255.0);
            break;
        } else if (inside==0 && p[0]>1) {
            col.r = 0.5 + (r/255.0);
            break;
        }
    }
    for (int r=0; r<MAX_PIXEL_DISTANCE; r++) {
        int xc = int(textCoord.x * size.x);
        int yc = int(textCoord.y * size.y);
        ivec2 p = get_points_at_radius(shad, xc, yc, r);
        int inside = pixel_inside(shad, xc, yc);
        if (inside==1)
            col.g = 0.0;
        if (inside==1 && p[1]>1) {
            col.g = 0.5 - (r/255.0);
            break;
        } else if (inside==0 && p[0]>1) {
            col.g = 0.5 + (r/255.0);
            break;
        }
    }
    for (int r=0; r<MAX_PIXEL_DISTANCE; r++) {
        int xc = int(textCoord.x * size.x);
        int yc = int(textCoord.y * size.y);
        ivec2 p = get_points_at_radius(line, xc, yc, r);
        int inside = pixel_inside(line, xc, yc);
        if (inside==1)
            col.b = 0.0;
        if (inside==1 && p[1]>1) {
            col.b = 0.5 - (r/255.0);
            break;
        } else if (inside==0 && p[0]>1) {
            col.b = 0.5 + (r/255.0);
            break;
        }
    }
    // col.r = 1.0-col.r;
    // col.g = 1.0-col.g;
    // col.b = 1.0-col.b;
    // col.a = col.r;
    fragColor = col;
    return;
}
