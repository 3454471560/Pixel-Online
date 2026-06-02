#include <Window/Frontend/Window.h>
#include <Window/Common/Platform.h>

#include <Window/Backend/SDL/SDLWindow.h>  // 你放 SDLWindow 的路径按你的工程调整

namespace Online::Window
{
    Window* Window::Factory::Create(Online::Window::Platform platform)
    {
        switch (platform)
        {
        case Online::Window::Platform::SDL:
            return ONLINE_NEW(Online::Window::SDLWindow);

        default:
            return nullptr;
        }
    }
}
