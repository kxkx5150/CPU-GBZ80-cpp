#ifndef _H_IO_
#define _H_IO_
#include <sys/types.h>
#include <SDL2/SDL.h>

class Gamepad {
  public:
    u_int8_t keys_dpad = 0xef, keys_buttons = 0xdf;

    void keyDown(int player, int button)
    {
        bool flg = true;
        if (player == 1) {
            switch (button) {
                case SDLK_a:    // a
                    keys_buttons &= ~(1 << 0);
                    break;
                case SDLK_z:    // b
                    keys_buttons &= ~(1 << 1);
                    break;
                case SDLK_x:    // select
                    keys_buttons &= ~(1 << 2);
                    break;
                case SDLK_s:    // start
                    keys_buttons &= ~(1 << 3);
                    break;
                case SDLK_UP:    // up
                    keys_dpad &= ~(1 << 2);
                    break;
                case SDLK_DOWN:    // down
                    keys_dpad &= ~(1 << 3);
                    break;
                case SDLK_LEFT:    // left
                    keys_dpad &= ~(1 << 1);
                    break;
                case SDLK_RIGHT:    // right
                    keys_dpad &= ~(1 << 0);
                    break;
                default:
                    flg = false;
                    break;
            }
        }
    }

    void keyUp(int player, int button)
    {
        if (player == 1) {
            switch (button) {
                case SDLK_a:    // a
                    keys_buttons |= 1 << 0;
                    break;
                case SDLK_z:    // b
                    keys_buttons |= 1 << 1;
                    break;
                case SDLK_x:    // select
                    keys_buttons |= 1 << 2;
                    break;
                case SDLK_s:    // start
                    keys_buttons |= 1 << 3;
                    break;
                case SDLK_UP:    // up
                    keys_dpad |= 1 << 2;
                    break;
                case SDLK_DOWN:    // down
                    keys_dpad |= 1 << 3;
                    break;
                case SDLK_LEFT:    // left
                    keys_dpad |= 1 << 1;
                    break;
                case SDLK_RIGHT:    // right
                    keys_dpad |= 1 << 0;
                    break;
            }
        }
    }
};
#endif
