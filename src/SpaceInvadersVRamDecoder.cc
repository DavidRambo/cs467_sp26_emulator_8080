#include <SDL3/SDL.h>
#include <vector>
#include "SpaceInvadersVRamDecoder.h"

namespace SpaceInvadersVRamDecoder
{
    constexpr int MM_COLS = 32;
    constexpr int MM_ROWS = 224;
    constexpr int MM_PIX_PER_BYTE = 8;

    std::vector<SDL_FPoint> DecodeTopPixels(const char* data)
    {
        std::vector<SDL_FPoint> points;

        for (int row = 112; row < MM_ROWS; row++)
        {
            for (int col = 0; col < MM_COLS; col++)
            {
                for (int bit = 0; bit < MM_PIX_PER_BYTE; bit++)
                {
                    bool pixelSet =
                        data[(row * MM_COLS) + col] & (0x01 << bit);
                    if (pixelSet)
                    {
                        SDL_FPoint pixel{ (float)((col * MM_PIX_PER_BYTE) + bit),
                                        (float)(row) };
                        points.push_back(pixel);
                    }
                }
            }
        }
        return points;
    }

    std::vector<SDL_FPoint> DecodeBottomPixels(const char* data)
    {
        std::vector<SDL_FPoint> points;

        for (int row = 0; row < MM_ROWS / 2; row++)
        {
            for (int col = 0; col < MM_COLS; col++)
            {
                for (int bit = 0; bit < MM_PIX_PER_BYTE; bit++)
                {
                    bool pixelSet =
                        data[(row * MM_COLS) + col] & (0x01 << bit);
                    if (pixelSet)
                    {
                        SDL_FPoint pixel{ (float)((col * MM_PIX_PER_BYTE) + bit),
                                        (float)(row) };
                        points.push_back(pixel);
                    }
                }
            }
        }

        return points;
    }
}