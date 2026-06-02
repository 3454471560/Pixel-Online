#pragma once

#include <Core/Allocate/Allocate.h>
#include <Context/Common/Module.h>

#include <Event/Common/Event.h>
#include <Event/Common/EventToken.h>
#include <Event/Common/EventType.h>

#include <unordered_map>
#include <vector>

namespace Online::Event
{
    class EventDispatcher
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<EventDispatcher>;
        private:
            static EventDispatcher* Create() { return ONLINE_NEW(EventDispatcher); }
            static void Destroy(EventDispatcher* p) { ONLINE_DELETE(p); }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<EventDispatcher>;
        private:
            static bool Initialize(EventDispatcher* p) { return p->Initialize(); }
            static void Release(EventDispatcher* p) { p->Release(); }
        };

    private:
        struct CallBack
        {
            void (*OnCallBack)(void*, const ::Online::Event::Event&) = nullptr;
            void* listener = nullptr;
        };

        struct EventRegistry
        {
            std::vector<std::unordered_map<::Online::Event::EventToken, CallBack>> map;
        };

    private:
        EventDispatcher() = default;
        ~EventDispatcher() = default;

    public:
        EventDispatcher(const EventDispatcher&) = delete;
        EventDispatcher& operator=(const EventDispatcher&) = delete;

    private:
        bool Initialize();
        void Release();

    public:
        Online::Event::EventToken Subscribe(
            Online::Event::EventType type,
            void (*OnCallBack)(void*, const ::Online::Event::Event&),
            void* listener) noexcept;

        bool UnSubscribe(const Online::Event::EventToken& token) noexcept;
        void Emit(const Online::Event::Event& event) noexcept;

    private:
        EventRegistry registry;
    };
}
