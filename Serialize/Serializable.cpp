#include <Serialize/Serializable.h>

#include <Serialize/Backend/MsgPack/MsgPackContext.h>
#include <Serialize/Backend/Json/CJsonContext.h>

#include <Log/Common/FuncTable.h>

#include <memory>
#include <exception>

namespace Online::Serialize
{
    namespace
    {
        std::unique_ptr<SerializeContext> CreateSerializeContext(API api) noexcept
        {
            switch (api)
            {
            case API::MsgPack: return std::make_unique<MsgPackSerializeContext>();
            case API::Json:    return std::make_unique<CJsonSerializeContext>();
            default:
                Online::Log::Error("Unknown serialize api, fallback to MsgPack");
                return std::make_unique<MsgPackSerializeContext>();
            }
        }

        std::unique_ptr<DeserializeContext> CreateDeserializeContext(API api) noexcept
        {
            switch (api)
            {
            case API::MsgPack: return std::make_unique<MsgPackDeserializeContext>();
            case API::Json:    return std::make_unique<CJsonDeserializeContext>();
            default:
                Online::Log::Error("Unknown deserialize api, fallback to MsgPack");
                return std::make_unique<MsgPackDeserializeContext>();
            }
        }
    }

    bool Serializable::SerializeToFile(const std::filesystem::path& file, API api) const
    {
        auto context = CreateSerializeContext(api);
        if (!context)
        {
            Online::Log::Error("serialize context create fail, path:" + file.string());
            return false;
        }

        try
        {
            context->OpenFile(file);
            Serialize(*context);
            context->Save();
            return true;
        }
        catch (const std::exception& e)
        {
            Online::Log::Error("serialize fail, path:" + file.string() + ", error:" + e.what());
            return false;
        }
    }

    bool Serializable::DeserializeFromFile(const std::filesystem::path& file, API api)
    {
        auto context = CreateDeserializeContext(api);
        if (!context)
        {
            Online::Log::Error("deserialize context create fail, path:" + file.string());
            return false;
        }

        try
        {
            context->LoadFile(file);
            Deserialize(*context);
            return true;
        }
        catch (const std::exception& e)
        {
            return false;
        }
    }

    bool Serializable::SerializeToString(std::string& out, API api) const
    {
        auto context = CreateSerializeContext(api);
        if (!context)
        {
            Online::Log::Error("serialize context create fail");
            return false;
        }

        try
        {
            Serialize(*context);
            out = context->ToString();
            return true;
        }
        catch (const std::exception& e)
        {
            Online::Log::Error(std::string("serialize to string fail, error:") + e.what());
            return false;
        }
    }

    bool Serializable::DeserializeFromString(const std::string& data, API api)
    {
        auto context = CreateDeserializeContext(api);
        if (!context)
        {
            Online::Log::Error("deserialize context create fail");
            return false;
        }

        try
        {
            context->ParseString(data);
            Deserialize(*context);
            return true; // ÐÞ¸´£º³É¹¦·µ»Ø true
        }
        catch (const std::exception& e)
        {
            Online::Log::Error(std::string("deserialize from string fail, error:") + e.what());
            return false;
        }
    }

    bool Serializable::SerializeToBytes(Blob& out, API api) const
    {
        auto context = CreateSerializeContext(api);
        if (!context)
        {
            Online::Log::Error("serialize context create fail");
            return false;
        }

        try
        {
            Serialize(*context);
            out = context->ToBytes();
            return true;
        }
        catch (const std::exception& e)
        {
            Online::Log::Error(std::string("serialize to bytes fail, error:") + e.what());
            return false;
        }
    }

    bool Serializable::DeserializeFromBytes(std::span<const std::byte> data, API api)
    {
        auto context = CreateDeserializeContext(api);
        if (!context)
        {
            Online::Log::Error("deserialize context create fail");
            return false;
        }

        try
        {
            context->ParseBytes(data);
            Deserialize(*context);
            return true;
        }
        catch (const std::exception& e)
        {
            Online::Log::Error(std::string("deserialize from bytes fail, error:") + e.what());
            return false;
        }
    }
}
