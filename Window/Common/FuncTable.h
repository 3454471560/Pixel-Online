#pragma once

#include <Context/Context.h>
#include <Client/Context/ClientContext.h>
#include <Window/Frontend/Window.h>

#include <stdexcept>

namespace Online::Runtime
{
    template<>
    struct FuncTable<Online::Window::Window>
    {
		friend class Online::Runtime::Client;
    private:
        FuncTable() = default;
        ~FuncTable() = default;

    public:
        FuncTable(const FuncTable&) = delete;
        FuncTable& operator=(const FuncTable&) = delete;
        FuncTable(FuncTable&&) = delete;
        FuncTable& operator=(FuncTable&&) = delete;

    public:
        bool Check() const
        {
            if (!OnGetWindow) { throw std::runtime_error("FuncTable miss [Window::GetWindow] Function!"); }
            if (!OnPollEvents) { throw std::runtime_error("FuncTable miss [Window::PollEvents] Function!"); }
            if (!OnCloseWindow) { throw std::runtime_error("FuncTable miss [Window::CloseWindow] Function!"); }
            if (!OnIsClose) { throw std::runtime_error("FuncTable miss [Window::IsClose] Function!"); }
            if (!OnGetNativeWindow) { throw std::runtime_error("FuncTable miss [Window::GetNativeWindow] Function!"); }
            return true;
        }

        void UnRegister() noexcept
        {
            OnGetWindow = nullptr;
            OnPollEvents = nullptr;
            OnCloseWindow = nullptr;
            OnIsClose = nullptr;
            OnGetNativeWindow = nullptr;
        }

    public:
        Online::Window::Window* InvokeOnGetWindow() const noexcept { return OnGetWindow(); }
        void InvokeOnPollEvents() const { OnPollEvents(); }
        void InvokeOnCloseWindow() const { OnCloseWindow(); }
        bool InvokeOnIsClose() const noexcept { return OnIsClose(); }
        void* InvokeOnGetNativeWindow() const noexcept { return OnGetNativeWindow(); }

    private:
        Online::Window::Window* (*OnGetWindow)() noexcept = nullptr;
        void (*OnPollEvents)() = nullptr;
        void (*OnCloseWindow)() = nullptr;
        bool (*OnIsClose)() noexcept = nullptr;
        void* (*OnGetNativeWindow)() noexcept = nullptr;
    };
}

namespace Online::Window
{
    inline Online::Window::Window* GetWindow() noexcept
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Window::Window>().InvokeOnGetWindow();
    }
    inline void PollEvents()
    {
        Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Window::Window>().InvokeOnPollEvents();
    }
    inline void CloseWindow()
    {
        Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Window::Window>().InvokeOnCloseWindow();
    }
    inline bool IsClose() noexcept
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Window::Window>().InvokeOnIsClose();
    }
    inline void* GetNativeWindow() noexcept
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Window::Window>().InvokeOnGetNativeWindow();
    }
}
