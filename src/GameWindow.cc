#include "GameWindow.h"
#include <iostream>
#include <SDL3/SDL.h>

namespace GraphicsDisplay
{
    GameWindow::GameWindow(const char* videoData, int windowWidth,
        int windowHeight, const char* windowName)
    {
        this->videoData = videoData;
        this->windowName = windowName;
        this->windowWidth = windowWidth;
        this->windowHeight = windowHeight;

        window = SDL_CreateWindow(windowName, windowWidth, windowHeight,
            SDL_WINDOW_RESIZABLE);
        renderer = SDL_CreateRenderer(window, NULL);
    }

    void GameWindow::UpdateDisplayTop(const std::vector<SDL_FPoint>& points)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderPoints(renderer, points.data(), points.size());
        SDL_RenderPresent(renderer);
    }

    void GameWindow::UpdateDisplayBottom(const std::vector<SDL_FPoint>& points)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderPoints(renderer, points.data(), points.size());
        SDL_RenderPresent(renderer);
    }

    GameWindow::~GameWindow()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }
}