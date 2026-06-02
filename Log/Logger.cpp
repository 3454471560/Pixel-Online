#include <Log/Logger.h>
#include <Thread/Common/FuncTable.h>
#include <Core/String/String.h>
#include <Core/Time/time.h>
#include <Core/Utils/File.h>

#include <cstring>
//#include <stdexcept>

namespace Online::Log
{
    bool Logger::Initialize()
    {
        const std::filesystem::path path = std::filesystem::path(Online::Core::GetExeDir()) / fileName;
        ofs.open(path, std::ios::out | std::ios::trunc);
        if (!ofs.is_open()) { throw std::runtime_error("Open Log File Fail! Path: " + path.string()); }
        front.reserve(1024 * 16);
        back.reserve(1024 * 16);
        isRunning.store(true, std::memory_order_release);
        logThread = Online::Thread::RegisterThread("Log", &Logger::BootstrapLogThread, this, nullptr);
        if (logThread == Online::Core::Thread::Identifier::Invalid) { throw std::runtime_error("Log Thread Register Fail!"); }
        return true;
    }

    void Logger::Release()
    {
        isRunning.store(false, std::memory_order_release);
        cond.notify_one();
        Online::Thread::UnregisterThread(logThread);
        Flush();
        ofs.close();
    }

    void Logger::LogThread()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!cond.wait_for(lock, WaitTimeoutThreshold, [this] {return !front.empty() || !isRunning.load(std::memory_order_acquire); }))
            {
                continue;
            };

            std::swap(front, back);
            frontMessageCount = 0;
            lock.unlock();

            if (!back.empty())
            {
                ParseBuffer(back);
                back.clear();
                ofs.flush();
            }

            if (!isRunning.load(std::memory_order_acquire))
            {
                break;
            }
        }
    }

    void Logger::Write(Online::Log::LogLevel level, std::string_view message)
    {
        if (message.empty()) { return; }

        const time_t currentTime = std::time(nullptr);
        const uint32_t messageLength = static_cast<uint32_t>(message.size());
        const Online::Core::Thread::Identifier currentThreadId = Online::Core::Thread::GetCurrentThreadId();

        {
            std::lock_guard<std::mutex> lock(mutex);

            const size_t neededBytes = sizeof(time_t) + sizeof(Online::Core::Thread::Identifier) +
                sizeof(Online::Log::LogLevel) + sizeof(uint32_t) + messageLength;

            const size_t currentSize = front.size();
            front.resize(currentSize + neededBytes);
            std::byte* writePtr = front.data() + currentSize;

            auto writeInfo = [&](const void* data, size_t size)
                {
                    std::memcpy(writePtr, data, size);
                    writePtr += size;
                };

            writeInfo(&currentTime, sizeof(time_t));
            writeInfo(&currentThreadId, sizeof(Online::Core::Thread::Identifier));
            writeInfo(&level, sizeof(Online::Log::LogLevel));
            writeInfo(&messageLength, sizeof(uint32_t));
            writeInfo(message.data(), messageLength);

            if (++frontMessageCount >= WakeupLogCountThreshold)
            {
                cond.notify_one();
            }
        }
    }

    void Logger::ParseBuffer(const std::vector<std::byte>& buffer)
    {
        size_t offset = 0;
        const size_t totalSize = buffer.size();

        while (offset < totalSize)
        {
            time_t logTime;
            if (offset + sizeof(time_t) > totalSize) { throw std::runtime_error("Log Thread Out-of-bounds access"); }
            std::memcpy(&logTime, buffer.data() + offset, sizeof(time_t));
            offset += sizeof(time_t);

            Online::Core::Thread::Identifier threadId;
            if (offset + sizeof(Online::Core::Thread::Identifier) > totalSize) { throw std::runtime_error("Log Thread Out-of-bounds access"); }
            std::memcpy(&threadId, buffer.data() + offset, sizeof(Online::Core::Thread::Identifier));
            offset += sizeof(Online::Core::Thread::Identifier);

            Online::Log::LogLevel level;
            if (offset + sizeof(Online::Log::LogLevel) > totalSize) { throw std::runtime_error("Log Thread Out-of-bounds access"); }
            std::memcpy(&level, buffer.data() + offset, sizeof(Online::Log::LogLevel));
            offset += sizeof(Online::Log::LogLevel);

            uint32_t msgLen;
            if (offset + sizeof(uint32_t) > totalSize) { throw std::runtime_error("Log Thread Out-of-bounds access"); }
            std::memcpy(&msgLen, buffer.data() + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);

            std::string_view msgText(reinterpret_cast<const char*>(buffer.data() + offset), msgLen);
            offset += msgLen;

            ofs << "[" << Online::Core::Data(logTime) << "]["
                << ToString(level) << "]["
                << Online::Thread::GetThreadName(threadId) << "]"
                << msgText << "\n";
        }
    }

    void Logger::Flush()
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            ParseBuffer(back);
            ParseBuffer(front);
            back.clear();
            front.clear();
            frontMessageCount = 0;
        }

        ofs.flush();
    }
}