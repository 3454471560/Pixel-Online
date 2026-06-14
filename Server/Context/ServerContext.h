#pragma once
#include <Context/Context.h>
#include <Core/Singleton/Singleton.h>

namespace Online::Net::Server { class HybridServer; }

namespace Online::Runtime
{
    class Server;

    class ServerContext final : public Online::Core::Singleton<ServerContext>
    {
        friend class Online::Core::Singleton<ServerContext>;
        friend class Online::Runtime::Server;
    public:
        ServerContext() = default;
        ~ServerContext() = default;

    private:
        ServerContext(const ServerContext&) = delete;
        ServerContext& operator=(const ServerContext&) = delete;
        ServerContext(ServerContext&&) = delete;
        ServerContext& operator=(ServerContext&&) = delete;

    public:

        template<typename T>
        inline Module<T>& GetServerModule() const noexcept
        {
            static_assert(
                std::is_same_v<T, Online::Net::Server::HybridServer>
                , "ServerContext::GetServerModule<T>(): T must be a server-specific module type"
                );

            if constexpr (std::is_same_v<T, Online::Net::Server::HybridServer>) { return *serverModules.NetworkServer; }
        }

        template<typename T>
        inline FuncTable<T>& GetServerFuncTable() const noexcept
        {
            static_assert(
                std::is_same_v<T, Online::Net::Server::HybridServer>
                , "ServerContext::GetServerFuncTable<T>(): T must be a server-specific FuncTable type"
                );

            if constexpr (std::is_same_v<T, Online::Net::Server::HybridServer>) { return *serverFuncTables.NetworkServer; }
        }

        inline bool Check() const
        {
            return serverModules.Check() && serverFuncTables.Check();
        }

        inline void UnRegister() noexcept
        {
            serverModules.UnRegister();
            serverFuncTables.UnRegister();
        }

    public:
        struct ServerModules
        {
            Module<Online::Net::Server::HybridServer>* NetworkServer = nullptr;

            inline bool Check() const
            {
                if (!NetworkServer) { throw std::runtime_error("Context [Module] miss [AssetHub]"); }

                return true;
            }
            inline void UnRegister() noexcept
            {
                NetworkServer = nullptr;
            }
        };

        struct ServerFuncTables
        {
            FuncTable<Online::Net::Server::HybridServer>* NetworkServer = nullptr;

            inline bool Check() const
            {
                if (!NetworkServer) { throw std::runtime_error("Context [FuncTable] miss [NetworkServer]"); }

                return true;
            }
            inline void UnRegister() noexcept
            {
                NetworkServer = nullptr;

            }
        };

        ServerModules serverModules;
        ServerFuncTables serverFuncTables;
    };
}

extern template class Online::Core::Singleton<Online::Runtime::ServerContext>;