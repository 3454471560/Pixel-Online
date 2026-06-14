#pragma once
#include <Serialize/Common/SerializeAPI.h>
#include <Serialize/Frontend/SerializeContext.h>
#include <Serialize/Frontend/DeserializeContext.h>

#include <filesystem>
#include <string>
#include <span>

namespace Online::Serialize
{
    class Serializable
    {
    public:
        Serializable() = default;
        virtual ~Serializable() = default;

    public:
        virtual void Serialize(SerializeContext&) const = 0;
        virtual void Deserialize(const DeserializeContext&) = 0;

    public:
        bool SerializeToFile(const std::filesystem::path& file, API api = API::Json) const;
        bool DeserializeFromFile(const std::filesystem::path& file, API api = API::Json);

        bool SerializeToString(std::string& out, API api = API::Json) const;
        bool DeserializeFromString(const std::string& data, API api = API::Json);

        bool SerializeToBytes(std::vector<std::byte>& out, API api = API::Json) const;
        bool DeserializeFromBytes(std::span<const std::byte> data, API api = API::Json);
    };
}
