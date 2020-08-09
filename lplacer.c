/*
 * This is a tool to help with placing the levels in the world. For the most part, it is just used
 * to get the pixel positions so that we can plan out the positions in krita, and then translate
 * them to coordinates here.
 */
#include "SDL.h"
#include <stdbool.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool quit = false;


int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window;
    window = SDL_CreateWindow("level placer.", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              1000.0, 1000.0, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
    // int width, height, nrChannels;
    // unsigned char *fill_data = stbi_load("static/b3.png",&width,&height,&nrChannels,0);
    //     if (fill_data == NULL) {
    //     SDL_Log("Loading image failed: %s", stbi_failure_reason());
    //     return 1;
    // }
    SDL_Surface* bitmapSurface = SDL_LoadBMP("static/b4.bmp");
    SDL_Texture* bitmapTex = SDL_CreateTextureFromSurface(renderer, bitmapSurface);

    SDL_Event event;
    while (!quit) {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bitmapTex, NULL, NULL);
        SDL_RenderPresent(renderer);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = true;
            if (event.type == SDL_MOUSEBUTTONUP) {
                printf("%f, %f\n", event.button.x*4.0, event.button.y*4.0);
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
