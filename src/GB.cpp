#include "GB.h"
#include "disp.h"
#include "io.h"
#include "mem.h"
#include "rom.h"
#include <cstdint>
#include <cstdio>
#include <thread>
#include <chrono>

GB::GB()
{
}
GB::~GB()
{
    delete cpu;
    delete mem;
    delete rom;
    delete disp;
    delete pad;
}
void GB::init()
{
    load("Arkanoid (Bung) (PD) [C].gbc");

    pad = new Gamepad();
    mem = new Mem(this, rom, pad);
    cpu = new CPU();
    cpu->init(mem);
    disp = new Disp(this);

    printf("init\n");
}
void GB::load(std::string path)
{
    paused = true;

    FILE *f = fopen(path.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    const int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *rombin = new uint8_t[size];
    auto     _      = fread(rombin, size, 1, f);
    rom             = new Rom(rombin, size);
    fclose(f);

    interrupt_enable = 0;
    interrupt_flags  = 0;
    timer_running    = 1;
}
void GB::start(SDL_Renderer *_renderer, SDL_Texture *_MooseTexture, int _width, int _height)
{
    mem->reset();
    cpu->reset();
    mem->init();

    width  = _width;
    height = _height;

    renderer     = _renderer;
    MooseTexture = _MooseTexture;
}
void GB::run_cpu()
{
    int cycles = 4;
    if (!cpu->halted) {
        cycles = cpu->run();
    }

    if ((divPrescaler += cycles) > 255) {
        divPrescaler -= 256;
        mem->ram[0xff04]++;
    }

    if (timerEnable) {
        timerPrescaler -= cycles;
        while (timerPrescaler < 0) {
            timerPrescaler += timerLength;
            if (mem->ram[0xff05]++ == 0xff) {
                mem->ram[0xff05] = mem->ram[0xff06];
                mem->ram[0xff0f] |= 1 << 2;
                cpu->halted = false;
            }
        }
    }

    if (cpu->irq_flg) {
        int i = mem->ram[0xff0f] & mem->ram[0xffff];
        if (i & (1 << 0)) {
            int ival         = mem->ram[0xff0f] & ~(1 << 0);
            mem->ram[0xff0f] = ival;
            cycles += triggerInterrupt(0x40);
        } else if (i & (1 << 1)) {
            mem->ram[0xff0f] &= ~(1 << 1);
            cycles += triggerInterrupt(0x48);
        } else if (i & (1 << 2)) {
            mem->ram[0xff0f] &= ~(1 << 2);
            cycles += triggerInterrupt(0x50);
        } else if (i & (1 << 3)) {
            mem->ram[0xff0f] &= ~(1 << 3);
            cycles += triggerInterrupt(0x58);
        } else if (i & (1 << 4)) {
            mem->ram[0xff0f] &= ~(1 << 4);
            cycles += triggerInterrupt(0x60);
        }
    }
    disp->run(cycles);
    cpu->num_cycles = 0;
}
int GB::triggerInterrupt(int vector)
{
    cpu->halted = false;
    mem->writeMemWord((cpu->reg_SP -= 2), cpu->reg_PC);
    cpu->reg_PC  = vector;
    cpu->irq_flg = false;
    return 20;
}
void GB::raise_interrupt(int val)
{
    if (val == 0x48)
        mem->ram[0xff0f] |= 1 << 1;
    else if (val == 0x40)
        mem->ram[0xff0f] |= 1 << 0;
}
void GB::put_image(uint32_t *imgdata)
{
    int64_t start = SDL_GetPerformanceCounter();

    UpdateTexture(MooseTexture, imgdata, width, height);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, MooseTexture, NULL, NULL);
    SDL_RenderPresent(renderer);

    Uint64 end       = SDL_GetPerformanceCounter();
    float  elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
    if (16.666f > elapsedMS)
        SDL_Delay(floor(16.666f - elapsedMS));
}
void GB::UpdateTexture(SDL_Texture *texture, uint32_t *imgdata, int width, int height)
{
    size_t     imgidx = 0;
    SDL_Color *color;
    Uint8     *src;
    Uint32    *dst;
    int        row, col;
    void      *pixels;
    int        pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    for (row = 0; row < height; ++row) {
        dst = (Uint32 *)((Uint8 *)pixels + row * pitch);
        for (col = 0; col < width; ++col) {
            *dst++ = imgdata[imgidx++];
        }
    }
    SDL_UnlockTexture(texture);
}
void GB::hit_stop_instruction()
{
}
