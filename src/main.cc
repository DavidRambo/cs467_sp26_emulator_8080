#include "CPU8080.h"
#include "Memory8080.h"
#include "AudioMixer.h"
#include "GameWindow.h"
#include "SpaceInvadersVRamDecoder.h"
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

int main() 
{ 
    constexpr int kRows = 224;
    constexpr int kCols = 256;

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    std::unique_ptr<intel_8080::Memory8080> memory = 
        std::make_unique<intel_8080::Memory8080>(intel_8080::Memory8080());

    memory->load_rom("./invaders/invaders_all.txt");

    graphics_display::GameWindow display(nullptr, kCols, kRows, "SpaceInvaders");
    audio::Mixer sound;

    for (int i = 0; i < 5; i++) {
        SDL_Delay(2000);
    }
    SDL_Delay(5000);

    SDL_Quit();
    return 0; 
}
