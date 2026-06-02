#pragma once
#include <Context/Context.h>
#include <stdexcept>

namespace Online::Runtime
{
    template<>
    struct FuncTable<Online::Time::Chronometer>
    {
        friend class Online::Runtime::Client;
        friend class Online::Runtime::Server;
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
            if (!OnDelta) { throw std::runtime_error("FuncTable miss [Time::Delta] Function!"); }
            if (!OnFixDelta) { throw std::runtime_error("FuncTable miss [Time::FixDelta] Function!"); }
            if (!OnUnscaledDelta) { throw std::runtime_error("FuncTable miss [Time::UnscaledDelta] Function!"); }
            if (!OnTimeScale) { throw std::runtime_error("FuncTable miss [Time::TimeScale] Function!"); }
            if (!OnFramerate) { throw std::runtime_error("FuncTable miss [Time::Framerate] Function!"); }
            if (!OnSeconds) { throw std::runtime_error("FuncTable miss [Time::Seconds] Function!"); }
            if (!OnMilliseconds) { throw std::runtime_error("FuncTable miss [Time::Milliseconds] Function!"); }
            return true;
        }

        void UnRegister() noexcept
        {
            OnDelta = nullptr;
            OnFixDelta = nullptr;
            OnUnscaledDelta = nullptr;
            OnTimeScale = nullptr;
            OnFramerate = nullptr;
            OnSeconds = nullptr;
            OnMilliseconds = nullptr;
        }

    public:
        float InvokeOnDelta() const noexcept { return OnDelta(); }
        float InvokeOnFixDelta() const noexcept { return OnFixDelta(); }
        float InvokeOnUnscaledDelta() const noexcept { return OnUnscaledDelta(); }
        float InvokeOnTimeScale() const noexcept { return OnTimeScale(); }
        float InvokeOnFramerate() const noexcept { return OnFramerate(); }
        double InvokeOnSeconds() const noexcept { return OnSeconds(); }
        int64_t InvokeOnMilliseconds() const noexcept { return OnMilliseconds(); }

    private:
        float(*OnDelta)() noexcept = nullptr;
        float(*OnFixDelta)() noexcept = nullptr;
        float(*OnUnscaledDelta)() noexcept = nullptr;
        float(*OnTimeScale)() noexcept = nullptr;
        float(*OnFramerate)() noexcept = nullptr;
        double(*OnSeconds)() noexcept = nullptr;
        int64_t(*OnMilliseconds)() noexcept = nullptr;
    };
}

namespace Online::Time
{
    inline float delta() noexcept
    {
        return Online::Runtime::Context::Instance().GetFuncTable<Chronometer>().InvokeOnDelta();
    }
    inline float fixdelta() noexcept
    {
        return Online::Runtime::Context::Instance().GetFuncTable<Chronometer>().InvokeOnFixDelta();
    }
    inline float unscaledDelta() noexcept
    {
        return Online::Runtime::Context::Instance().GetFuncTable<Chronometer>().InvokeOnUnscaledDelta();
    }
    inline float timeScale() noexcept
    {
        return Online::Runtime::Context::Instance().GetFuncTable<Chronometer>().InvokeOnTimeScale();
    }
    inline float framerate() noexcept
    {
        return Online::Runtime::Context::Instance().GetFuncTable<Chronometer>().InvokeOnFramerate();
    }
    inline double seconds() noexcept
    {
        return Online::Runtime::Context::Instance().GetFuncTable<Chronometer>().InvokeOnSeconds();
    }
    inline int64_t milliseconds() noexcept
    {
        return Online::Runtime::Context::Instance().GetFuncTable<Chronometer>().InvokeOnMilliseconds();
    }
}