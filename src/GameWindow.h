#pragma once
#include <SDL3/SDL.h>
#include <vector>

namespace GraphicsDisplay
{
    class GameWindow
    {
    private:
        const char* videoData;
        const char* windowName;
        int windowWidth;
        int windowHeight;

        SDL_Window* window;
        SDL_Renderer* renderer;

    public:
        GameWindow(const char* videoData,
            int windowWidth,
            int windowHeight,
            const char* windowName);
        ~GameWindow();
        void UpdateDisplayTop(const std::vector<SDL_FPoint>& points);
        void UpdateDisplayBottom(const std::vector<SDL_FPoint>& points);
    };
}