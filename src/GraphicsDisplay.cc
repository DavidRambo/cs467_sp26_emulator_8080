#include "GraphicsDisplay.h"
#include <iostream>
#include <SDL3/SDL.h>

namespace GraphicsDisplay
{
    GameWindow::GameWindow(int v)
    {
        window = SDL_CreateWindow("Space Invaders", 256, 224, SDL_WINDOW_RESIZABLE);
        renderer = SDL_CreateRenderer(window, NULL);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        value = v;
    }

    void GameWindow::UpdateDisplay(const char* videoData)
    {
        SDL_FPoint points[256 * 224] = { {0,0} };

        for (int row = 0; row < 224; row++)
        {
            for (int col = 0; col < 256; col++)
            {
                SDL_FPoint activePixel = { (float)col, (float)row };
                points[row * col + col] = activePixel;
                //for (int pixel = 0; pixel < 8; pixel++)
                //{
                //    bool isPixActive = videoData[0x2400 + (row * 256) + col] & (0x01 << pixel);
                //    if (isPixActive)
                //    {
                //        SDL_FPoint activePixel{ (col * 8 + pixel), (row * 256) };
                //        points[(row * 256) + (col * 8) + pixel] = activePixel;
                //    }
                //}
            }
        }

        SDL_RenderPoints(renderer, points, 256 * 224);
        SDL_RenderPresent(renderer);
    }

    void GameWindow::displayValue()
    {
        std::cout << "Value: " << value << std::endl;
    }

    GameWindow::~GameWindow()
    {
        SDL_DestroyWindow(window);
    }
}