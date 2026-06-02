#pragma once

#include <Event/Common/EventType.h>
#include <atomic>
#include <cstdint>

namespace Online::Event
{
    struct EventToken
    {
        EventToken(EventType type = EventType::Invalid)
            : id(Next()), type(type) {
        }

        EventToken(const EventToken&) = default;
        EventToken& operator=(const EventToken&) = default;

        bool operator==(const EventToken& other) const noexcept { return id == other.id; }

    public:
        uint32_t id = 0;
        EventType type = EventType::Invalid;

    private:
        inline static uint32_t Next() noexcept
        {
            static std::atomic<uint32_t> g = 1;
            return g.fetch_add(1, std::memory_order_relaxed);
        }
    };
}

namespace std
{
    template<>
    struct hash<Online::Event::EventToken>
    {
        size_t operator()(const Online::Event::EventToken& t) const noexcept
        {
            return std::hash<uint32_t>()(t.id);
        }
    };
}
