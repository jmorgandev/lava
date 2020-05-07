#include "app.h"
#include <SDL/SDL.h>
#include "renderer.h"


lava_app::lava_app(const char * window_title, int width, int height)
{
    SDL_Init(SDL_INIT_VIDEO);

    sdl_window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN);

    renderer = std::make_unique<lava_renderer>(this);
    running = true;
}

void lava_app::poll_events()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            running = false;
            break;
        }
    }
}

lava_app::~lava_app()
{
    renderer.release();
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}