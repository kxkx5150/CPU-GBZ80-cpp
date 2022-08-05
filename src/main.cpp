#define SDL_MAIN_HANDLED

#include "GB.h"
#include <cstddef>
#include <cstdint>
#include <SDL2/SDL.h>
#include <cstdio>
#include <time.h>
#include <thread>
#include <bitset>

int Running = 1;
int width = 160, height = 144;

int main(int ArgCount, char **Args)
{
    GB *pc = new GB();
    pc->init();

    SDL_Renderer *renderer;
    SDL_Texture  *MooseTexture;
    SDL_bool      done = SDL_FALSE;
    SDL_Window   *window =
        SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * 2, height * 2, SDL_WINDOW_SHOWN);
    renderer     = SDL_CreateRenderer(window, -1, 0);
    MooseTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_RenderSetScale(renderer, 2.0, 2.0);
    SDL_Event event;
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
        exit(EXIT_FAILURE);
    atexit(SDL_Quit);
    pc->disp->imgok = false;

    pc->start(renderer, MooseTexture, width, height);

    size_t  count = 0;
    size_t  i     = 0;
    bool    quit  = false;
    uint8_t pad   = 0;

    while (!quit) {
        i++;
        if (i == count)
            break;

        while (!pc->disp->imgok) {
            pc->run_cpu();
        }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN: {
                    auto keyCode = event.key.keysym.sym;
                    pc->pad->keyDown(1, keyCode);
                } break;
                case SDL_KEYUP: {
                    auto keyCode = event.key.keysym.sym;
                    pc->pad->keyUp(1, keyCode);
                } break;
            }
        }
        pc->disp->imgok = false;
    }
}
// case SDLK_s:    // RUN 'S'
//     UnsetButtonRUN(0);
//     break;
// case SDLK_a:    // SELECT 'A'
//     UnsetButtonSELECT(0);
//     break;
// case SDLK_z:    // SHOT2 'Z'
//     UnsetButtonSHOT2(0);
//     break;
// case SDLK_x:    // SHOT1 'X'
//     UnsetButtonSHOT1(0);
//     break;

//     // case SDLK_v:    // SHOT2 'V'
//     //     UnsetButtonSHOT2(0);
//     //     break;
//     // case SDLK_b:    // SHOT1 'B'
//     //     UnsetButtonSHOT1(0);
//     //     break;

// case SDLK_LEFT:    // LEFT
//     UnsetButtonLEFT(0);
//     break;
// case SDLK_RIGHT:    // RIGHT
//     UnsetButtonRIGHT(0);
//     break;
// case SDLK_DOWN:    // DOWN
//     UnsetButtonDOWN(0);
//     break;
// case SDLK_UP:    // UP
//     UnsetButtonUP(0);
//     break;