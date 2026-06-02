#pragma once
#include<Core/Singleton/Singleton.h>

#include<stdexcept>
#include<type_traits>

namespace Online::Runtime { class Client; class Server; }

namespace Online::Time { class Chronometer; }
namespace Online::Log { class Logger; }
namespace Online::Thread { class ThreadTracker; }
namespace Online::Event { class EventDispatcher; }
namespace Online::Task { class TaskScheduler; }
namespace Online::Physics { class PhysicsSimulator; }
namespace Online::Game { class GameWorld; }
namespace Online::Script { class LifeCycleTable; }

namespace Online::Runtime
{
	template<typename T> class Module;
	template<typename T> struct FuncTable;

	class Context :public Online::Core::Singleton<Context>
	{
	public:
		Context() = default;
		~Context() = default;
		
	private:
		struct Modules
		{

			Online::Runtime::Module<Online::Log::Logger>* Logger = nullptr;
			Online::Runtime::Module<Online::Event::EventDispatcher>* EventDispatcher = nullptr;
			Online::Runtime::Module<Online::Thread::ThreadTracker>* ThreadTracker = nullptr;
			Online::Runtime::Module<Online::Time::Chronometer>* Chronometer = nullptr;
			Online::Runtime::Module<Online::Physics::PhysicsSimulator>* PhysicsSimulator = nullptr;
			Online::Runtime::Module<Online::Task::TaskScheduler>* TaskScheduler = nullptr;
			Online::Runtime::Module<Online::Script::LifeCycleTable>* LifeCycleTable = nullptr;

			inline bool Check() const
			{
				if (!Chronometer) { throw std::runtime_error("Context [Module] miss [Chronometer]"); }
				if (!EventDispatcher) { throw std::runtime_error("Context [Module] miss [EventDispatcher]"); }
				if (!Logger) { throw std::runtime_error("Context [Module] miss [Logger]"); }
				if (!ThreadTracker) { throw std::runtime_error("Context [Module] miss [ThreadTracker]"); }
				if (!PhysicsSimulator) { throw std::runtime_error("Context [Module] miss [PhysicsSimulator]"); }
				if (!TaskScheduler) { throw std::runtime_error("Context [Module] miss [TaskScheduler]"); }
				if (!LifeCycleTable) { throw std::runtime_error("Context [Module] miss [LifeCycleTable]"); }

				return true;
			}
			inline void UnRegister() noexcept
			{
				PhysicsSimulator = nullptr;
				Chronometer = nullptr;
				EventDispatcher = nullptr;
				Logger = nullptr;
				ThreadTracker = nullptr;
				TaskScheduler = nullptr;
				LifeCycleTable = nullptr;
			}
		};
		struct FuncTables
		{
			Online::Runtime::FuncTable<Online::Time::Chronometer>* Chronometer = nullptr;
			Online::Runtime::FuncTable<Online::Log::Logger>* Logger = nullptr;
			Online::Runtime::FuncTable<Online::Thread::ThreadTracker>* ThreadTracker = nullptr;
			Online::Runtime::FuncTable<Online::Event::EventDispatcher>* EventDispatcher = nullptr;
			Online::Runtime::FuncTable<Online::Task::TaskScheduler>* TaskScheduler = nullptr;
			Online::Runtime::FuncTable<Online::Physics::PhysicsSimulator>* PhysicsSimulator = nullptr;
			Online::Runtime::FuncTable<Online::Script::LifeCycleTable>* LifeCycleTable = nullptr;

			inline bool Check() const
			{
				if (!EventDispatcher) { throw std::runtime_error("Context [FuncTable] miss [EventDispatcher]"); }
				if (!Logger) { throw std::runtime_error("Context [FuncTable] miss [Logger]"); }
				if (!ThreadTracker) { throw std::runtime_error("Context [FuncTable] miss [ThreadTracker]"); }
				if (!Chronometer) { throw std::runtime_error("Context [FuncTable] miss [Chronometer]"); }
				if (!TaskScheduler) { throw std::runtime_error("Context [FuncTable] miss [TaskScheduler]"); }
				if (!PhysicsSimulator) { throw std::runtime_error("Context [FuncTable] miss [PhysicsSimulator]"); }
				if (!LifeCycleTable) { throw std::runtime_error("Context [FuncTable] miss [LifeCycleTable]"); }

				return true;
			}
			inline void UnRegister() noexcept
			{
				Chronometer = nullptr;
				Logger = nullptr;
				ThreadTracker = nullptr;
				EventDispatcher = nullptr;
				TaskScheduler = nullptr;
				PhysicsSimulator = nullptr;
				LifeCycleTable = nullptr;
			}
		};

	private:
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(Context&&) = delete;

	public:
		template<typename T>
		inline void RegisterCommonModule(Module<T>* module) noexcept
		{
			if constexpr (std::is_same_v<T, Online::Log::Logger>) { commonModules.Logger = module; }
			else if constexpr (std::is_same_v<T, Online::Event::EventDispatcher>) { commonModules.EventDispatcher = module; }
			else if constexpr (std::is_same_v<T, Online::Thread::ThreadTracker>) { commonModules.ThreadTracker = module; }
			else if constexpr (std::is_same_v<T, Online::Time::Chronometer>) { commonModules.Chronometer = module; }
			else if constexpr (std::is_same_v<T, Online::Physics::PhysicsSimulator>) { commonModules.PhysicsSimulator = module; }
			else if constexpr (std::is_same_v<T, Online::Task::TaskScheduler>) { commonModules.TaskScheduler = module; }
			else if constexpr (std::is_same_v<T, Online::Script::LifeCycleTable>) { commonModules.LifeCycleTable = module; }
		}

		template<typename T>
		inline void RegisterCommonFuncTable(FuncTable<T>* table) noexcept
		{
			if constexpr (std::is_same_v<T, Online::Time::Chronometer>) { commonFuncTables.Chronometer = table; }
			else if constexpr (std::is_same_v<T, Online::Log::Logger>) { commonFuncTables.Logger = table; }
			else if constexpr (std::is_same_v<T, Online::Thread::ThreadTracker>) { commonFuncTables.ThreadTracker = table; }
			else if constexpr (std::is_same_v<T, Online::Event::EventDispatcher>) { commonFuncTables.EventDispatcher = table; }
			else if constexpr (std::is_same_v<T, Online::Task::TaskScheduler>) { commonFuncTables.TaskScheduler = table; }
			else if constexpr (std::is_same_v<T, Online::Physics::PhysicsSimulator>) { commonFuncTables.PhysicsSimulator = table; }
			else if constexpr (std::is_same_v<T, Online::Script::LifeCycleTable>) { commonFuncTables.LifeCycleTable = table; }
		}

		template<typename T>
		inline Online::Runtime::Module<T>& GetModule() const noexcept
		{
			static_assert(
				std::is_same_v<T, Online::Task::TaskScheduler> ||
				std::is_same_v<T, Online::Log::Logger> ||
				std::is_same_v<T, Online::Event::EventDispatcher> ||
				std::is_same_v<T, Online::Thread::ThreadTracker> ||
				std::is_same_v<T, Online::Time::Chronometer> ||
				std::is_same_v<T, Online::Physics::PhysicsSimulator> ||
				std::is_same_v<T, Online::Task::TaskScheduler> ||
				std::is_same_v<T, Online::Script::LifeCycleTable>
				, "Context::GetModule<T>(): T must be a valid Runtime Module Type");

			if constexpr (std::is_same_v<T, Online::Log::Logger>) { return *commonModules.Logger; }
			else if constexpr (std::is_same_v<T, Online::Event::EventDispatcher>) { return *commonModules.EventDispatcher; }
			else if constexpr (std::is_same_v<T, Online::Thread::ThreadTracker>) { return *commonModules.ThreadTracker; }
			else if constexpr (std::is_same_v<T, Online::Time::Chronometer>) { return *commonModules.Chronometer; }
			else if constexpr (std::is_same_v<T, Online::Physics::PhysicsSimulator>) { return *commonModules.PhysicsSimulator; }
			else if constexpr (std::is_same_v<T, Online::Task::TaskScheduler>) { return *commonModules.TaskScheduler; }
			else if constexpr (std::is_same_v<T, Online::Script::LifeCycleTable>) { return *commonModules.LifeCycleTable; }
		}

		template<typename T>
		inline Online::Runtime::FuncTable<T>& GetFuncTable() const noexcept
		{
			static_assert(
				std::is_same_v<T, Online::Time::Chronometer> ||
				std::is_same_v<T, Online::Log::Logger> ||
				std::is_same_v<T, Online::Thread::ThreadTracker> ||
				std::is_same_v<T, Online::Event::EventDispatcher> ||
				std::is_same_v<T, Online::Task::TaskScheduler> ||
				std::is_same_v<T, Online::Physics::PhysicsSimulator> ||
				std::is_same_v<T, Online::Script::LifeCycleTable>

				, "Context::GetFuncTable<T>(): T must be a valid Runtime FuncTable Type");

			if constexpr (std::is_same_v<T, Online::Time::Chronometer>) { return *commonFuncTables.Chronometer; }
			else if constexpr (std::is_same_v<T, Online::Log::Logger>) { return *commonFuncTables.Logger; }
			else if constexpr (std::is_same_v<T, Online::Thread::ThreadTracker>) { return *commonFuncTables.ThreadTracker; }
			else if constexpr (std::is_same_v<T, Online::Event::EventDispatcher>) { return *commonFuncTables.EventDispatcher; }
			else if constexpr (std::is_same_v<T, Online::Task::TaskScheduler>) { return *commonFuncTables.TaskScheduler; }
			else if constexpr (std::is_same_v<T, Online::Physics::PhysicsSimulator>) { return *commonFuncTables.PhysicsSimulator; }
			else if constexpr (std::is_same_v<T, Online::Script::LifeCycleTable>) { return *commonFuncTables.LifeCycleTable; }
		}

		inline bool Check() const
		{
			return commonModules.Check() && commonFuncTables.Check();
		}

		inline void UnRegister() noexcept
		{
			commonModules.UnRegister();
			commonFuncTables.UnRegister();
		}

	public:
		Modules commonModules;
		FuncTables commonFuncTables;
	};
}

extern template class Online::Core::Singleton<Online::Runtime::Context>;