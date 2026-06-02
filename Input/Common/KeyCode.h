#pragma once
#include <cstdint>

namespace Online::Input
{
    enum class KeyCode : uint8_t
    {
        Mouse0, Mouse1, Mouse2, Mouse3, Mouse4,

        Key0, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,

        A, B, C, D, E, F, J, H, I, G, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        Space,
        Semicolon,
        Equal,
        LeftBracket,
        RightBracket,
        Comma,

        Escape,
        Enter,
        Tab,
        Backspace,
        Insert,
        Delete,

        Right,
        Left,
        Down,
        Up,

        PageUp,
        PageDown,

        Home,
        End,

        CapsLock,

        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

        LeftShift,
        LeftControl,
        LeftAlt,
        LeftSuper,

        RightShift,
        RightControl,
        RightAlt,
        RightSuper,

        Unknown
    };
}
