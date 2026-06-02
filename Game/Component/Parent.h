#pragma once
#include<Game/Component/Component.h>

#include<entt/entt.hpp>

namespace Online::Game
{
	struct Parent : public Component
	{
	public:
		inline void SetId(entt::entity parentId) noexcept
		{
			this->id = parentId;
		}
	public:
		inline entt::entity GetId() const noexcept
		{
			if(id == entt::null)
			{
				return entt::null;
			}
			return id;
		}
	private:
		entt::entity id = entt::null;
	};
}
