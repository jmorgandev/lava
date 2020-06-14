#ifndef LAVA_APP_H
#define LAVA_APP_H

#include <memory>

struct SDL_Window;
class lava_renderer;

class lava_app
{
public:
    lava_app(const char * window_title, int width, int height);
    ~lava_app();

    const char * title;
    SDL_Window * sdl_window;
    std::unique_ptr<lava_renderer> renderer;
    bool running;
    int window_width, window_height;

    void poll_events();
};

#endif