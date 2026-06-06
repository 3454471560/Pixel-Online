#pragma once

#include<Core/Allocate/Allocate.h>
#include<Context/Common/Module.h>
#include<Serialize/Serializable.h>
#include<Render/Common/API.h>
#include<Window/Common/Platform.h>
#include<Config/Common/FuncTable.h>
#include<TileEdit/Common/TileMap.h>
#include<Config/Common/Info/AnimationInfo.h>
#include<Config/Common/TileMapID.h>
#include<Config/Common/Info/CharLayout.h>

#include<array>
#include<string>
#include<unordered_map>

namespace Online::Runtime { class Runtime; }

namespace Online::Config
{
	class Configurator
	{
	public:
		struct Factory
		{
			friend class Online::Runtime::Module<Configurator>;
		private:
			static Configurator* Create()
			{
				Configurator* configurator = ONLINE_NEW(Configurator);
				configurator->Import();
				return configurator;
			}
			static void Destroy(Configurator* configurator)
			{
				ONLINE_DELETE(configurator);
			}
		};
		struct Lifecycle
		{
			friend class Online::Runtime::Module<Configurator>;
		private:
			static bool Initialize(Configurator* configurator)
			{
				return configurator->Initialize();
			}
			static void Release(Configurator* configurator)
			{
				configurator->Release();
			}
		};

	private:
		Configurator() = default;
		~Configurator() = default;

	public:
		Configurator(const Configurator&) = delete;
		Configurator& operator=(const Configurator&) = delete;
		Configurator(Configurator&&) = delete;
		Configurator& operator=(Configurator&&) = delete;

	private:
		struct ConfigInfo : public Online::Serialize::Serializable
		{
			std::string Name = "Online";
			std::string Version = "1.0.0";

			Online::Render::API RenderAPI = Online::Render::API::SDL2D;
			bool EnableVSync = true;

			Online::Window::Platform WindowPlatform = Online::Window::Platform::SDL;
			int32_t WindowWidth = 1280;
			int32_t WindowHeight = 720;

		public:
			void Serialize(Online::Serialize::SerializeContext& context) const override;
			void Deserialize(const Online::Serialize::DeserializeContext& context) override;
		};

		struct AnimationsInfo : public Online::Serialize::Serializable
		{
			std::vector<Online::Config::AnimationInfo> animations;
		public:
			void Serialize(Online::Serialize::SerializeContext& context) const override;
			void Deserialize(const Online::Serialize::DeserializeContext& context) override;
		};

		struct MapInfo
		{
			std::vector<Online::TileEdit::TileMap> tileMaps;
		};
	private:
		bool Initialize();
		void Release();

	public:
		bool Import();
		bool Export();

	public:
		inline Online::Render::API GetRenderAPI() const noexcept
		{
			return configInfo.RenderAPI;
		}
		inline Online::Window::Platform GetWindowPlatform() const noexcept
		{
			return configInfo.WindowPlatform;
		}
		inline const std::string& GetWindowName()  const noexcept
		{
			return configInfo.Name;
		}
		inline int32_t GetWindowWidth() const noexcept
		{
			return configInfo.WindowWidth;
		}
		inline int32_t GetWindowHeight() const noexcept
		{
			return configInfo.WindowHeight;
		}
		inline bool GetEnableVSync() const noexcept
		{
			return configInfo.EnableVSync;
		}
		inline const AnimationsInfo& GetAnimationsInfo() const noexcept
		{
			return animationsInfo;
		}
		inline const std::vector<AnimationInfo>& GetAnimations() const noexcept
		{
			return animationsInfo.animations;
		}
		inline const std::vector<Online::TileEdit::TileMap>& GetTileMaps() const noexcept
		{
			return tileMapsInfo.tileMaps;
		}
		inline const Online::TileEdit::TileMap& GetTileMap(TileMapID ID) const noexcept
		{
			return tileMapsInfo.tileMaps[static_cast<size_t>(ID)];
		}
		inline const AnimationInfo* GetAnimationByName(const std::string& name) const noexcept
		{
			for (const auto& anim : animationsInfo.animations)
			{
				if (anim.name == name)
					return &anim;
			}
			return nullptr;
		}
		inline const std::vector<CharLayout>& GetCharLayouts() const noexcept
		{
			return charLayouts;
		}
	private:
		Online::Config::Configurator::ConfigInfo configInfo;
		Online::Config::Configurator::AnimationsInfo animationsInfo;
		Online::Config::Configurator::MapInfo tileMapsInfo;

		std::u32string charset;
		int charGridCols = 0;
		int charGridRows = 0;
		std::vector<Config::CharLayout> charLayouts;

		void BuildCharLayout();

	private:
		inline static constexpr const char* configFileName = "Client.json";
		inline static constexpr const char* animationsFileName = "animations.json";
		inline static constexpr const char* charsetFileName = "charset.txt";
		inline static constexpr const char* Maps[] = {
			"Map_01.csv",
			"Map_02.csv",
			"Map_03.csv",
			"Map_04.csv"
		};
	};
}
