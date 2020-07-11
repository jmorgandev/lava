#ifndef LAVA_APP_H
#define LAVA_APP_H

#include <memory>

struct SDL_Window;

namespace lava
{
    class Renderer;
    class App
    {
    public:
        App(const char * window_title, int width, int height);
        ~App();

        const char * title;
        SDL_Window * sdl_window;
        std::unique_ptr<Renderer> renderer;
        bool running;
        int window_width, window_height;

        void poll_events();
        void draw_frame();
    };
}

#endif