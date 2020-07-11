#include "app.h"
#include <SDL/SDL.h>
#include "renderer.h"

using namespace lava;

App::App(const char * window_title, int width, int height)
    : window_width(width), window_height(height)
{
    SDL_Init(SDL_INIT_VIDEO);

    sdl_window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN);

    renderer = std::make_unique<Renderer>(this);
    running = true;
}

void App::poll_events()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_RESIZED)
                renderer->handle_window_resize();
            break;
        }
    }
}

void App::draw_frame()
{
    renderer->draw_frame();
}

App::~App()
{
    renderer.reset();
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}