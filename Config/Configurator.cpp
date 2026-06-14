#include<Config/Configurator.h>
#include<Core/Utils/File.h>
#include<Core/String/String.h>
#include<Log/Common/FuncTable.h>

#include <unordered_set>

bool Online::Config::Configurator::Initialize()
{
	return true;
}
void Online::Config::Configurator::Release()
{
	if (!Export())
	{
		Online::Log::Error("config export fail!");
	}
}
bool Online::Config::Configurator::Import()
{
	tileMapsInfo.tileMaps.resize(4);
	for (int i = 0; i < 4; ++i) {
		std::filesystem::path map_path = std::filesystem::path(Online::Core::GetExeDir()) / Maps[i];
		if (!std::filesystem::exists(map_path)) { throw std::runtime_error("缺失地图配置文件 "); }
		tileMapsInfo.tileMaps[i].deserialize(map_path.string());
	}
#ifdef PIXEL_CLIENT
	std::filesystem::path path = std::filesystem::path(Online::Core::GetExeDir()) / configFileName;
	if (!std::filesystem::exists(path)) { throw std::runtime_error("缺失客户端配置文件"); }
	if (!configInfo.DeserializeFromFile(path, Online::Serialize::API::Json)) { throw std::runtime_error("客户端配置文件加载失败"); }
	path.clear(); path = std::filesystem::path(Online::Core::GetExeDir()) / animationsFileName;
	if (!animationsInfo.DeserializeFromFile(path, Online::Serialize::API::Json)) { throw std::runtime_error("缺失动画配置文件"); }

	path = std::filesystem::path(Online::Core::GetExeDir()) / charsetFileName;
	if (!std::filesystem::exists(path)) { throw std::runtime_error("缺失字符集配置文件 " + path.string()); }
	std::string fileContent; Online::Core::ReadFileToString(path, fileContent);
	if (!fileContent.empty()) {
		charset = Core::Utf8ToUtf32(fileContent);
		std::u32string uniqueChars;
		std::unordered_set<char32_t> seen;
		for (char32_t ch : charset) {
			if (seen.insert(ch).second)
				uniqueChars.push_back(ch);
		}
		charset = std::move(uniqueChars);
		BuildCharLayout();
	}
#endif // PIXEL_CLIENT
	
	return true;
}
bool Online::Config::Configurator::Export()
{
	std::filesystem::path path = std::filesystem::path(Online::Core::GetExeDir()) / configFileName;	
	return configInfo.SerializeToFile(path, Online::Serialize::API::Json);
}

void Online::Config::Configurator::BuildCharLayout()
{
	charLayouts.clear();

	if (charset.empty())
		return;

	charLayouts.reserve(charset.size());
	for (char32_t ch : charset)
	{
		CharLayout layout;
		layout.character = ch;
		charLayouts.push_back(layout);
	}
}

void Online::Config::Configurator::ConfigInfo::Serialize(Online::Serialize::SerializeContext& context) const
{
	auto& client = context.GetSubContext("Client");
	client.Write("Name", Name);
	client.Write("Version", Version);

	auto& renderer = context.GetSubContext("Render");
	std::string renderApiStr;
	switch (RenderAPI)
	{
	case Online::Render::API::SDL2D:
		renderApiStr = "SDL2D";
		break;
	case Online::Render::API::Unknown:
	default:
		renderApiStr = "SDL2D";
		break;
	}
	renderer.Write("API", renderApiStr);
	renderer.Write("EnableVSync", EnableVSync);

	auto& window = context.GetSubContext("Window");
	std::string windowPlatformStr;
	switch (WindowPlatform)
	{
	case Online::Window::Platform::SDL:
		windowPlatformStr = "SDL";
		break;
	case Online::Window::Platform::Invalid:
	default:
		windowPlatformStr = "SDL";
		break;
	}
	window.Write("Platform", windowPlatformStr);
	window.Write("Width", WindowWidth);
	window.Write("Height", WindowHeight);
}
void Online::Config::Configurator::ConfigInfo::Deserialize(const Online::Serialize::DeserializeContext& context)
{
	if (context.HasSubContext("Client"))
	{
		const auto& client = context.GetSubContext("Client");
		client.Read("Name", Name);
		client.Read("Version", Version);
	}
	if (context.HasSubContext("Render"))
	{
		const auto& renderer = context.GetSubContext("Render");
		std::string api;
		if (renderer.Read("API", api))
		{
			if (api == "SDL2D")
				RenderAPI = Render::API::SDL2D;
			else
				RenderAPI = Render::API::SDL2D;
		}
		renderer.Read("EnableVSync", EnableVSync);
	}
	if (context.HasSubContext("Window"))
	{
		const auto& window = context.GetSubContext("Window");
		window.Read("Width", WindowWidth);
		window.Read("Height", WindowHeight);

		std::string platform;
		if (window.Read("Platform", platform))
		{
			if (platform == "SDL")
				WindowPlatform = Window::Platform::SDL;
			else
				WindowPlatform = Window::Platform::SDL;
		}
	}
}

void Online::Config::Configurator::AnimationsInfo::Serialize(Online::Serialize::SerializeContext& context) const
{
	
}
void Online::Config::Configurator::AnimationsInfo::Deserialize(const Online::Serialize::DeserializeContext& context)
{
	animations.clear();

	if (!context.HasSubContext("animations"))
		return;

	const auto& arrCtx = context.GetSubContext("animations");

	size_t count = 0;
	if (!arrCtx.GetArraySize("", count))
		return;

	for (size_t i = 0; i < count; ++i)
	{
		const auto& itemCtx = arrCtx.GetArrayElement(i);

		AnimationInfo info;

		itemCtx.Read("name", info.name);
		itemCtx.Read("frameRate", info.frameRate);
		itemCtx.Read("looping", info.looping);

		const auto& gridCtx = itemCtx.GetSubContext("grid");
		gridCtx.Read("cols", info.grid[0]);
		gridCtx.Read("rows", info.grid[1]);

		const auto& framesCtx = itemCtx.GetSubContext("frames");
		size_t frameCount = 0;
		if (framesCtx.GetArraySize("", frameCount))
		{
			for (size_t f = 0; f < frameCount; ++f)
			{
				int val = 0;
				framesCtx.GetArrayElement(f).Read("", val);
				info.frames.push_back(val);
			}
		}

		animations.push_back(std::move(info));
	}
}
