#pragma once

#include <Core/Allocate/Allocate.h>
#include <Context/Common/Module.h>

#include <Input/Common/KeyCode.h>
#include <Window/Common/Platform.h>

namespace Online::Runtime { class Runtime; }

namespace Online::Window
{
    class Window
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<Window>;
        private:
            static Window* Create(Online::Window::Platform platform);
            static void Destroy(Window* window)
            {
                ONLINE_DELETE(window);
            }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<Window>;
        private:
            static bool Initialize(Window* window, int width, int height, const char* title)
            {
                return window->Initialize(width, height, title);
            }
            static void Release(Window* window)
            {
                window->Release();
            }
        };

    protected:
        Window() = default;
        virtual ~Window() = default;

    public:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

    protected:
        virtual bool Initialize(int width, int height, const char* title) = 0;
        virtual void Release() = 0;

    public:
        virtual void PollEvents() = 0;
        virtual void CloseWindow() = 0;

    public:
        virtual bool IsClose() const = 0;
        virtual void* GetNativeWindow() const = 0;

    protected:
        inline Online::Input::KeyCode IntToKeyCode(int scancode) const noexcept
        {
            return (scancode < 0 || scancode >= 512)
                ? Online::Input::KeyCode::Unknown
                : map[scancode];
        }

    protected:
        Online::Input::KeyCode map[512] = {};
        int width = 0;
        int height = 0;
    };
}
