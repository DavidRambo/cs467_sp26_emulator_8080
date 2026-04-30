#include "CPU8080.h"
#include "Instructions8080.h"
#include "Memory8080.h"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <SDL3/SDL.h>
#include "GraphicsDisplay.h"

using namespace std;

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    GraphicsDisplay::GameWindow obj(10);
    SDL_Delay(2000);
    obj.displayValue();

    int numDrivers = SDL_GetNumRenderDrivers();
    for (int i = 0; i < numDrivers - 1; i++)
    {
        cout << SDL_GetRenderDriver(i) << endl;
    }

    ifstream e_file("../invaders.e", ios::binary);
    ifstream f_file("../invaders.f", ios::binary);
    ifstream g_file("../invaders.g", ios::binary);
    ifstream h_file("../invaders.h", ios::binary);

    char buffer[65536] = { (char)0 };

    char data;
    uint i = 0;

    while (h_file.get(data))
    {
        buffer[i++] = data;
    }

    while (g_file.get(data))
    {
        buffer[i++] = data;
    }

    while (f_file.get(data))
    {
        buffer[i++] = data;
    }

    while (e_file.get(data))
    {
        buffer[i++] = data;
    }

    int startIndex = 0x2400;
    for (int row = 0; row < 224; row++)
    {
        for (int col = 0; col < 32; col++)
        {
            buffer[startIndex + row * 32 + col] =
                buffer[startIndex + row * 32 + col] ^ 0xFF;
        }
    }
    obj.UpdateDisplay(buffer);

    SDL_Delay(15000);
    for (int x = 0x2400; x < 0x4000; x++)
    {
        std::cout << std::hex << (int)(unsigned char)buffer[x] << " ";
    }


    SDL_Quit();
    return 0;
}
