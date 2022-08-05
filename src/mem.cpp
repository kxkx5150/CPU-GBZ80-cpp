#include "mem.h"
#include <cstdint>
#include "GB.h"
#include "io.h"

Mem::Mem(GB *_gb, Rom *_rom, Gamepad *_pad)
{
    core   = _gb;
    rom[0] = _rom;
    pad    = _pad;
    reset();
}
Mem::~Mem()
{
}
void Mem::init()
{
    ROMbank       = 1;
    ROMbankoffset = (ROMbank - 1) * 0x4000;
    RAMbank       = 0;
    RAMbankoffset = RAMbank * 0x2000 - 0xa000;
    RAMenabled    = false;
    MBCRamMode    = 0;
    ram[0xff41]   = 1;
    ram[0xff43]   = 0;
}
int Mem::read(int addr)
{
    if (addr >= 0xff10 && addr <= 0xff26) {
        return 0;    // Sound.readMem(addr);
    }
    if (addr >= 0xff30 && addr <= 0xff3f) {
        return 0;    // Sound.readMem(addr);
    }

    if (addr <= 0x3fff)
        return rom[0]->bin[addr];
    if (addr <= 0x7fff)
        return rom[0]->bin[addr + ROMbankoffset];
    if (addr >= 0xa000 && addr <= 0xbfff) {
        return cartRAM[addr + RAMbankoffset];
    }
    if (addr == 0xff00) {
        if (ram[0xff00] & 0x20) {
            return pad->keys_dpad;
        } else if (ram[0xff00] & 0x10) {
            return pad->keys_buttons;
        } else
            return 0xff;
    }
    return ram[addr];
}
void Mem::write(int addr, int data)
{
    if (addr >= 0xff10 && addr <= 0xff26) {
        // Sound.soundMapper(addr, data);
        return;
    }
    if (addr >= 0xff30 && addr <= 0xff3f) {
        // Sound.nodes[3].waveChanged = true;
        // Sound.MEM2[addr]           = data;
        return;
    }

    if (addr <= 0x7fff) {
        doMBC(addr, data);
        return;
    }
    if (addr >= 0xa000 && addr <= 0xbfff && RAMenabled) {
        cartRAM[addr + RAMbankoffset] = data;
        // storeData(cartRAM)
        return;
    }

    if (addr == 0xff04) {
        ram[0xff04] = 0;
        return;
    }

    if (addr == 0xff07) {
        core->timerEnable    = (data & (1 << 2)) != 0;
        core->timerLength    = core->timerLenTable[data & 0x3];
        core->timerPrescaler = core->timerLength;
        ram[addr]            = 0xf8 | data;
        return;
    }

    if (addr == 0xff40) {
        uint64_t cc = data & (1 << 7);
        if (core->LCD_enabled != cc) {
            core->LCD_enabled = !!cc;
            if (!core->LCD_enabled) {
                core->LCD_scan = 0;
                ram[0xff41]    = (ram[0xff41] & 0xfc) + 1;
            }
        }
    }
    if (addr == 0xff41) {
        ram[0xff41] &= 0x3;
        data &= 0xfc;
        ram[0xff41] |= 0x80 | data;
        return;
    }

    if (addr == 0xff44) {
        ram[0xff44] = 0;
        return;
    }

    if (addr == 0xff46) {
        uint64_t st = data << 8;
        for (int i = 0; i <= 0x9f; i++)
            ram[0xfe00 + i] = read(st + i);
        return;
    }

    if (addr == 0xff50) {
        for (int i = 0; i < 256; i++)
            rom[0]->bin[i] = rom[0]->FirstROMPage[i];
        return;
    }

    ram[addr] = data;
}
void Mem::doMBC(uint64_t addr, uint64_t data)
{
    switch (rom[0]->bin[0x147]) {
        case 0:
            break;
        case 0x01:
        case 0x02:
        case 0x03:
            if (addr <= 0x1fff) {
                RAMenabled = (data & 0x0f) == 0xa;
            } else if (addr <= 0x3fff) {
                data &= 0x1f;
                if (data == 0)
                    data = 1;

                ROMbank       = (ROMbank & 0xe0) | (data & 0x1f);
                ROMbankoffset = ((ROMbank - 1) * 0x4000) % rom[0]->romlen;
            } else if (addr <= 0x5fff) {
                data &= 0x3;
                if (MBCRamMode == 0) {
                    ROMbank       = (ROMbank & 0x1f) | (data << 5);
                    ROMbankoffset = ((ROMbank - 1) * 0x4000) % rom[0]->romlen;
                } else {
                    RAMbank       = data;
                    RAMbankoffset = RAMbank * 0x2000 - 0xa000;
                }
            } else {
                MBCRamMode = data & 1;
                if (MBCRamMode == 0) {
                    RAMbank       = 0;
                    RAMbankoffset = RAMbank * 0x2000 - 0xa000;
                } else {
                    ROMbank &= 0x1f;
                    ROMbankoffset = ((ROMbank - 1) * 0x4000) % rom[0]->romlen;
                }
            }

            break;
        case 0x05:
        case 0x06:
            if (addr <= 0x1fff) {
                if ((addr & 0x0100) == 0)
                    RAMenabled = (data & 0x0f) == 0xa;
            } else if (addr <= 0x3fff) {
                data &= 0x0f;
                if (data == 0)
                    data = 1;
                ROMbank       = data;
                ROMbankoffset = ((ROMbank - 1) * 0x4000) % rom[0]->romlen;
            }
            break;

        case 0x11:
        case 0x12:
        case 0x13:
            if (addr <= 0x1fff) {
                RAMenabled = (data & 0x0f) == 0xa;
            } else if (addr <= 0x3fff) {
                if (data == 0)
                    data = 1;
                ROMbank       = data & 0x7f;
                ROMbankoffset = ((ROMbank - 1) * 0x4000) % rom[0]->romlen;
            } else if (addr <= 0x5fff) {
                if (data < 8) {
                    RAMbank       = data;
                    RAMbankoffset = RAMbank * 0x2000 - 0xa000;
                } else {
                }
            } else {
            }
            break;
        case 0x19:
        case 0x1a:
        case 0x1b:
            if (addr <= 0x1fff) {
                RAMenabled = (data & 0x0f) == 0xa;
            } else if (addr <= 0x2fff) {
                ROMbank &= 0x100;
                ROMbank |= data;
                ROMbankoffset = (ROMbank - 1) * 0x4000;
                while (ROMbankoffset > rom[0]->romlen)
                    ROMbankoffset -= rom[0]->romlen;
            } else if (addr <= 0x3fff) {
                ROMbank &= 0xff;
                if (data & 1)
                    ROMbank += 0x100;
                ROMbankoffset = (ROMbank - 1) * 0x4000;
                while (ROMbankoffset > rom[0]->romlen)
                    ROMbankoffset -= rom[0]->romlen;
            } else if (addr <= 0x5fff) {
                RAMbank       = data & 0x0f;
                RAMbankoffset = RAMbank * 0x2000 - 0xa000;
            }
            break;

        default:
            throw "Unimplemented memory controller";
    }
}

int Mem::readMemWord(int addr)
{
    uint64_t hval = read(addr + 1);
    uint64_t lval = read(addr);
    return (hval << 8) | lval;
}
void Mem::writeMemWord(int addr, int value)
{
    write(addr, value & 0xff);
    write(addr + 1, value >> 8);
}
void Mem::reset()
{
    for (int i = 0; i < memsize; i++) {
        ram[i] = 0x00;
    }
    for (int i = 0; i < 0x8000; i++) {
        cartRAM[i] = 0x00;
    }
}
