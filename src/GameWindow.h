#ifndef GAMEWINDOW
#define GAMEWINDOW
#include <SDL3/SDL.h>
#include <vector>

namespace graphics_display
{
    class GameWindow
    {
    private:
        const char* video_data_;
        const char* window_name_;
        int window_width_;
        int window_height_;

        SDL_Window* window_;
        SDL_Renderer* renderer_;

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
#endif