/*
 * So this is a tool that takes in the colour image, shadow layer and outline layer, and
 * outputs a single png image that is used for creating the watercolour effects that we
 * are looking for.
 *
 * All the inputs need to be RGBA images of the same size. We will just be checking the
 * alpha values to get the distance. Basically, an alpha value greater than MIN_ALPHA will be
 * considered as part of the shape, we will be finding distance from those pixels.
 *
 * The output has:
 *      the red channel as distance from the colour image
 *      the blue channel as distance from the shadow
 *      the green channel as distance from the outline
 *
 *  Note that we are maintaining the distance field with a 0 value at 128. This is because
 *  we need to store negative values as well. So the distance is an absolute pixel distance
 *  from that pixel to the field that we are building.
 */

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MIN_ALPHA 0.7
#define MAX_PIXEL_DISTANCE 127
#define u8 uint8_t

typedef struct {
    int inside;
    int outside;
} points;

bool pixel_inside(u8* img, int w, int h, int x, int y) {
    if (x<0 || y<0 || x>=w || y>=h)
        return false;
    int index = (y*w) + x;
    u8 alpha = img[index*4 + 3];
    return alpha > (int) (MIN_ALPHA*255);
}

int update_point_count(points* p, bool inside) {
    if (inside)
        p->inside++;
    else
        p->outside++;
    return 0;
}

points get_points_at_radius(u8* img, int w, int h, int xc, int yc, int r) {
    int x = r;
    int y = 0;
    int err = 0;
    points p;
    p.inside = 0;
    p.outside = 0;
    while (x >= y) {
        update_point_count(&p, pixel_inside(img, w, h, xc+x, yc+y));
        update_point_count(&p, pixel_inside(img, w, h, xc+y, yc+x));
        update_point_count(&p, pixel_inside(img, w, h, xc-x, yc+y));
        update_point_count(&p, pixel_inside(img, w, h, xc-y, yc+x));
        update_point_count(&p, pixel_inside(img, w, h, xc+x, yc-y));
        update_point_count(&p, pixel_inside(img, w, h, xc+y, yc-x));
        update_point_count(&p, pixel_inside(img, w, h, xc-x, yc-y));
        update_point_count(&p, pixel_inside(img, w, h, xc-y, yc-x));
        if (err <= 0) {
            y++;
            err += 2*y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2*x + 1;
        }
    }
    return p;
}

int main(int argc, char** argv) {
    clock_t start_time;
    start_time = clock();
    int w, h, c;
    u8 *image  = stbi_load("static/m1.png",&w,&h,&c,0);
    u8 *shadow = stbi_load("static/m2.png",&w,&h,&c,0);
    u8 *line   = stbi_load("static/m3.png",&w,&h,&c,0);
    u8* output = (u8*) calloc(w*h*4, sizeof(u8));
    for (int x=0; x<w; x++) {
        printf("image x=%i\n", x);
        for (int y=0; y<h; y++) {
            int i = (y*w) + x;
            u8 val = 255;
            for (int r=0; r<MAX_PIXEL_DISTANCE; r++) {
                points p = get_points_at_radius(image, w, h, x, y, r);
                bool inside = pixel_inside(image, w, h, x, y);
                if (inside && p.outside>0) {
                    val = 128 - r;
                    break;
                } else if (!inside && p.inside>0) {
                    val = 128 + r;
                    break;
                }
            }
            output[i*4 + 0] = val;
            output[i*4 + 3] = 255;
        }
    }
    for (int x=0; x<w; x++) {
        printf("shadow x=%i\n", x);
        for (int y=0; y<h; y++) {
            int i = (y*w) + x;
            u8 val = 255;
            for (int r=0; r<MAX_PIXEL_DISTANCE; r++) {
                points p = get_points_at_radius(shadow, w, h, x, y, r);
                bool inside = pixel_inside(shadow, w, h, x, y);
                if (inside && p.outside>0) {
                    val = 128 - r;
                    break;
                } else if (!inside && p.inside>0) {
                    val = 128 + r;
                    break;
                }
            }
            output[i*4 + 1] = val;
        }
    }
    for (int x=0; x<w; x++) {
        printf("line x=%i\n", x);
        for (int y=0; y<h; y++) {
            int i = (y*w) + x;
            u8 val = 255;
            for (int r=0; r<MAX_PIXEL_DISTANCE; r++) {
                points p = get_points_at_radius(line, w, h, x, y, r);
                bool inside = pixel_inside(line, w, h, x, y);
                if (inside && p.outside>0) {
                    val = 128 - r;
                    break;
                } else if (!inside && p.inside>0) {
                    val = 128 + r;
                    break;
                }
            }
            output[i*4 + 2] = val;
        }
    }

    stbi_write_png("test_png2.png", w, h, 4, output, 0);
    start_time = clock() - start_time;
    printf("processing complete in %f seconds\n", (double)start_time/CLOCKS_PER_SEC);
    printf("done\n");
    return 0;
}
