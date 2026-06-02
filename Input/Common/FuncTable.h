#pragma once

#include <Context/Context.h>
#include <Client/Context/ClientContext.h>
#include <Input/Common/KeyCode.h>

#include <glm.hpp>

#include <stdexcept>

namespace Online::Runtime
{
    template<>
    struct FuncTable<Online::Input::InputMonitor>
    {
		friend class Online::Runtime::Client;
    private:
        FuncTable() = default;
        ~FuncTable() = default;

    public:
        FuncTable(const FuncTable&) = delete;
        FuncTable& operator=(const FuncTable&) = delete;

    public:
        bool Check() const
        {
            if (!OnGetKeyDown) { throw std::runtime_error("FuncTable miss [Input::GetKeyDown] Function!"); }
            if (!OnGetKeyPressed) { throw std::runtime_error("FuncTable miss [Input::GetKeyPressed] Function!"); }
            if (!OnGetKeyReleased) { throw std::runtime_error("FuncTable miss [Input::GetKeyReleased] Function!"); }
            if (!OnResetAllState) { throw std::runtime_error("FuncTable miss [Input::OnResetAllState] Function!"); }
            if (!OnResetMouseState) { throw std::runtime_error("FuncTable miss [Input::OnResetMouseState] Function!"); }
            if (!OnGetMousePosition) { throw std::runtime_error("FuncTable miss [Input::OnGetMousePosition] Function!"); }
            return true;
        }

        void UnRegister() noexcept
        {
            OnGetKeyDown = nullptr;
            OnGetKeyPressed = nullptr;
            OnGetKeyReleased = nullptr;
            OnResetAllState = nullptr;
            OnResetMouseState = nullptr;
            OnGetMousePosition = nullptr;
        }

    public:
        bool InvokeOnGetKeyDown(Online::Input::KeyCode keyCode) noexcept 
        { 
            return OnGetKeyDown(keyCode); 
        }
        bool InvokeOnGetKeyPressed(Online::Input::KeyCode keyCode) noexcept 
        { 
            return OnGetKeyPressed(keyCode); 
        }
        bool InvokeOnGetKeyReleased(Online::Input::KeyCode keyCode) noexcept 
        { 
            return OnGetKeyReleased(keyCode); 
        }
        void InvokeOnResetAllState() noexcept
        {
            OnResetAllState();
        }
        void InvokeOnResetMouseState() noexcept
        {
            OnResetMouseState();
        }
        glm::vec2 InvokeOnGetMousePosition() noexcept
        {
            return OnGetMousePosition();
        }

    private:
        bool (*OnGetKeyDown)(Online::Input::KeyCode) noexcept = nullptr;
        bool (*OnGetKeyPressed)(Online::Input::KeyCode) noexcept = nullptr;
        bool (*OnGetKeyReleased)(Online::Input::KeyCode) noexcept = nullptr;
        void (*OnResetAllState)() noexcept = nullptr;
        void (*OnResetMouseState)() noexcept = nullptr;
        glm::vec2(*OnGetMousePosition)() noexcept = nullptr;
    };
}

namespace Online::Input
{
    inline bool GetKeyDown(Online::Input::KeyCode keyCode) noexcept
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Input::InputMonitor>().InvokeOnGetKeyDown(keyCode);
    }

    inline bool GetKeyPressed(Online::Input::KeyCode keyCode) noexcept
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Input::InputMonitor>().InvokeOnGetKeyPressed(keyCode);
    }

    inline bool GetKeyReleased(Online::Input::KeyCode keyCode) noexcept
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Input::InputMonitor>().InvokeOnGetKeyReleased(keyCode);
    }

    inline void ResetAllState() noexcept
    {
        Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Input::InputMonitor>().InvokeOnResetAllState();
    }

    inline void ResetMouseState() noexcept
    {
        Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Input::InputMonitor>().InvokeOnResetMouseState();
    }

    inline glm::vec2 OnGetMousePosition() noexcept
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Input::InputMonitor>().InvokeOnGetMousePosition();
    }
}
