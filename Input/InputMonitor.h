#pragma once

#include <Core/Allocate/Allocate.h>
#include <Context/Common/Module.h>
#include <Time/Common/FuncTable.h>
#include <Event/Common/Event.h>
#include <Event/Common/EventType.h>
#include <Event/Common/FuncTable.h>
#include <Input/Common/KeyCode.h>
#include <Input/Common/MouseState.h>

#include <glm.hpp>
#include <string>
#include <cstddef>
#include <cstring>

namespace Online::Runtime { class Runtime; }

namespace Online::Input
{
    class InputMonitor
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<InputMonitor>;
        private:
            static InputMonitor* Create() { return ONLINE_NEW(InputMonitor); }
            static void Destroy(InputMonitor* p) { ONLINE_DELETE(p); }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<InputMonitor>;
        private:
            static bool Initialize(InputMonitor* p) { return p->Initialize(); }
            static void Release(InputMonitor* p) { p->Release(); }
        };

    private:
        InputMonitor() = default;
        ~InputMonitor() = default;

    public:
        InputMonitor(const InputMonitor&) = delete;
        InputMonitor& operator=(const InputMonitor&) = delete;

    private:
        bool Initialize();
        void Release();

    public:
        inline void PrepareNewFrame() noexcept
        {
            std::memcpy(previousFrameState, currentFrameState, sizeof(previousFrameState));
            std::memcpy(&mousePreviousFrameState, &mouseCurrentFrameState, sizeof(mouseCurrentFrameState));
        }

        inline void UpdateSnapshots() noexcept
        {
            std::memcpy(currentFrameState, hardwareState, sizeof(currentFrameState));
            std::memcpy(&mouseCurrentFrameState, &mouseHardWareState, sizeof(mouseCurrentFrameState));
        }

        inline bool GetKeyDown(Online::Input::KeyCode keyCode) const noexcept
        {
            return (keyCode != Online::Input::KeyCode::Unknown)
                ? currentFrameState[static_cast<size_t>(keyCode)]
                : false;
        }

        inline bool GetKeyPressed(Online::Input::KeyCode keyCode) const noexcept
        {
            return (keyCode != Online::Input::KeyCode::Unknown)
                ? (currentFrameState[static_cast<size_t>(keyCode)] && !previousFrameState[static_cast<size_t>(keyCode)])
                : false;
        }

        inline bool GetKeyReleased(Online::Input::KeyCode keyCode) const noexcept
        {
            return (keyCode != Online::Input::KeyCode::Unknown)
                ? (!currentFrameState[static_cast<size_t>(keyCode)] && previousFrameState[static_cast<size_t>(keyCode)])
                : false;
        }

        inline glm::vec2 GetMousePosition() const noexcept
        {
            return { mouseCurrentFrameState.mouseX, mouseCurrentFrameState.mouseY };
        }

        std::string GetTextInputBuffer() noexcept;

        std::string GetCompositionText() const noexcept;
        int GetCompositionCursor() const noexcept;

        void StartTextInput() noexcept;
        void StopTextInput() noexcept;

        void SetTextInputRect(int x, int y, int w, int h) noexcept;

        inline void ResetAllState() noexcept
        {
            std::memset(hardwareState, 0, sizeof(hardwareState));
            std::memset(currentFrameState, 0, sizeof(currentFrameState));
            std::memset(previousFrameState, 0, sizeof(previousFrameState));
        }

        inline void ResetMouseState() noexcept
        {
            constexpr size_t mouseStart = static_cast<size_t>(KeyCode::Mouse0);
            constexpr size_t mouseEnd = static_cast<size_t>(KeyCode::Mouse4);
            for (size_t i = mouseStart; i <= mouseEnd; ++i)
            {
                hardwareState[i] = false;
                currentFrameState[i] = false;
                previousFrameState[i] = false;
            }
        }

    private:
        void OnKey(const Online::Event::Event& event);
        void OnMouseButton(const Online::Event::Event& event);
        void OnMouseMove(const Online::Event::Event& event);
        void OnTextInput(const Online::Event::Event& event);
        void OnTextEditing(const Online::Event::Event& event);

        static void OnKeyThunk(void* listener, const Online::Event::Event& event);
        static void OnMouseButtonThunk(void* listener, const Online::Event::Event& event);
        static void OnMouseMoveThunk(void* listener, const Online::Event::Event& event);
        static void OnTextInputThunk(void* listener, const Online::Event::Event& event);
        static void OnTextEditingThunk(void* listener, const Online::Event::Event& event);


    private:
        Online::Event::EventToken keyToken;
        Online::Event::EventToken mouseButtonToken;
        Online::Event::EventToken mouseMoveToken;
        Online::Event::EventToken mouseScrollToken;
        Online::Event::EventToken windowResizeToken;
        Online::Event::EventToken textInputToken;
        Online::Event::EventToken textEditingToken;

        std::string textInputBuffer;
        std::string compositionText;
        int compositionStart = 0;
        int compositionLength = 0;

        bool currentFrameState[static_cast<size_t>(Online::Input::KeyCode::Unknown)] = {};
        bool previousFrameState[static_cast<size_t>(Online::Input::KeyCode::Unknown)] = {};
        bool hardwareState[static_cast<size_t>(Online::Input::KeyCode::Unknown)] = {};

        Online::Input::MouseSate mouseCurrentFrameState;
        Online::Input::MouseSate mousePreviousFrameState;
        Online::Input::MouseSate mouseHardWareState;
    };
}