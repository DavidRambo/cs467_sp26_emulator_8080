#pragma once
#include <SDL3/SDL.h>

namespace GraphicsDisplay
{
    class GameWindow
    {
    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        int value;

    public:
        GameWindow(int v);
        ~GameWindow();
        void UpdateDisplay(const char* videoData);
        void displayValue();
        int ReturnValue();
    };
}