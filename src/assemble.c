#include <math.h>
#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_rect_pack.h"
#include "stb_image_write.h"

#define u8 uint8_t
#define TOTAL_NUMBER 17
#define NUMBER_OF_NODES 2048
#define SPRITE_DATA_FILE "static/sprite_data.txt"

int main(int argc, char** argv) {
    char* filenames[TOTAL_NUMBER];
    unsigned char* fill_data[TOTAL_NUMBER];
    stbrp_rect rects[TOTAL_NUMBER];
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
    filenames[12] = "static/ground5";
    filenames[13] = "static/ground6";
    filenames[14] = "static/player_push";
    filenames[15] = "static/player_jump";
    filenames[16] = "static/player_slip";

    for (int i=0; i<TOTAL_NUMBER; i++) {
        char fillname[50];
        sprintf(fillname, "%s_Fill.png", filenames[i]);
        rects[i].id = i;
        fill_data[i] = stbi_load(fillname, &rects[i].w, &rects[i].h, &n, 0);
    }
    int total_width = 0;
    int total_height = 0;
    for (int i=0; i<TOTAL_NUMBER; i++) {
        total_width += rects[i].w;
        total_height = max(total_height, rects[i].h);
    }
    // FIXME (22 Aug 2020 sam): manually reducing width so that we can ensure the packing is working
    total_width -= 560;
    // pack the rects
    // TODO (22 Aug 2020 sam): The output should be using the optimal size after packing
    stbrp_context packing_context;
    stbrp_node* nodes = (stbrp_node*) malloc(sizeof(stbrp_node)*NUMBER_OF_NODES);
    stbrp_init_target(&packing_context, total_width, total_height, nodes, NUMBER_OF_NODES);
    stbrp_setup_allow_out_of_mem(&packing_context, 1);
    if (stbrp_pack_rects(&packing_context, rects, TOTAL_NUMBER) == 0) {
        printf("failed to pack rects. quitting.\n");
        return 0;
    }
    // create image
    u8* fill_output = (u8*) calloc(total_width*total_height*4, sizeof(u8));
    for (int i=0; i<TOTAL_NUMBER; i++) {
        for (int x=0; x<rects[i].w; x++) {
            for (int y=0; y<rects[i].h; y++) {
                int packed_x = rects[i].x;
                int packed_y = rects[i].y;
                for (int j=0; j<4; j++) {
                    fill_output[((packed_y+y)*total_width + packed_x+x)*4 + j] = fill_data[i][(y*rects[i].w+x)*4 + j];
                }
            }
        }
    }
    stbi_write_png("static/fillsheet.png", total_width, total_height, 4, fill_output, 0);
    FILE* sprite_data = fopen(SPRITE_DATA_FILE, "w");
    fprintf(sprite_data, "%i\n", TOTAL_NUMBER);
    for (int i=0; i<TOTAL_NUMBER; i++) {
        // w h tx1 ty1 tx2 ty2
        stbrp_rect rect = rects[i];
        double x1 = 1.0 * rect.x / total_width;
        // this is because data is saved upside down compared to opengl loader.
        double y1 = 1.0 * (total_height - (rect.y + rect.h)) / total_height;
        double x2 = 1.0 * (rect.x + rect.w) / total_width;
        double y2 = 1.0 * (total_height-rect.y) / total_height;
        fprintf(sprite_data, "%i %i %lf %lf %lf %lf\n", rect.w, rect.h, x1, y1, x2, y2);
    }
    fclose(sprite_data);

    
    printf("exitting.\n");
    return 0;
}
