#include <Event/EventDispatcher.h>
#include <Log/Common/FuncTable.h>

#include <cstdint>

bool Online::Event::EventDispatcher::Initialize()
{
    registry.map.resize(static_cast<uint8_t>(Online::Event::EventType::Invalid));
    return true;
}

void Online::Event::EventDispatcher::Release()
{
    registry.map.clear();
}

Online::Event::EventToken Online::Event::EventDispatcher::Subscribe
(
    Online::Event::EventType type,
    void (*OnCallBack)(void*, const ::Online::Event::Event&),
    void* listener) noexcept
{
    if (!OnCallBack || !listener || type == Online::Event::EventType::Invalid)
    {
        Online::Log::Warning("try to subscibe a invalid event");
        return Online::Event::EventToken(Online::Event::EventType::Invalid);
    }

    Online::Event::EventToken token(type);
    registry.map[static_cast<uint8_t>(type)][token] = { OnCallBack, listener };
    return token;
}

bool Online::Event::EventDispatcher::UnSubscribe(const Online::Event::EventToken& token) noexcept
{
    if (token.type == Online::Event::EventType::Invalid)
    {
        Online::Log::Warning("try to unsubscibe event from a invalid token");
        return false;
    }

    auto& m = registry.map[static_cast<uint8_t>(token.type)];
    auto it = m.find(token);
    if (it == m.end())
    {
        Online::Log::Warning("event token not in event map");
        return false;
    }

    m.erase(it);
    return true;
}

void Online::Event::EventDispatcher::Emit(const Online::Event::Event& event) noexcept
{
    if (event.type == Online::Event::EventType::Invalid)
    {
        Online::Log::Warning("try to emit a invalid event");
        return;
    }

    auto& m = registry.map[static_cast<uint8_t>(event.type)];

    std::vector<CallBack> callbacks;
    callbacks.reserve(m.size());
    for (const auto& kv : m) { callbacks.push_back(kv.second); }

    for (const auto& cb : callbacks)
    {
        if (cb.OnCallBack) { cb.OnCallBack(cb.listener, event); }
    }
}
