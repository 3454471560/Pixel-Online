#pragma once

#include <Input/Common/KeyCode.h>
#include <entt/entt.hpp>

#include <string>
#include <vector>

namespace Online::Event
{
    struct EventArgs
    {
        virtual ~EventArgs() = default;
    };

    struct CloseEventArgs : public EventArgs
    {
    };

    struct FramebufferResetSizeEventArgs : public EventArgs
    {
        FramebufferResetSizeEventArgs(int width, int height) : width(width), height(height) {}
        int width = 0;
        int height = 0;
    };

    struct DropFileEventArgs : public EventArgs
    {
        DropFileEventArgs() = default;

        explicit DropFileEventArgs(std::vector<std::string> inPaths)
            : ownedPaths(std::move(inPaths))
        {
            ptrs.reserve(ownedPaths.size());
            for (auto& s : ownedPaths) { ptrs.push_back(s.c_str()); }

            count = static_cast<int>(ptrs.size());
            paths = ptrs.empty() ? nullptr : ptrs.data();
        }

        int count = 0;
        const char* const* paths = nullptr;

        std::vector<std::string> ownedPaths;
        std::vector<const char*> ptrs;
    };

    struct MouseCurrsorMoveEventArgs : public EventArgs
    {
        MouseCurrsorMoveEventArgs(double xPosition, double yPosition, double xOffset, double yOffset)
            : xPosition(xPosition), yPosition(yPosition), xOffset(xOffset), yOffset(yOffset) {
        }
        double xPosition = 0;
        double yPosition = 0;
        double xOffset = 0;
        double yOffset = 0;
    };

    struct MouseScrollEventArgs : public EventArgs
    {
        MouseScrollEventArgs(double xOffset, double yOffset) : xOffset(xOffset), yOffset(yOffset) {}
        double xOffset = 0;
        double yOffset = 0;
    };

    struct KeyEventArgs : public EventArgs
    {
        KeyEventArgs(Online::Input::KeyCode keycode, int scancode, int action, int mods)
            : keycode(keycode), scancode(scancode), action(action), mods(mods) {
        }

        Online::Input::KeyCode keycode = Online::Input::KeyCode::Unknown;
        int scancode = 0;
        int action = 0;
        int mods = 0;
    };

    struct MouseButtonEventArgs : public EventArgs
    {
        MouseButtonEventArgs(Online::Input::KeyCode button, int action, int mods)
            : button(button), action(action), mods(mods) {
        }

        Online::Input::KeyCode button = Online::Input::KeyCode::Unknown;
        int action = 0;
        int mods = 0;
    };

    struct WindowFocusEventArgs : public EventArgs
    {
        explicit WindowFocusEventArgs(bool gained) : gained(gained) {}
        bool gained = false;
    };

    struct MouseWindowEnterLeaveEventArgs : public EventArgs
    {
        explicit MouseWindowEnterLeaveEventArgs(bool entered) : entered(entered) {}
        bool entered = false;
    };

    struct TextInputEventArgs : public EventArgs
    {
        explicit TextInputEventArgs(std::string text) : text(std::move(text)) {}
        std::string text;
    };

    struct PhysTriggerEventArgs : public EventArgs
    {
        PhysTriggerEventArgs(entt::entity trigger, entt::entity other)
            : triggerEntity(trigger), otherEntity(other) {
        }
        entt::entity triggerEntity;
        entt::entity otherEntity;
    };

    struct TextInputStartEventArgs : public EventArgs
    {
    };

    struct TextInputStopEventArgs : public EventArgs 
    {
    };

    struct TextEditingEventArgs : public EventArgs
    {
        std::string text;
        int start = 0;
        int length = 0;

        TextEditingEventArgs(const char* txt, int s, int len)
            : text(txt ? txt : ""), start(s), length(len) {
        }
    };

    struct SetTextInputRectEventArgs : public EventArgs
    {
        int x, y, w, h;
        SetTextInputRectEventArgs(int x, int y, int w, int h)
            : x(x), y(y), w(w), h(h) {
        }
    };

    struct PhysFixedUpdateEventArgs : public EventArgs
    {
    };
}
