#include <Time/Chronometer.h>

#include <SDL.h>
#include <SDL_video.h>
bool Online::Time::Chronometer::Initialize(bool EnableVSync)
{
    this->EnableVSync = EnableVSync;
    freq = static_cast<double>(SDL_GetPerformanceFrequency());
    startCounter = SDL_GetPerformanceCounter();
    lastCounter = startCounter;
    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(0, &mode) == 0) {
        state.targetFPS = static_cast<float>(mode.refresh_rate);
    }
    return true;
}
bool Online::Time::Chronometer::Initialize()
{
    this->EnableVSync = false;
    freq = static_cast<double>(SDL_GetPerformanceFrequency());
    startCounter = SDL_GetPerformanceCounter();
    lastCounter = startCounter;
    return true;
}

void Online::Time::Chronometer::Release()
{

}

void Online::Time::Chronometer::Tick()
{
    uint64_t now = SDL_GetPerformanceCounter();
    double deltaTime = static_cast<double>(now - lastCounter) / freq;

    state.unscaledDeltaTime = static_cast<float>(deltaTime);
    state.delta = state.unscaledDeltaTime * state.scale;
    state.seconds = static_cast<double>(now - startCounter) / freq;
    state.milliseconds = static_cast<int64_t>(state.seconds * 1000.0);

    if (state.delta > 0.0f) {
        state.FPS = 1.0f / state.delta;
    }

    lastCounter = now;
}

void Online::Time::Chronometer::FrameSync()
{
    if(!EnableVSync)
		return;
    const double targetFPS = static_cast<double>(state.targetFPS);
    const double targetFrameTime = 1.0 / targetFPS;

    uint64_t now = SDL_GetPerformanceCounter();
    double frameTime = static_cast<double>(now - lastCounter) / freq;

    if (frameTime < targetFrameTime)
    {
        double delayTime = targetFrameTime - frameTime;
        SDL_Delay(static_cast<Uint32>(delayTime * 1000.0));
    }
}