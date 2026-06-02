#include <Serialize/Backend/MsgPack/MsgPackContext.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <limits>

namespace Online::Serialize
{
    // -------------------------------------------------------------------------
    // MsgPackSerializeContext 构造与初始化
    // -------------------------------------------------------------------------

    MsgPackSerializeContext::MsgPackSerializeContext()
    {
        // 初始化根节点为 Map
        root = std::make_unique<Node>(Node::Map);
        stateStack.push_back({ root.get(), nullptr });
    }

    void MsgPackSerializeContext::OpenFile(const std::filesystem::path& file)
    {
        if (isSubContext)
            throw std::runtime_error("MsgPackSerializeContext: sub context cannot OpenFile");

        this->file = file;

        // 重置状态
        root = std::make_unique<Node>(Node::Map);
        stateStack.clear();
        stateStack.push_back({ root.get(), nullptr });
    }

    // -------------------------------------------------------------------------
    // 子上下文 (GetSubContext)
    // -------------------------------------------------------------------------

    SerializeContext& MsgPackSerializeContext::GetSubContext(const std::string& key)
    {
        Node* current = GetCurrentNode();
        if (current->type != Node::Map) {
            throw std::runtime_error("MsgPackSerializeContext: GetSubContext must be called on a Map/Object");
        }

        // 查找是否已存在
        auto& subs = current->subs;
        if (subs.count(key) == 0)
        {
            auto sub = std::make_unique<MsgPackSerializeContext>();
            sub->isSubContext = true;
            subs[key] = std::move(sub);
        }
        return *subs[key];
    }

    // -------------------------------------------------------------------------
    // 数组实现 (BeginArray / EndArray)
    // -------------------------------------------------------------------------

    void MsgPackSerializeContext::BeginArray(const std::string& key)
    {
        Node* current = GetCurrentNode();
        if (current->type != Node::Map) {
            throw std::runtime_error("MsgPackSerializeContext: BeginArray must be called on a Map");
        }

        // 1. 创建一个 Array 类型的 Node
        auto arrayNode = std::make_unique<Node>(Node::Array);
        Node* arrayPtr = arrayNode.get();

        // 2. 将这个 Node 存入当前 Map
        current->mapData.emplace_back(key, std::move(arrayNode));

        // 3. 压入栈
        stateStack.push_back({ arrayPtr, nullptr });
    }

    void MsgPackSerializeContext::EndArray()
    {
        if (stateStack.size() <= 1) {
            throw std::runtime_error("MsgPackSerializeContext: EndArray called without BeginArray");
        }

        Node* current = GetCurrentNode();
        if (current->type != Node::Array) {
            throw std::runtime_error("MsgPackSerializeContext: EndArray called but not in array state");
        }

        stateStack.pop_back();
    }

    // -------------------------------------------------------------------------
    // 数组内对象 (WriteArrayObjectBegin / End)
    // -------------------------------------------------------------------------

    SerializeContext& MsgPackSerializeContext::WriteArrayObjectBegin()
    {
        Node* current = GetCurrentNode();
        if (current->type != Node::Array) {
            throw std::runtime_error("MsgPackSerializeContext: WriteArrayObjectBegin must be called in an Array");
        }

        // 1. 创建一个 Map 类型的 Node
        auto objNode = std::make_unique<Node>(Node::Map);
        Node* objPtr = objNode.get();

        // 2. 将这个 Node 存入 Array (先占个位置)
        current->arrayData.emplace_back(std::move(objNode));

        // 3. 获取刚才存入的 Value 的指针 (因为 variant 移动后，我们需要引用刚刚存入的那个)
        // 注意：上面 emplace_back 后，vector 可能扩容导致引用失效，所以我们不能直接存指针。
        // 安全的做法是：压栈时记录 index，或者信任 variant 的移动。
        // 这里我们用一个更简单的逻辑：栈里的 node 指针依然有效，因为 unique_ptr 只是移动了所有权，对象本身还在。

        // 压入栈
        stateStack.push_back({ objPtr, nullptr });
        return *this;
    }

    void MsgPackSerializeContext::WriteArrayObjectEnd()
    {
        if (stateStack.size() <= 1) {
            throw std::runtime_error("MsgPackSerializeContext: WriteArrayObjectEnd mismatch");
        }

        Node* current = GetCurrentNode();
        if (current->type != Node::Map) {
            throw std::runtime_error("MsgPackSerializeContext: WriteArrayObjectEnd called but not in object state");
        }

        stateStack.pop_back();
    }

    void MsgPackSerializeContext::BeginObject(const std::string& key)
    {
    }

    void MsgPackSerializeContext::EndObject()
    {
    }

    // -------------------------------------------------------------------------
    // 基础写入实现 (Write)
    // -------------------------------------------------------------------------

    template<typename T>
    void WriteToMap(MsgPackSerializeContext::Node* node, const std::string& key, T&& val) {
        // 移除旧 Key
        auto& mapData = node->mapData;
        for (auto it = mapData.begin(); it != mapData.end(); ++it) {
            if (it->first == key) {
                mapData.erase(it);
                break;
            }
        }
        // 添加新 Key
        mapData.emplace_back(key, std::forward<T>(val));
    }

    void MsgPackSerializeContext::Write(const std::string& key, const std::string& value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }
    void MsgPackSerializeContext::Write(const std::string& key, const char* value)
    {
        Write(key, std::string(value ? value : ""));
    }
    void MsgPackSerializeContext::Write(const std::string& key, float value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }
    void MsgPackSerializeContext::Write(const std::string& key, int value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }
    void MsgPackSerializeContext::Write(const std::string& key, bool value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }
    void MsgPackSerializeContext::Write(const std::string& key, uint32_t value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }
    void MsgPackSerializeContext::Write(const std::string& key, glm::vec2 value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }
    void MsgPackSerializeContext::Write(const std::string& key, glm::vec3 value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }
    void MsgPackSerializeContext::Write(const std::string& key, glm::vec4 value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }
    void MsgPackSerializeContext::Write(const std::string& key, glm::ivec2 value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }
    void MsgPackSerializeContext::Write(const std::string& key, glm::ivec3 value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }
    void MsgPackSerializeContext::Write(const std::string& key, glm::ivec4 value)
    {
        WriteToMap(GetCurrentNode(), key, value);
    }

    // -------------------------------------------------------------------------
    // 数组元素写入 (WriteArrayItem)
    // -------------------------------------------------------------------------

    template<typename T>
    void WriteToArray(MsgPackSerializeContext::Node* node, T&& val) {
        node->arrayData.emplace_back(std::forward<T>(val));
    }

    void MsgPackSerializeContext::WriteArrayItem(const std::string& value)
    {
        WriteToArray(GetCurrentNode(), value);
    }
    void MsgPackSerializeContext::WriteArrayItem(const char* value)
    {
        WriteToArray(GetCurrentNode(), std::string(value ? value : ""));
    }
    void MsgPackSerializeContext::WriteArrayItem(float value)
    {
        WriteToArray(GetCurrentNode(), value);
    }
    void MsgPackSerializeContext::WriteArrayItem(int value)
    {
        WriteToArray(GetCurrentNode(), value);
    }
    void MsgPackSerializeContext::WriteArrayItem(bool value)
    {
        WriteToArray(GetCurrentNode(), value);
    }
    void MsgPackSerializeContext::WriteArrayItem(uint32_t value)
    {
        WriteToArray(GetCurrentNode(), value);
    }
    void MsgPackSerializeContext::WriteArrayItem(glm::vec2 value)
    {
        WriteToArray(GetCurrentNode(), value);
    }
    void MsgPackSerializeContext::WriteArrayItem(glm::vec3 value)
    {
        WriteToArray(GetCurrentNode(), value);
    }
    void MsgPackSerializeContext::WriteArrayItem(glm::vec4 value)
    {
        WriteToArray(GetCurrentNode(), value);
    }
    void MsgPackSerializeContext::WriteArrayItem(glm::ivec2 value)
    {
        WriteToArray(GetCurrentNode(), value);
    }
    void MsgPackSerializeContext::WriteArrayItem(glm::ivec3 value)
    {
        WriteToArray(GetCurrentNode(), value);
    }
    void MsgPackSerializeContext::WriteArrayItem(glm::ivec4 value)
    {
        WriteToArray(GetCurrentNode(), value);
    }

    // -------------------------------------------------------------------------
    // 核心递归打包逻辑
    // -------------------------------------------------------------------------

    void MsgPackSerializeContext::PackValue(msgpack::packer<msgpack::sbuffer>& pk, const Value& val) const
    {
        // 使用 std::visit 进行模式匹配
        std::visit([&pk, this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::monostate>) {
                pk.pack_nil();
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                pk.pack(arg);
            }
            else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int> || std::is_same_v<T, bool> || std::is_same_v<T, uint32_t>) {
                pk.pack(arg);
            }
            // GLM Types
            else if constexpr (std::is_same_v<T, glm::vec2>) {
                pk.pack_array(2); pk.pack(arg.x); pk.pack(arg.y);
            }
            else if constexpr (std::is_same_v<T, glm::vec3>) {
                pk.pack_array(3); pk.pack(arg.x); pk.pack(arg.y); pk.pack(arg.z);
            }
            else if constexpr (std::is_same_v<T, glm::vec4>) {
                pk.pack_array(4); pk.pack(arg.x); pk.pack(arg.y); pk.pack(arg.z); pk.pack(arg.w);
            }
            else if constexpr (std::is_same_v<T, glm::ivec2>) {
                pk.pack_array(2); pk.pack(arg.x); pk.pack(arg.y);
            }
            else if constexpr (std::is_same_v<T, glm::ivec3>) {
                pk.pack_array(3); pk.pack(arg.x); pk.pack(arg.y); pk.pack(arg.z);
            }
            else if constexpr (std::is_same_v<T, glm::ivec4>) {
                pk.pack_array(4); pk.pack(arg.x); pk.pack(arg.y); pk.pack(arg.z); pk.pack(arg.w);
            }
            // 递归节点
            else if constexpr (std::is_same_v<T, std::unique_ptr<Node>>) {
                if (arg) {
                    PackNode(pk, *arg);
                }
                else {
                    pk.pack_nil();
                }
            }
            }, val);
    }

    void MsgPackSerializeContext::PackNode(msgpack::packer<msgpack::sbuffer>& pk, const Node& node) const
    {
        if (node.type == Node::Map) {
            // 计算大小：普通 KV 数量 + SubContext 数量
            size_t mapSize = node.mapData.size() + node.subs.size();
            pk.pack_map(mapSize);

            // 1. 打包普通 KV
            for (const auto& kv : node.mapData) {
                pk.pack(kv.first);
                PackValue(pk, kv.second);
            }

            // 2. 【关键递归】打包 SubContext
            for (const auto& [key, subCtx] : node.subs) {
                pk.pack(key);
                // 子上下文一定有一个 Root 节点
                if (subCtx->root) {
                    PackNode(pk, *subCtx->root);
                }
                else {
                    pk.pack_nil();
                }
            }
        }
        else if (node.type == Node::Array) {
            pk.pack_array(node.arrayData.size());
            for (const auto& item : node.arrayData) {
                PackValue(pk, item);
            }
        }
    }

    // -------------------------------------------------------------------------
    // 输出与保存
    // -------------------------------------------------------------------------

    Blob MsgPackSerializeContext::ToBytes() const
    {
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> pk(sbuf);

        if (!root) {
            throw std::runtime_error("MsgPackSerializeContext: No root node");
        }

        // 开始递归打包
        PackNode(pk, *root);

        Blob out;
        out.resize(sbuf.size());
        std::memcpy(out.data(), sbuf.data(), sbuf.size());
        return out;
    }

    std::string MsgPackSerializeContext::ToString() const
    {
        // MsgPack 是二进制格式，ToString 通常用于调试
        // 这里我们可以返回一个 JSON 风格的预览，或者直接返回 Base64
        // 为了简单，这里提示使用 ToBytes
        return "<MsgPack Binary Data>";
    }

    void MsgPackSerializeContext::Save()
    {
        if (isSubContext)
            throw std::runtime_error("MsgPackSerializeContext: sub context cannot Save");
        if (file.empty())
            throw std::runtime_error("MsgPackSerializeContext: no file path set");

        if (!file.parent_path().empty() && !std::filesystem::exists(file.parent_path()))
            std::filesystem::create_directories(file.parent_path());

        std::ofstream ofs(file, std::ios::binary | std::ios::trunc);
        if (!ofs) throw std::runtime_error("MsgPackSerializeContext: failed to open file - " + file.string());

        Blob b = ToBytes();
        ofs.write(reinterpret_cast<const char*>(b.data()), static_cast<std::streamsize>(b.size()));
        ofs.flush();
    }

    // =========================================================================
    // MsgPackDeserializeContext 实现 (保持不变，已补全 glm)
    // =========================================================================

    MsgPackDeserializeContext::MsgPackDeserializeContext()
    {
        owner = std::make_shared<msgpack::object_handle>();
    }

    MsgPackDeserializeContext::MsgPackDeserializeContext(std::shared_ptr<msgpack::object_handle> owner, const msgpack::object* obj)
        : owner(std::move(owner)), obj(obj), isSubContext(true)
    {
    }

    void MsgPackDeserializeContext::LoadFile(const std::filesystem::path& file)
    {
        if (isSubContext)
            throw std::runtime_error("MsgPackDeserializeContext: sub context cannot LoadFile");

        std::ifstream ifs(file, std::ios::binary);
        if (!ifs) throw std::runtime_error("MsgPackDeserializeContext: failed to open file - " + file.string());

        std::stringstream ss;
        ss << ifs.rdbuf();
        ParseString(ss.str());
    }

    void MsgPackDeserializeContext::ParseBytes(std::span<const std::byte> data)
    {
        if (isSubContext)
            throw std::runtime_error("MsgPackDeserializeContext: sub context cannot ParseBytes");

        try {
            *owner = msgpack::unpack(reinterpret_cast<const char*>(data.data()), data.size());
            obj = &(owner->get());
            subs.clear();
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("MsgPackDeserializeContext: unpack failed - ") + e.what());
        }
    }

    void MsgPackDeserializeContext::ParseString(const std::string& data)
    {
        ParseBytes(std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(data.data()), data.size()
        ));
    }

    const msgpack::object* MsgPackDeserializeContext::Find(const std::string& key) const
    {
        if (!obj || obj->type != msgpack::type::MAP) return nullptr;

        auto map = obj->via.map;
        for (size_t i = 0; i < map.size; ++i) {
            if (map.ptr[i].key.type == msgpack::type::STR && map.ptr[i].key.as<std::string>() == key) {
                return &map.ptr[i].val;
            }
        }
        return nullptr;
    }

    const DeserializeContext& MsgPackDeserializeContext::GetSubContext(const std::string& key) const
    {
        if (subs.count(key) == 0)
        {
            const msgpack::object* v = Find(key);
            if (v && v->type == msgpack::type::MAP)
                subs[key] = std::make_unique<MsgPackDeserializeContext>(owner, v);
            else
                subs[key] = std::make_unique<MsgPackDeserializeContext>(owner, nullptr);
        }
        return *subs[key];
    }

    bool MsgPackDeserializeContext::HasSubContext(const std::string& key) const
    {
        const msgpack::object* v = Find(key);
        return v && v->type == msgpack::type::MAP;
    }

    std::vector<std::string> MsgPackDeserializeContext::GetAllSubKeys() const
    {
        std::vector<std::string> keys;
        if (!obj || obj->type != msgpack::type::MAP) return keys;

        auto map = obj->via.map;
        for (size_t i = 0; i < map.size; ++i) {
            if (map.ptr[i].key.type == msgpack::type::STR) {
                keys.push_back(map.ptr[i].key.as<std::string>());
            }
        }
        return keys;
    }

    // 数组反序列化
    bool MsgPackDeserializeContext::GetArraySize(const std::string& key, size_t& outSize) const
    {
        const msgpack::object* v = Find(key);
        if (!v || v->type != msgpack::type::ARRAY) return false;
        outSize = v->via.array.size;
        return true;
    }

    bool MsgPackDeserializeContext::HasArrayElement(size_t index) const
    {
        // 这里检查当前对象本身是否为数组
        if (!obj || obj->type != msgpack::type::ARRAY) return false;
        return index < obj->via.array.size;
    }

    const DeserializeContext& MsgPackDeserializeContext::GetArrayElement(size_t index) const
    {
        std::string key = std::to_string(index);
        if (subs.count(key) == 0)
        {
            // 这里有两种情况：
            // 1. 当前 m_obj 是一个 Map，我们通过 Find(key) 找数组
            // 2. 当前 m_obj 本身就是一个 Array (由上一层 GetArrayElement 返回)

            const msgpack::object* elem = nullptr;

            // 简单处理：如果当前是 Array，直接取 index
            if (obj && obj->type == msgpack::type::ARRAY && index < obj->via.array.size) {
                elem = &obj->via.array.ptr[index];
            }

            subs[key] = std::make_unique<MsgPackDeserializeContext>(owner, elem);
        }
        return *subs[key];
    }

    // 基础类型读取
    bool MsgPackDeserializeContext::Read(const std::string& key, std::string& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v || v->type != msgpack::type::STR) return false;
        out = v->as<std::string>();
        return true;
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, float& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v) return false;
        if (v->type != msgpack::type::FLOAT && v->type != msgpack::type::POSITIVE_INTEGER && v->type != msgpack::type::NEGATIVE_INTEGER) return false;
        out = v->as<float>();
        return true;
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, int& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v) return false;
        try { out = v->as<int>(); return true; }
        catch (...) { return false; }
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, bool& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v || v->type != msgpack::type::BOOLEAN) return false;
        out = v->as<bool>();
        return true;
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, uint32_t& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v || v->type != msgpack::type::POSITIVE_INTEGER) return false;
        try {
            uint64_t val = v->as<uint64_t>();
            if (val > std::numeric_limits<uint32_t>::max()) return false;
            out = static_cast<uint32_t>(val);
            return true;
        }
        catch (...) { return false; }
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, uint16_t& out) const
    {
        return false;
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, uint8_t& out) const
    {
        return false;
    }

    // GLM 反序列化
    bool MsgPackDeserializeContext::UnpackVec(const msgpack::object& o, glm::vec2& v)
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 2) return false;
        v.x = o.via.array.ptr[0].as<float>();
        v.y = o.via.array.ptr[1].as<float>();
        return true;
    }
    bool MsgPackDeserializeContext::UnpackVec(const msgpack::object& o, glm::vec3& v)
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 3) return false;
        v.x = o.via.array.ptr[0].as<float>();
        v.y = o.via.array.ptr[1].as<float>();
        v.z = o.via.array.ptr[2].as<float>();
        return true;
    }
    bool MsgPackDeserializeContext::UnpackVec(const msgpack::object& o, glm::vec4& v)
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 4) return false;
        v.x = o.via.array.ptr[0].as<float>();
        v.y = o.via.array.ptr[1].as<float>();
        v.z = o.via.array.ptr[2].as<float>();
        v.w = o.via.array.ptr[3].as<float>();
        return true;
    }
    bool MsgPackDeserializeContext::UnpackVec(const msgpack::object& o, glm::ivec2& v)
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 2) return false;
        v.x = o.via.array.ptr[0].as<int>();
        v.y = o.via.array.ptr[1].as<int>();
        return true;
    }
    bool MsgPackDeserializeContext::UnpackVec(const msgpack::object& o, glm::ivec3& v)
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 3) return false;
        v.x = o.via.array.ptr[0].as<int>();
        v.y = o.via.array.ptr[1].as<int>();
        v.z = o.via.array.ptr[2].as<int>();
        return true;
    }
    bool MsgPackDeserializeContext::UnpackVec(const msgpack::object& o, glm::ivec4& v)
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 4) return false;
        v.x = o.via.array.ptr[0].as<int>();
        v.y = o.via.array.ptr[1].as<int>();
        v.z = o.via.array.ptr[2].as<int>();
        v.w = o.via.array.ptr[3].as<int>();
        return true;
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, glm::vec2& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::vec3& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::vec4& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::ivec2& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::ivec3& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::ivec4& out) const
    {
        const msgpack::object* v = Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
}