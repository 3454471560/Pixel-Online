#pragma once
#include<Game/Component/Component.h>

#include<entt/entt.hpp>

namespace Online::Game
{
    struct ChildLink : public Component
    {
    public:
        inline void SetFirstChild(entt::entity childId) noexcept
        {
            firstChild = childId;
        }
        inline void SetPreviousSibling(entt::entity siblingId) noexcept
        {
            previousSibling = siblingId;
        }
        inline void SetNextSibling(entt::entity siblingId) noexcept
        {
            nextSibling = siblingId;
        }
        inline entt::entity GetFirstChild() const noexcept
        {
            return firstChild;
        }
        inline entt::entity GetPreviousSibling() const noexcept
        {
            return previousSibling;
        }
        inline entt::entity GetNextSibling() const noexcept
        {
            return nextSibling;
        }
    private:
        entt::entity firstChild = entt::null;
        entt::entity previousSibling = entt::null;
        entt::entity nextSibling = entt::null;
    };
}
