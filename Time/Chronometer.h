#pragma once
#include <Core/Allocate/Allocate.h>
#include <Context/Common/Module.h>
#include <Time/Common/FrameState.h>

#include <cstdint>

namespace Online::Runtime { class Runtime; }

namespace Online::Time
{
    class Chronometer
    {

    public:
        struct Factory
        {
            friend class Online::Runtime::Module<Chronometer>;
        private:
            static Chronometer* Create() { return ONLINE_NEW(Chronometer); }
            static void Destroy(Chronometer* p) { ONLINE_DELETE(p); }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<Chronometer>;
        private:
            static bool Initialize(Chronometer* p,bool EnableVSync) { return p->Initialize(EnableVSync); }
            static bool Initialize(Chronometer* p) { return p->Initialize(); }
            static void Release(Chronometer* p) { p->Release(); }
            static void Tick(Chronometer* p) { p->Tick(); }
            static void FrameSync(Chronometer* p) { p->FrameSync(); }
        };

    private:
        Chronometer() = default;
        ~Chronometer() = default;

    public:
        Chronometer(const Chronometer&) = delete;
        Chronometer& operator=(const Chronometer&) = delete;
        Chronometer(Chronometer&&) = delete;
        Chronometer& operator=(Chronometer&&) = delete;
    public:
        bool Initialize(bool EnableVSync);
        bool Initialize();
        void Release();
        void Tick();
        void FrameSync();

    public:
        inline float Getdelta() const noexcept { return state.delta; }
        inline float GetFixdelta() const noexcept { return state.fixdelta; }
        inline float GetUnscaledDelta() const noexcept { return state.unscaledDeltaTime; }
        inline float GetTimeScale() const noexcept { return state.scale; }
        inline float GetFramerate() const noexcept { return state.FPS; }
        inline double Seconds() const noexcept { return state.seconds; }
        inline int64_t Milliseconds() const noexcept { return state.milliseconds; }

    private:
        FrameState state;
		bool EnableVSync = true;
        uint64_t lastCounter = 0;
        uint64_t startCounter = 0;
        double freq = 0.0;
    };
}