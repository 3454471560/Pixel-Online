#pragma once
#include <Context/Context.h>
#include <Core//Singleton/Singleton.h>

namespace Online::Asset  { class AssetHub; }
namespace Online::Render { class Renderer; class RenderPipeline; }
namespace Online::Window { class Window; }
namespace Online::Net::Client   { class HybridClient; }
namespace Online::Audio { class AudioPlayer; }

namespace Online::Runtime
{
    class Client;

	class ClientContext final : public Online::Core::Singleton<ClientContext>
    {
        friend class Online::Core::Singleton<ClientContext>;
        friend class Online::Runtime::Client;
    public:
        ClientContext() = default;
        ~ClientContext() = default;

    private:
        ClientContext(const ClientContext&) = delete;
        ClientContext& operator=(const ClientContext&) = delete;
        ClientContext(ClientContext&&) = delete;
        ClientContext& operator=(ClientContext&&) = delete;

    public:

        template<typename T>
        inline Module<T>& GetClientModule() const noexcept
        {
            static_assert(
                std::is_same_v<T, Online::Asset::AssetHub> ||
                std::is_same_v<T, Online::Render::Renderer> ||
                std::is_same_v<T, Online::Render::RenderPipeline> ||
                std::is_same_v<T, Online::Window::Window> ||
                std::is_same_v<T, Online::Net::Client::HybridClient> ||
                std::is_same_v<T, Online::Audio::AudioPlayer>
                ,"ClientContext::GetClientModule<T>(): T must be a client-specific module type"
                );

            if constexpr (std::is_same_v<T, Online::Asset::AssetHub>) { return *clientModules.AssetHub; }
            else if constexpr (std::is_same_v<T, Online::Render::Renderer>) { return *clientModules.Renderer; }
            else if constexpr (std::is_same_v<T, Online::Render::RenderPipeline>) { return *clientModules.RenderPipeline; }
            else if constexpr (std::is_same_v<T, Online::Window::Window>) { return *clientModules.Window; }
            else if constexpr (std::is_same_v<T, Online::Net::Client::HybridClient>) { return *clientModules.NetworkClient; }
            else if constexpr (std::is_same_v<T, Online::Audio::AudioPlayer>) { return *clientModules.AudioPlayer; }
        }

        template<typename T>
        inline FuncTable<T>& GetClientFuncTable() const noexcept
        {
            static_assert(
                std::is_same_v<T, Online::Input::InputMonitor> ||
                std::is_same_v<T, Online::Window::Window> ||
                std::is_same_v<T, Online::Asset::AssetHub> ||
                std::is_same_v<T, Online::Net::Client::HybridClient> 
                ,"ClientContext::GetClientFuncTable<T>(): T must be a client-specific FuncTable type"
                );

            if constexpr (std::is_same_v<T, Online::Window::Window>) { return *clientFuncTables.Window; }
            else if constexpr (std::is_same_v<T, Online::Asset::AssetHub>) { return *clientFuncTables.AssetHub; }
            else if constexpr (std::is_same_v<T, Online::Net::Client::HybridClient>) { return *clientFuncTables.NetworkClient; }
        }

        inline bool Check() const
        {
            return clientModules.Check() && clientFuncTables.Check();
        }

        inline void UnRegister() noexcept
        {
            clientModules.UnRegister();
            clientFuncTables.UnRegister();
        }

    public:
        struct ClientModules
        {
            Module<Online::Asset::AssetHub>*        AssetHub = nullptr;
            Module<Online::Render::Renderer>*       Renderer = nullptr;
            Module<Online::Render::RenderPipeline>* RenderPipeline = nullptr;
            Module<Online::Window::Window>*         Window = nullptr;
            Module<Online::Net::Client::HybridClient>* NetworkClient = nullptr;
            Module<Online::Audio::AudioPlayer>*     AudioPlayer = nullptr;

            inline bool Check() const
            {
				if (!AssetHub)      { throw std::runtime_error("Context [Module] miss [AssetHub]"); }
                if (!Renderer)      { throw std::runtime_error("Context [Module] miss [Renderer]"); }
                if (!RenderPipeline){ throw std::runtime_error("Context [Module] miss [RenderPipeline]"); }
                if (!Window)        { throw std::runtime_error("Context [Module] miss [Window]"); }
                if (!NetworkClient) { throw std::runtime_error("Context [Module] miss [NetworkClient]"); }
                if (!AudioPlayer)   { throw std::runtime_error("Context [Module] miss [AudioPlayer]"); }

                return true;
            }
            inline void UnRegister() noexcept
            {
                AssetHub       = nullptr;
                Renderer       = nullptr;
                RenderPipeline = nullptr;
                Window         = nullptr;
				NetworkClient  = nullptr;
                AudioPlayer    = nullptr;
            }
        };

        struct ClientFuncTables
        {
            FuncTable<Online::Window::Window>*       Window = nullptr;
            FuncTable<Online::Asset::AssetHub>*      AssetHub = nullptr;
            FuncTable<Online::Net::Client::HybridClient>* NetworkClient = nullptr;

            inline bool Check() const
            {
                if (!Window)       { throw std::runtime_error("Context [FuncTable] miss [Window]"); }
                if (!AssetHub)     { throw std::runtime_error("Context [FuncTable] miss [AssetHub]"); }
                if (!NetworkClient){ throw std::runtime_error("Context [FuncTable] miss [NetworkClient]"); }

                return true;
            }
            inline void UnRegister() noexcept
            {
                Window       = nullptr;
                AssetHub     = nullptr;
                NetworkClient= nullptr;
            }
        };

        ClientModules clientModules;
        ClientFuncTables clientFuncTables;
    };
}

extern template class Online::Core::Singleton<Online::Runtime::ClientContext>;