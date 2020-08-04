#include <memory>
#include "app.h"

int main(int argc, char** argv)
{
    auto app = std::make_unique<lava::App>("lava renderer", 1280, 720);
    while (app->running)
    {
        app->poll_events();
        app->draw_frame();
    }

    app.reset();

    return 0;
}