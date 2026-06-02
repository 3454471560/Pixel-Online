#pragma once

#include<Core/Identifier/UID.h>

namespace Online::Core
{
	class Object
	{
	public:
		Object() = default;
		~Object() = default;
		Object(const Object&) = default;
		Object& operator=(const Object&) = default;
		Object(Object&&) = default;
		Object& operator=(Object&&) = default;

	public:
		inline Online::Core::UID GetUID() const noexcept
		{
			return uid;
		}
		inline Online::Core::UID SetUID(Online::Core::UID uid) noexcept
		{
			this->uid = uid;
		}

	protected:
		Online::Core::UID uid = Online::Core::UID::Next();
	};
}
