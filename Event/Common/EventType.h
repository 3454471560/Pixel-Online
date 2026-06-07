#pragma once
#include <cstdint>

namespace Online::Event
{
    enum class EventType : uint8_t
    {
        System = 0,

        // Window
        WindowClose,
        WindowFramebufferResetSize,
        DropFile,
        WindowFocusGained,
        WindowFocusLost,
        MouseEnterWindow,
        MouseLeaveWindow,
        TextInput,
        TextInputStart,
        TextInputStop,
        TextEditing,
        SetTextInputRect,

        // Input
        MouseCurrsorMove,
        MouseScroll,
        MouseButton,
        Key,

        // Physic
        PhysicsTriggerEnter,
        PhysicsTriggerExit,
        PhysicsTriggerStay,
        PhysFixedUpdate,

        Invalid,
    };
}
