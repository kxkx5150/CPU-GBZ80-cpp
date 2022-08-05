#ifndef _H_PC
#define _H_PC

#include "cpu.h"
#include <stdexcept>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstddef>
#include "mem.h"
#include "rom.h"
#include "disp.h"
#include "io.h"

class GB {

  public:
    CPU     *cpu  = nullptr;
    Mem     *mem  = nullptr;
    Rom     *rom  = nullptr;
    Disp    *disp = nullptr;
    Gamepad *pad  = nullptr;

    bool paused           = false;
    int  interrupt_enable = 0;
    int  interrupt_flags  = 0;
    int  timer_running    = 0;

    bool timerEnable      = false;
    int  timerLength      = 1;
    int  timerLenTable[4] = {1024, 16, 64, 256};
    int  timerPrescaler   = 0;

    bool LCD_enabled  = false;
    int  LCD_lastmode = 1;
    int  LCD_scan     = 0;

    int width  = 0;
    int height = 0;

    SDL_Renderer *renderer     = nullptr;
    SDL_Texture  *MooseTexture = nullptr;

    int divPrescaler = 0;

  public:
    GB();
    ~GB();

    void init();
    void load(std::string path);
    void start(SDL_Renderer *_renderer, SDL_Texture *_MooseTexture, int _width, int _height);

    void run_cpu();
    void put_image(uint32_t *imgdata);
    void UpdateTexture(SDL_Texture *texture, uint32_t *imgdata, int width, int height);

    void hit_stop_instruction();
    int  triggerInterrupt(int vector);
    void raise_interrupt(int val);
};
#endif
