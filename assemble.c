#include <math.h>
#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_rect_pack.h"
#include "stb_image_write.h"

#define u8 uint8_t
#define TOTAL_NUMBER 15

int main(int argc, char** argv) {
    char* filenames[TOTAL_NUMBER];
    unsigned char* fill_data[TOTAL_NUMBER];
    unsigned char* sdf_data[TOTAL_NUMBER];
    int widths[TOTAL_NUMBER];
    int heights[TOTAL_NUMBER];
    int w, h, n;
    filenames[0] = "static/cube";
    filenames[1] = "static/wall";
    filenames[2] = "static/ground";
    filenames[3] = "static/target";
    filenames[4] = "static/target2";
    filenames[5] = "static/furn";
    filenames[6] = "static/player";
    filenames[7] = "static/slippery";
    filenames[8] = "static/ground1";
    filenames[9] = "static/ground2";
    filenames[10] = "static/ground3";
    filenames[11] = "static/ground4";
    filenames[12] = "static/ice1";
    filenames[13] = "static/ice2";
    filenames[14] = "static/ice3";
    for (int i=0; i<TOTAL_NUMBER; i++) {
        char fillname[50];
        char sdfname[50];
        sprintf(fillname, "%s_Fill.png", filenames[i]);
        sprintf(sdfname, "%s_sdf.png", filenames[i]);
        fill_data[i] = stbi_load(fillname, &widths[i], &heights[i], &n, 0);
        sdf_data[i] = stbi_load(sdfname, &widths[i], &heights[i], &n, 0);
    }
    int total_width = 0;
    int total_height = 0;
    int current_x = 0;
    for (int i=0; i<TOTAL_NUMBER; i++) {
        total_width += widths[i];
        total_height = max(total_height, heights[i]);
    }
    u8* fill_output = (u8*) malloc(sizeof(u8) * total_width*total_height*4);
    u8* sdf_output = (u8*) malloc(sizeof(u8) * total_width*total_height*4);
    for (int i=0; i<TOTAL_NUMBER; i++) {
        for (int x=0; x<widths[i]; x++) {
            for (int y=0; y<heights[i]; y++) {
                for (int j=0; j<4; j++) {
                    fill_output[(y*total_width + current_x+x)*4 + j] = fill_data[i][(y*widths[i]+x)*4 + j];
                    sdf_output[(y*total_width + current_x+x)*4 + j] = sdf_data[i][(y*widths[i]+x)*4 + j];
                }
            }
        }
        current_x += widths[i];
    }
    stbi_write_png("static/fillsheet.png", total_width, total_height, 4, fill_output, 0);
    stbi_write_png("static/sdfsheet.png", total_width, total_height, 4, sdf_output, 0);
}
