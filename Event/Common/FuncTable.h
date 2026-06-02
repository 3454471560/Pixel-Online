#pragma once

#include <Context/Context.h>
#include <Event/Common/Event.h>
#include <Event/Common/EventToken.h>
#include <Event/Common/EventType.h>

#include <stdexcept>

namespace Online::Runtime
{
    template<>
    struct FuncTable<Online::Event::EventDispatcher>
    {
		friend class Online::Runtime::Client;
		friend class Online::Runtime::Server;
    private:
        FuncTable() = default;
        ~FuncTable() = default;

    public:
        FuncTable(const FuncTable&) = delete;
        FuncTable& operator=(const FuncTable&) = delete;

    public:
        bool Check() const
        {
            if (!OnSubscribe) { throw std::runtime_error("FuncTable miss [Event::Subscribe] Function!"); }
            if (!OnUnSubscribe) { throw std::runtime_error("FuncTable miss [Event::UnSubscribe] Function!"); }
            if (!OnEmit) { throw std::runtime_error("FuncTable miss [Event::Emit] Function!"); }
            return true;
        }

        void UnRegister() noexcept
        {
            OnSubscribe = nullptr;
            OnUnSubscribe = nullptr;
            OnEmit = nullptr;
        }

        void Register(
            Online::Event::EventToken(*sub)(Online::Event::EventType, void (*)(void*, const ::Online::Event::Event&), void*) noexcept,
            bool (*unsub)(const Online::Event::EventToken&) noexcept,
            void (*emit)(const Online::Event::Event&) noexcept
        ) noexcept
        {
            OnSubscribe = sub;
            OnUnSubscribe = unsub;
            OnEmit = emit;
        }

    public:
        Online::Event::EventToken InvokeOnSubscribe(
            Online::Event::EventType type,
            void (*OnCallBack)(void*, const ::Online::Event::Event&),
            void* listener) noexcept
        {
            return OnSubscribe(type, OnCallBack, listener);
        }

        bool InvokeOnUnSubscribe(const Online::Event::EventToken& token) noexcept
        {
            return OnUnSubscribe(token);
        }

        void InvokeOnEmit(const Online::Event::Event& event) noexcept
        {
            OnEmit(event);
        }

    private:
        Online::Event::EventToken(*OnSubscribe)(
            Online::Event::EventType,
            void (*)(void*, const ::Online::Event::Event&),
            void*) noexcept = nullptr;

        bool (*OnUnSubscribe)(const Online::Event::EventToken&) noexcept = nullptr;
        void (*OnEmit)(const Online::Event::Event&) noexcept = nullptr;
    };
}

namespace Online::Event
{
    inline Online::Event::EventToken Subscribe(
        Online::Event::EventType type,
        void (*OnCallBack)(void*, const ::Online::Event::Event&),
        void* listener) noexcept
    {
        return Online::Runtime::Context::Instance()
            .GetFuncTable<Online::Event::EventDispatcher>()
            .InvokeOnSubscribe(type, OnCallBack, listener);
    }

    inline bool UnSubscribe(const Online::Event::EventToken& token) noexcept
    {
        return Online::Runtime::Context::Instance()
            .GetFuncTable<Online::Event::EventDispatcher>()
            .InvokeOnUnSubscribe(token);
    }

    inline void Emit(const Online::Event::Event& event) noexcept
    {
        Online::Runtime::Context::Instance()
            .GetFuncTable<Online::Event::EventDispatcher>()
            .InvokeOnEmit(event);
    }
}
