#pragma once
#include <filesystem>
#include <string>

#include <glm.hpp>
#include <gtc/quaternion.hpp>

namespace Online::Serialize
{
    class SerializeContext
    {
    public:
        SerializeContext() = default;
        virtual ~SerializeContext() = default;

    public:
        virtual void OpenFile(const std::filesystem::path& file) = 0;
        virtual void Save() = 0;

    public:
        virtual SerializeContext& GetSubContext(const std::string& key) = 0;

        virtual SerializeContext& WriteArrayObjectBegin() = 0;
        virtual void WriteArrayObjectEnd() = 0;

        virtual void BeginObject(const std::string& key) = 0;
        virtual void EndObject() = 0;
    public:
        virtual void Write(const std::string& key, const std::string& value) = 0;
        virtual void Write(const std::string& key, const char* value) = 0;
        virtual void Write(const std::string& key, float value) = 0;
        virtual void Write(const std::string& key, glm::vec2 value) = 0;
        virtual void Write(const std::string& key, glm::vec3 value) = 0;
        virtual void Write(const std::string& key, glm::vec4 value) = 0;
        virtual void Write(const std::string& key, int value) = 0;
        virtual void Write(const std::string& key, glm::ivec2 value) = 0;
        virtual void Write(const std::string& key, glm::ivec3 value) = 0;
        virtual void Write(const std::string& key, glm::ivec4 value) = 0;
        virtual void Write(const std::string& key, bool value) = 0;
        virtual void Write(const std::string& key, uint32_t value) = 0;

    public:
        virtual void BeginArray(const std::string& key) = 0;
        virtual void EndArray() = 0;
        virtual void WriteArrayItem(const std::string& value) = 0;
        virtual void WriteArrayItem(const char* value) = 0;
        virtual void WriteArrayItem(float value) = 0;
        virtual void WriteArrayItem(int value) = 0;
        virtual void WriteArrayItem(bool value) = 0;
        virtual void WriteArrayItem(uint32_t value) = 0;
        virtual void WriteArrayItem(glm::vec2 value) = 0;
        virtual void WriteArrayItem(glm::vec3 value) = 0;
        virtual void WriteArrayItem(glm::vec4 value) = 0;
        virtual void WriteArrayItem(glm::ivec2 value) = 0;
        virtual void WriteArrayItem(glm::ivec3 value) = 0;
        virtual void WriteArrayItem(glm::ivec4 value) = 0;

    public:
        virtual std::vector<std::byte> ToBytes() const = 0;
        virtual std::string ToString() const = 0;
    };
}
