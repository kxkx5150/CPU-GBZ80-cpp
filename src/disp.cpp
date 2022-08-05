#include "disp.h"
#include <cstdio>
#include "GB.h"

Disp::Disp(GB *_core)
{
    core = _core;

    for (int d1 = 0; d1 < 256; d1++) {
        for (int d2 = 0; d2 < 256; d2++) {
            pixelDecoder[d1][d2][0] = ((d1 & 128) + 2 * (d2 & 128)) >> 7;
            pixelDecoder[d1][d2][1] = ((d1 & 64) + 2 * (d2 & 64)) >> 6;
            pixelDecoder[d1][d2][2] = ((d1 & 32) + 2 * (d2 & 32)) >> 5;
            pixelDecoder[d1][d2][3] = ((d1 & 16) + 2 * (d2 & 16)) >> 4;
            pixelDecoder[d1][d2][4] = ((d1 & 8) + 2 * (d2 & 8)) >> 3;
            pixelDecoder[d1][d2][5] = ((d1 & 4) + 2 * (d2 & 4)) >> 2;
            pixelDecoder[d1][d2][6] = ((d1 & 2) + 2 * (d2 & 2)) >> 1;
            pixelDecoder[d1][d2][7] = (d1 & 1) + 2 * (d2 & 1);
        }
    }
    printf(" ");
}
Disp::~Disp()
{
}
void Disp::run(int64_t cycles)
{
    if (core->LCD_enabled) {
        core->LCD_scan += cycles;

        int  mode        = 0;
        bool coincidence = false;
        bool draw        = false;

        if (core->LCD_scan <= 80)
            mode = 2;
        else if (core->LCD_scan <= 252)
            mode = 3;
        else if (core->LCD_scan < 456) {
            draw = core->LCD_lastmode != 0;
            mode = 0;
        } else {
            mode = 2;
            core->LCD_scan -= 456;
            core->mem->ram[0xff44]++;
            if (core->mem->ram[0xff44] > 153)
                core->mem->ram[0xff44] = 0;

            coincidence = core->mem->ram[0xff44] == core->mem->ram[0xff45];
        }

        if (core->mem->ram[0xff44] >= 144)
            mode = 1;
        else if (draw) {
            int LY         = core->mem->ram[0xff44];
            int dpy        = LY * 160;
            int drawWindow = core->mem->ram[0xff40] & (1 << 5) && LY >= core->mem->ram[0xff4a];
            int bgStopX    = drawWindow ? core->mem->ram[0xff4b] - 7 : 160;

            int  baseTileOffset;
            bool tileSigned;

            if (core->mem->ram[0xff40] & (1 << 4)) {
                baseTileOffset = 0x8000;
                tileSigned     = false;
            } else {
                baseTileOffset = 0x9000;
                tileSigned     = true;
            }

            int bgpalette[4];
            bgpalette[0] = core->mem->ram[0xff47] & 3;
            bgpalette[1] = (core->mem->ram[0xff47] >> 2) & 3;
            bgpalette[2] = (core->mem->ram[0xff47] >> 4) & 3;
            bgpalette[3] = (core->mem->ram[0xff47] >> 6) & 3;

            int pix[8];
            if (core->mem->ram[0xff40] & 1) {
                int bgTileMapAddr = core->mem->ram[0xff40] & (1 << 3) ? 0x9c00 : 0x9800;
                int x             = core->mem->ram[0xff43] >> 3;
                int xoff          = core->mem->ram[0xff43] & 7;
                int y             = (LY + core->mem->ram[0xff42]) & 0xff;

                bgTileMapAddr += ~~(y / 8) * 32;

                int tileOffset = baseTileOffset + (y & 7) * 2;
                grabTile(core->mem->ram[bgTileMapAddr + x], tileOffset, pix, tileSigned);

                for (int i = 0; i < bgStopX; i++) {
                    dpixels[dpy + i] = bgpalette[pix[xoff++]];
                    if (xoff == 8) {
                        x = (x + 1) & 0x1f;
                        grabTile(core->mem->ram[bgTileMapAddr + x], tileOffset, pix, tileSigned);
                        xoff = 0;
                    }
                }
            }

            if (drawWindow) {
                int wdTileMapAddr = core->mem->ram[0xff40] & (1 << 6) ? 0x9c00 : 0x9800;
                int xoff          = 0;
                int y             = LY - core->mem->ram[0xff4a];
                wdTileMapAddr += ~~(y / 8) * 32;

                int tileOffset = baseTileOffset + (y & 7) * 2;
                grabTile(core->mem->ram[wdTileMapAddr], tileOffset, pix, tileSigned);

                for (int i = std::max(0, bgStopX); i < 160; i++) {
                    dpixels[dpy + i] = bgpalette[pix[xoff++]];
                    if (xoff == 8) {
                        grabTile(core->mem->ram[++wdTileMapAddr], tileOffset, pix, tileSigned);
                        xoff = 0;
                    }
                }
            }

            if (core->mem->ram[0xff40] & 2) {
                int height, tileNumMask;
                if (core->mem->ram[0xff40] & (1 << 2)) {
                    height      = 16;
                    tileNumMask = 0xfe;
                } else {
                    height      = 8;
                    tileNumMask = 0xff;
                }

                int OBP0[4];
                OBP0[0] = 0;
                OBP0[1] = (core->mem->ram[0xff48] >> 2) & 3;
                OBP0[2] = (core->mem->ram[0xff48] >> 4) & 3;
                OBP0[3] = (core->mem->ram[0xff48] >> 6) & 3;

                int OBP1[4];
                OBP1[0] = 0;
                OBP1[1] = (core->mem->ram[0xff49] >> 2) & 3;
                OBP1[2] = (core->mem->ram[0xff49] >> 4) & 3;
                OBP1[3] = (core->mem->ram[0xff49] >> 6) & 3;

                int background = bgpalette[0];

                for (int i = 0xfe9c; i >= 0xfe00; i -= 4) {
                    int ypos = core->mem->ram[i] - 16 + height;

                    if (LY >= ypos - height && LY < ypos) {
                        int tileNum = 0x8000 + (core->mem->ram[i + 2] & tileNumMask) * 16, xpos = core->mem->ram[i + 1],
                            att = core->mem->ram[i + 3];

                        int *palette = att & (1 << 4) ? OBP1 : OBP0;
                        int  behind  = att & (1 << 7);

                        if (att & (1 << 6)) {
                            tileNum += (ypos - LY - 1) * 2;
                        } else {
                            tileNum += (LY - ypos + height) * 2;
                        }

                        int  d1  = core->mem->ram[tileNum];
                        int  d2  = core->mem->ram[tileNum + 1];
                        auto row = pixelDecoder[d1][d2];

                        if (att & (1 << 5)) {
                            if (behind) {
                                for (int j = 0; j < std::min(xpos, 8); j++) {
                                    if (dpixels[dpy + xpos - 1 - j] == background && row[j]) {
                                        dpixels[dpy + xpos - 1 - j] = palette[row[j]];
                                    }
                                }
                            } else {
                                for (int j = 0; j < std::min(xpos, 8); j++) {
                                    if (row[j]) {
                                        dpixels[dpy + xpos - (j + 1)] = palette[row[j]];
                                    }
                                }
                            }
                        } else {
                            if (behind) {
                                for (int j = std::max(8 - xpos, 0); j < 8; j++) {
                                    if (dpixels[dpy + xpos - 8 + j] == background && row[j]) {
                                        dpixels[dpy + xpos - 8 + j] = palette[row[j]];
                                    }
                                }
                            } else {
                                for (int j = std::max(8 - xpos, 0); j < 8; j++) {
                                    if (row[j]) {
                                        dpixels[dpy + xpos - 8 + j] = palette[row[j]];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (coincidence) {
            if (core->mem->ram[0xff41] & (1 << 6)) {
                core->mem->ram[0xff0f] |= 1 << 1;
                core->mem->ram[0xff41] |= 1 << 2;
            }
        } else
            core->mem->ram[0xff41] &= 0xfb;

        if (core->LCD_lastmode != mode) {
            if (mode == 0) {
                if (core->mem->ram[0xff41] & (1 << 3))
                    core->mem->ram[0xff0f] |= 1 << 1;
            } else if (mode == 1) {
                if (core->mem->ram[0xff41] & (1 << 4))
                    core->mem->ram[0xff0f] |= 1 << 1;

                if (core->mem->ram[0xffff] & 1)
                    core->mem->ram[0xff0f] |= 1 << 0;

                renderDisplayCanvas();
            } else if (mode == 2) {
                if (core->mem->ram[0xff41] & (1 << 5))
                    core->mem->ram[0xff0f] |= 1 << 1;
            }

            core->mem->ram[0xff41] &= 0xf8;
            core->mem->ram[0xff41] += mode;
            core->LCD_lastmode = mode;
        }
    }
}
void Disp::grabTile(int n, int offset, int pary[], bool tileSigned)
{
    int tileptr = 0;
    if (tileSigned && n > 127) {
        tileptr = offset + (n - 256) * 16;
    } else {
        tileptr = offset + n * 16;
    }
    int d1 = core->mem->ram[tileptr], d2 = core->mem->ram[tileptr + 1];

    for (int i = 0; i < 8; i++) {
        pary[i] = pixelDecoder[d1][d2][i];
    }
    return;
}
void Disp::renderDisplayCanvas()
{
    int R[4] = {224, 136, 52, 8};
    int G[4] = {248, 192, 104, 24};
    int B[4] = {208, 112, 86, 32};

    for (int i = 0, j = 0; i < 160 * 144; i++) {
        uint32_t dots   = (255 << 24) | (R[dpixels[i]] << 16) | (G[dpixels[i]] << 8) | B[dpixels[i]];
        imgdata[imgidx] = dots;
        imgidx++;
    }
    imgok  = true;
    imgidx = 0;
    disp_counts++;
    core->put_image(imgdata);
    if (disp_counts == 111) {
        printf(" ");
    }
}
