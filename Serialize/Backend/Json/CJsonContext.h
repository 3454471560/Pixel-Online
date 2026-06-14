#pragma once
#include <Core/Allocate/Allocate.h>
#include <Serialize/Frontend/SerializeContext.h>
#include <Serialize/Frontend/DeserializeContext.h>

#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include <cJSON.h>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <span>

namespace Online::Serialize
{
    struct CJsonDeleter
    {
        void operator()(cJSON* p) const noexcept { if (p) cJSON_Delete(p); }
    };

    using CJsonPtr = std::unique_ptr<cJSON, CJsonDeleter>;

    class CJsonSerializeContext final : public SerializeContext
    {
    public:
        CJsonSerializeContext();
        ~CJsonSerializeContext() override = default;

        struct WriteState
        {
            enum Type { Object, Array } type;
            cJSON* node;
        };

        void OpenFile(const std::filesystem::path& file) override;
        void Save() override;
        void SetFormatted(bool enable) { formatted = enable; }

        SerializeContext& GetSubContext(const std::string& key) override;

        void BeginObject(const std::string& key) override;
        void EndObject() override;

        void BeginArray(const std::string& key) override;
        void EndArray() override;

        SerializeContext& WriteArrayObjectBegin() override;
        [[deprecated("WriteArrayObjectEnd is no longer needed and has no effect")]]
        void WriteArrayObjectEnd() override;

        void Write(const std::string& key, const std::string& value) override;
        void Write(const std::string& key, const char* value) override;
        void Write(const std::string& key, float value) override;
        void Write(const std::string& key, glm::vec2 value) override;
        void Write(const std::string& key, glm::vec3 value) override;
        void Write(const std::string& key, glm::vec4 value) override;
        void Write(const std::string& key, int value) override;
        void Write(const std::string& key, glm::ivec2 value) override;
        void Write(const std::string& key, glm::ivec3 value) override;
        void Write(const std::string& key, glm::ivec4 value) override;
        void Write(const std::string& key, bool value) override;
        void Write(const std::string& key, uint32_t value) override;

        void WriteArrayItem(const std::string& value) override;
        void WriteArrayItem(const char* value) override;
        void WriteArrayItem(float value) override;
        void WriteArrayItem(int value) override;
        void WriteArrayItem(bool value) override;
        void WriteArrayItem(uint32_t value) override;
        void WriteArrayItem(glm::vec2 value) override;
        void WriteArrayItem(glm::vec3 value) override;
        void WriteArrayItem(glm::vec4 value) override;
        void WriteArrayItem(glm::ivec2 value) override;
        void WriteArrayItem(glm::ivec3 value) override;
        void WriteArrayItem(glm::ivec4 value) override;

        std::vector<std::byte> ToBytes() const override;
        std::string ToString() const override;

    private:
        explicit CJsonSerializeContext(cJSON* targetNode, CJsonSerializeContext* parent);

        void EnsureRootObject();
        void WriteItem(const std::string& key, cJSON* item);

        static cJSON* PackVec(glm::vec2 v);
        static cJSON* PackVec(glm::vec3 v);
        static cJSON* PackVec(glm::vec4 v);
        static cJSON* PackVec(glm::ivec2 v);
        static cJSON* PackVec(glm::ivec3 v);
        static cJSON* PackVec(glm::ivec4 v);

    private:
        std::filesystem::path file;
        bool formatted = true;
        bool isSubContext = false;

        // 只有主上下文拥有root的所有权
        CJsonPtr root;

        // 每个上下文独立的写入栈（解决嵌套污染问题）
        std::vector<WriteState> writeStack;

        // 子上下文缓存（避免重复创建，纯视图不持有所有权）
        std::unordered_map<std::string, CJsonSerializeContext*> subs;

        // 父上下文指针（主上下文为nullptr）
        CJsonSerializeContext* parent = nullptr;
    };

    class CJsonDeserializeContext final : public DeserializeContext
    {
    public:
        CJsonDeserializeContext();
        CJsonDeserializeContext(std::shared_ptr<CJsonPtr> owner, const cJSON* obj);
        ~CJsonDeserializeContext() override = default;

        void LoadFile(const std::filesystem::path& file) override;
        void ParseString(const std::string& data) override;
        void ParseBytes(std::span<const std::byte> data) override;

        const DeserializeContext& GetSubContext(const std::string& key) const override;
        bool HasSubContext(const std::string& key) const override;
        std::vector<std::string> GetAllSubKeys() const override;

        bool GetArraySize(const std::string& key, size_t& outSize) const override;
        const DeserializeContext& GetArrayElement(size_t index) const override;
        bool HasArrayElement(size_t index) const override;

        bool Read(const std::string& key, std::string& out) const override;
        bool Read(const std::string& key, float& out) const override;
        bool Read(const std::string& key, glm::vec2& out) const override;
        bool Read(const std::string& key, glm::vec3& out) const override;
        bool Read(const std::string& key, glm::vec4& out) const override;
        bool Read(const std::string& key, int& out) const override;
        bool Read(const std::string& key, glm::ivec2& out) const override;
        bool Read(const std::string& key, glm::ivec3& out) const override;
        bool Read(const std::string& key, glm::ivec4& out) const override;
        bool Read(const std::string& key, bool& out) const override;
        bool Read(const std::string& key, uint32_t& out) const override;
        bool Read(const std::string& key, uint8_t& out) const override;
        bool Read(const std::string& key, uint16_t& out) const override;

    private:
        const cJSON* Find(const std::string& key) const;

        static bool UnpackVec(const cJSON* arr, glm::vec2& v);
        static bool UnpackVec(const cJSON* arr, glm::vec3& v);
        static bool UnpackVec(const cJSON* arr, glm::vec4& v);
        static bool UnpackVec(const cJSON* arr, glm::ivec2& v);
        static bool UnpackVec(const cJSON* arr, glm::ivec3& v);
        static bool UnpackVec(const cJSON* arr, glm::ivec4& v);

    private:
        std::shared_ptr<CJsonPtr> owner;
        const cJSON* obj = nullptr;

        bool isSubContext = false;
        mutable std::unordered_map<std::string, std::unique_ptr<CJsonDeserializeContext>> subs;
    };
}