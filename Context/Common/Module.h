#pragma once

#include <utility>

namespace Online::Runtime
{
    class Client;
    class Server;
    class CommonContext;
    class ClientContext;
    class ServerContext;

    template<typename T>
    class Module
    {
        friend class Online::Runtime::Client;
        friend class Online::Runtime::Server;
        friend class Online::Runtime::CommonContext;
        friend class Online::Runtime::ClientContext;
        friend class Online::Runtime::ServerContext;

    private:
        Module() = default;
        ~Module() = default;

    public:
        Module(const Module&) = delete;
        Module& operator=(const Module&) = delete;
        Module(Module&&) = delete;
        Module& operator=(Module&&) = delete;

    public:
        T* operator->() noexcept 
        {
            return kernel; 
        }
        const T* operator->() const noexcept 
        {
            return kernel; 
        }

    public:
        inline explicit constexpr operator bool() const noexcept
        {
            return kernel != nullptr;
        }

    public:
        T* Get() const noexcept { return kernel; }

    private:
        template<typename... Args>
        bool Create(Args&&... args)
        {
            kernel = T::Factory::Create(std::forward<Args>(args)...);
            return kernel != nullptr;
        }

        template<typename... Args>
        inline void Destroy(Args&&... args)
        {
            T::Factory::Destroy(kernel, std::forward<Args>(args)...);
            kernel = nullptr;
        }

        template<typename... Args>
        bool Initialize(Args&&... args)
        {
            return T::Lifecycle::Initialize(kernel, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void BeginFrame(Args&&... args)
        {
            T::Lifecycle::BeginFrame(kernel, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void FixedUpdate(Args&&... args)
        {
            T::Lifecycle::FixedUpdate(kernel, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void Update(Args&&... args)
        {
            T::Lifecycle::Update(kernel, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void LateUpdate(Args&&... args)
        {
            T::Lifecycle::LateUpdate(kernel, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void EndFrame(Args&&... args)
        {
            T::Lifecycle::EndFrame(kernel, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void Release(Args&&... args)
        {
            T::Lifecycle::Release(kernel, std::forward<Args>(args)...);
        }

    private:
        T* kernel = nullptr;
    };
}
