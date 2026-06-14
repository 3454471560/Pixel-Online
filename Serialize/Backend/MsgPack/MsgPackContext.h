#pragma once
#define MSGPACK_NO_BOOST
#include <msgpack.hpp>

#include <Serialize/Frontend/SerializeContext.h>
#include <Serialize/Frontend/DeserializeContext.h>

#include <glm.hpp>
#include <gtc/quaternion.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
#include <variant>

namespace Online::Serialize
{
    class MsgPackSerializeContext final : public SerializeContext
    {
    public:
        MsgPackSerializeContext();
        ~MsgPackSerializeContext() override = default;

        void OpenFile(const std::filesystem::path& file) override;
        void Save() override;

        SerializeContext& GetSubContext(const std::string& key) override;

        // --- 数组与对象嵌套接口 ---
        void BeginArray(const std::string& key) override;
        void EndArray() override;

        SerializeContext& WriteArrayObjectBegin() override;
        void WriteArrayObjectEnd() override;

        void BeginObject(const std::string& key) override;
        void EndObject() override;

        // --- 基础写入 ---
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

        // --- 数组元素写入 ---
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
        // 前向声明
        struct Node;

        // 使用 Variant 定义可以存储的所有数据类型
        using Value = std::variant<
            std::monostate, // 空
            std::string,
            float,
            int,
            bool,
            uint32_t,
            glm::vec2,
            glm::vec3,
            glm::vec4,
            glm::ivec2,
            glm::ivec3,
            glm::ivec4,
            std::unique_ptr<Node> // 递归：指向对象或数组
        >;

        // 节点结构：可以是 Map(对象) 或 Array(数组)
        struct Node {
            enum Type { Map, Array } type;

            // 如果是 Map
            std::vector<std::pair<std::string, Value>> mapData;
            // 如果是 Array
            std::vector<Value> arrayData;

            // 子上下文存储 (延迟构建)
            std::unordered_map<std::string, std::unique_ptr<MsgPackSerializeContext>> subs;

            Node(Type t) : type(t) {}
        };

        // 序列化状态栈
        struct WriteState {
            Node* node; // 当前正在操作的节点
            // 如果是在 Array 里写 Map，我们需要记住这个 Map 最后要放回 Array
            Value* pendingArraySlot = nullptr;
        };

        // 递归打包函数
        void PackValue(msgpack::packer<msgpack::sbuffer>& pk, const Value& val) const;
        void PackNode(msgpack::packer<msgpack::sbuffer>& pk, const Node& node) const;

        // 辅助：获取当前节点
        Node* GetCurrentNode() { return stateStack.back().node; }

    private:
        std::filesystem::path file;
        bool isSubContext = false;

        // 根节点 (始终是一个 Map)
        std::unique_ptr<Node> root;

        // 写入状态栈
        std::vector<WriteState> stateStack;
    };

    class MsgPackDeserializeContext final : public DeserializeContext
    {
    public:
        MsgPackDeserializeContext();
        MsgPackDeserializeContext(std::shared_ptr<msgpack::object_handle> owner, const msgpack::object* obj);
        ~MsgPackDeserializeContext() override = default;

        void LoadFile(const std::filesystem::path& file) override;
        void ParseString(const std::string& data) override;
        void ParseBytes(std::span<const std::byte> data) override;

        const DeserializeContext& GetSubContext(const std::string& key) const override;
        bool HasSubContext(const std::string& key) const override;
        std::vector<std::string> GetAllSubKeys() const override;

        // 数组接口
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
        bool Read(const std::string& key, uint16_t& out) const override;
        bool Read(const std::string& key, uint8_t& out) const override;

    private:
        const msgpack::object* Find(const std::string& key) const;

        static bool UnpackVec(const msgpack::object& o, glm::vec2& v);
        static bool UnpackVec(const msgpack::object& o, glm::vec3& v);
        static bool UnpackVec(const msgpack::object& o, glm::vec4& v);
        static bool UnpackVec(const msgpack::object& o, glm::ivec2& v);
        static bool UnpackVec(const msgpack::object& o, glm::ivec3& v);
        static bool UnpackVec(const msgpack::object& o, glm::ivec4& v);

    private:
        std::shared_ptr<msgpack::object_handle> owner;
        const msgpack::object* obj = nullptr;

        bool isSubContext = false;
        mutable std::unordered_map<std::string, std::unique_ptr<MsgPackDeserializeContext>> subs;
    };
}