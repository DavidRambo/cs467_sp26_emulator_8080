#include "CPU8080.h"
#include "Instructions8080.h"
#include "Memory8080.h"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string_view>
#include <SDL3/SDL.h>
#include "GameWindow.h"
#include "SpaceInvadersVRamDecoder.h"

using namespace std;

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    constexpr int ROWS = 224;
    constexpr int COLS = 256;
    char buffer[7168] = { (char)0x00 };

    GraphicsDisplay::GameWindow displayWindow(buffer,
        COLS, ROWS, "Space Invaders");

    std::string input = "";
    bool renderTop = false;

    while (true)
    {
        std::cout << "Enter 'exit' to stop the loop.";
        std::getline(std::cin, input);

        if (renderTop)
        {
            displayWindow.UpdateDisplayTop(SpaceInvadersVRamDecoder::DecodeTopPixels(buffer));
            renderTop = false;
            for (int i = 0; i < 7168; i++)
            {
                buffer[i] = buffer[i] ^ 0xFF;
            }
        }
        else
        {
            displayWindow.UpdateDisplayBottom(SpaceInvadersVRamDecoder::DecodeBottomPixels(buffer));
            renderTop = true;
        }

        if (input == "exit") { break; }
    }

    SDL_Quit();
    return 0;
}
