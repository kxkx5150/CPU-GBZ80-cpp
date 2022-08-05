#include "rom.h"
#include <cstdint>
#include <cstdio>

Rom::Rom(uint8_t *_bin, uint64_t len)
{
    romlen = len;
    bin    = _bin;
    parse();
}
Rom::~Rom()
{
    delete bin;
}
void Rom::parse()
{
    for (int i = 0; i < 256; i++) {
        FirstROMPage[i] = bin[i];
        bin[i]          = bootCode[i];
    }
    printf(" ");
}
int Rom::read(int addr)
{
    return bin[addr];
}
void Rom::write(int addr, int data)
{
}
