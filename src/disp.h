#ifndef _H_DISP_
#define _H_DISP_
#include <cstdint>
#include <string>
#include <vector>

class GB;
class Disp {
  public:
    GB *core = nullptr;

    int pixelDecoder[256][256][8];
    int dpixels[160 * 144];

    uint32_t imgdata[160 * 144]{};
    int64_t  imgidx = 0;
    bool     imgok  = false;

    int disp_counts = 0;

  public:
    Disp(GB *core);
    ~Disp();

    void run(int64_t cycles);
    void grabTile(int n, int offset, int pary[], bool tileSigned);
    void renderDisplayCanvas();
};
#endif