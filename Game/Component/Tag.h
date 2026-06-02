#pragma once
#include<Game/Component/Component.h>
#include<Core/String/String.h>

namespace Online::Game
{
	class GameObject;
	struct Tag : public Component
	{
	public:
		Tag(std::string_view name, std::string_view tag)
		{
			Online::Core::CopyStringToCharArray(name, this->name);
			Online::Core::CopyStringToCharArray(tag, this->tag);
		}
		inline std::string_view GetName() const noexcept
		{
			return name;
		}
		inline std::string_view GetTag() const noexcept
		{
			return tag;
		}
		inline void SetName(std::string_view name) noexcept
		{
			Online::Core::CopyStringToCharArray(name, this->name);
		}
		inline void SetTag(std::string_view tag) noexcept
		{
			Online::Core::CopyStringToCharArray(tag, this->tag);
		}
		inline void Serialize(Online::Serialize::SerializeContext& ctx) const override
		{
			ctx.Write("name", name);
			ctx.Write("tag", tag);
		}
		void Deserialize(const Serialize::DeserializeContext& ctx) override
		{
			std::string nameStr, tagStr;
			if (ctx.Read("name", nameStr))
				Online::Core::CopyStringToCharArray(nameStr, this->name);
			if (ctx.Read("tag", tagStr)) 
				Online::Core::CopyStringToCharArray(tagStr, this->tag);

			Component::OnEnable();
		}
	public:
		inline static const constexpr uint32_t MAX_STR_SIZE = 128;
	private:
		char name[MAX_STR_SIZE] = {};
		char tag[MAX_STR_SIZE] = {};
	};
}