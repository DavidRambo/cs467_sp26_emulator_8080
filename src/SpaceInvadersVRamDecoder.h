#pragma once
#include <SDL3/SDL.h>
#include <vector>

namespace SpaceInvadersVRamDecoder
{
    std::vector<SDL_FPoint> DecodeTopPixels(const char* data);
    std::vector<SDL_FPoint> DecodeBottomPixels(const char* data);
}