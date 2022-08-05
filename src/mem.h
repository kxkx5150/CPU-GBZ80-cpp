#ifndef _H_MEM_
#define _H_MEM_
#include <cstdint>
#include <string>
#include <vector>
#include "rom.h"
#include "io.h"

class GB;
class Mem {
  public:
    uint64_t memsize = 0x10000;

    GB      *core = nullptr;
    Rom     *rom[4];
    Gamepad *pad = nullptr;

    uint8_t ram[0x10000];
    uint8_t cartRAM[0x8000];

    int ROMbank       = 1;
    int ROMbankoffset = (ROMbank - 1) * 0x4000;
    int RAMbank       = 0;
    int RAMbankoffset = RAMbank * 0x2000 - 0xa000;
    int RAMenabled    = false;
    int MBCRamMode    = 0;

    int joypad_dpad    = 0xef;
    int joypad_buttons = 0xdf;
    int keys_dpad      = 0xef;
    int keys_buttons   = 0xdf;

  public:
    Mem(GB *_gb, Rom *_rom, Gamepad *_pad);

    ~Mem();

    void init();
    int  read(int addr);
    void write(int addr, int data);
    int  readMemWord(int addr);
    void writeMemWord(int addr, int data);
    void doMBC(uint64_t addr, uint64_t data);

    void reset();
};
#endif