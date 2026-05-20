#ifndef SPACEINVADERSVRAMDECODER
#define SPACEINVADERSVRAMDECODER
#include <SDL3/SDL.h>
#include <vector>

namespace space_invaders_vram_decoder
{
    std::vector<SDL_FPoint> DecodeTopPixels(const char* data);
    std::vector<SDL_FPoint> DecodeBottomPixels(const char* data);
}
#endif