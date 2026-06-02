#pragma once

#include <Event/Common/EventArgs.h>
#include <Event/Common/EventType.h>

#include <memory>
#include <stdexcept>
#include <type_traits>

namespace Online::Event
{
    struct Event
    {
        Online::Event::EventType type = Online::Event::EventType::Invalid;
        const Online::Event::EventArgs* args;

        Event() = default;

        Event(Online::Event::EventType type, const Online::Event::EventArgs* args)
            : type(type), args(args) {}

        template<class T>
        const T& As() const
        {
            static_assert(std::is_base_of_v<Online::Event::EventArgs, T>);
            if (!args) { throw std::runtime_error("Event args is null"); }
            return static_cast<const T&>(*args);
        }
    };
}
