#include <Serialize/Backend/MsgPack/MsgPackContext.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <limits>

namespace Online::Serialize
{
    MsgPackSerializeContext::MsgPackSerializeContext()
    {
        root = std::make_unique<Node>(Node::Map);
        stateStack.push_back({ root.get(), nullptr });
    }

    void MsgPackSerializeContext::OpenFile(const std::filesystem::path& file)
    {
        if (isSubContext)
            throw std::runtime_error("MsgPackSerializeContext: sub context cannot OpenFile");

        this->file = file;

        root = std::make_unique<Node>(Node::Map);
        stateStack.clear();
        stateStack.push_back({ root.get(), nullptr });
    }

    SerializeContext& MsgPackSerializeContext::GetSubContext(const std::string& key)
    {
        Node* current = GetCurrentNode();
        if (current->type != Node::Map) {
            throw std::runtime_error("MsgPackSerializeContext: GetSubContext must be called on a Map/Object");
        }

        auto& subs = current->subs;
        if (subs.count(key) == 0)
        {
            auto sub = std::make_unique<MsgPackSerializeContext>();
            sub->isSubContext = true;
            subs[key] = std::move(sub);
        }
        return *subs[key];
    }

    void MsgPackSerializeContext::BeginObject(const std::string& key)
    {
        Node* current = GetCurrentNode();
        if (current->type != Node::Map) {
            throw std::runtime_error("MsgPackSerializeContext: BeginObject must be called on a Map");
        }

        // 创建一个新的 Map 节点
        auto objNode = std::make_unique<Node>(Node::Map);
        Node* objPtr = objNode.get();

        // 放入当前 Map 的数据中
        current->mapData.emplace_back(key, std::move(objNode));

        // 压栈，使后续的写入进入这个新 Map
        stateStack.push_back({ objPtr, nullptr });
    }

    void MsgPackSerializeContext::EndObject()
    {
        if (stateStack.size() <= 1) {
            throw std::runtime_error("MsgPackSerializeContext: EndObject called without matching BeginObject");
        }

        Node* current = GetCurrentNode();
        if (current->type != Node::Map) {
            throw std::runtime_error("MsgPackSerializeContext: EndObject called but not in object state");
        }

        stateStack.pop_back();
    }

    void MsgPackSerializeContext::BeginArray(const std::string& key)
    {
        Node* current = GetCurrentNode();
        if (current->type != Node::Map) {
            throw std::runtime_error("MsgPackSerializeContext: BeginArray must be called on a Map");
        }

        // 创建一个 Array 类型的 Node
        auto arrayNode = std::make_unique<Node>(Node::Array);
        Node* arrayPtr = arrayNode.get();

        // 将这个 Node 存入当前 Map
        current->mapData.emplace_back(key, std::move(arrayNode));

        // 压入栈
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

        // 创建一个 Map 类型的 Node
        auto objNode = std::make_unique<Node>(Node::Map);
        Node* objPtr = objNode.get();

        // 将这个 Node 存入 Array
        current->arrayData.emplace_back(std::move(objNode));

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
            size_t mapSize = node.mapData.size() + node.subs.size();
            pk.pack_map(mapSize);

            for (const auto& kv : node.mapData) {
                pk.pack(kv.first);
                PackValue(pk, kv.second);
            }

            for (const auto& [key, subCtx] : node.subs) {
                pk.pack(key);
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

    std::vector<std::byte> MsgPackSerializeContext::ToBytes() const
    {
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> pk(sbuf);

        if (!root) {
            throw std::runtime_error("MsgPackSerializeContext: No root node");
        }

        PackNode(pk, *root);

        std::vector<std::byte> out;
        out.resize(sbuf.size());
        std::memcpy(out.data(), sbuf.data(), sbuf.size());
        return out;
    }

    std::string MsgPackSerializeContext::ToString() const
    {
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

        std::vector<std::byte> b = ToBytes();
        ofs.write(reinterpret_cast<const char*>(b.data()), static_cast<std::streamsize>(b.size()));
        ofs.flush();
    }

    // =========================================================================
    // MsgPackDeserializeContext 实现
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

    bool MsgPackDeserializeContext::GetArraySize(const std::string& key, size_t& outSize) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v || v->type != msgpack::type::ARRAY) return false;
        outSize = v->via.array.size;
        return true;
    }

    bool MsgPackDeserializeContext::HasArrayElement(size_t index) const
    {
        if (!obj || obj->type != msgpack::type::ARRAY) return false;
        return index < obj->via.array.size;
    }

    const DeserializeContext& MsgPackDeserializeContext::GetArrayElement(size_t index) const
    {
        std::string key = std::to_string(index);
        if (subs.count(key) == 0)
        {
            const msgpack::object* elem = nullptr;
            if (obj && obj->type == msgpack::type::ARRAY && index < obj->via.array.size) {
                elem = &obj->via.array.ptr[index];
            }
            subs[key] = std::make_unique<MsgPackDeserializeContext>(owner, elem);
        }
        return *subs[key];
    }

    // ---------- Read 方法修复：支持空 key ----------

    bool MsgPackDeserializeContext::Read(const std::string& key, std::string& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v || v->type != msgpack::type::STR) return false;
        out = v->as<std::string>();
        return true;
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, float& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        if (v->type != msgpack::type::FLOAT && v->type != msgpack::type::POSITIVE_INTEGER && v->type != msgpack::type::NEGATIVE_INTEGER) return false;
        out = v->as<float>();
        return true;
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, int& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        try { out = v->as<int>(); return true; }
        catch (...) { return false; }
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, bool& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v || v->type != msgpack::type::BOOLEAN) return false;
        out = v->as<bool>();
        return true;
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, uint32_t& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
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
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v || v->type != msgpack::type::POSITIVE_INTEGER) return false;
        try {
            uint64_t val = v->as<uint64_t>();
            if (val > std::numeric_limits<uint16_t>::max()) return false;
            out = static_cast<uint16_t>(val);
            return true;
        }
        catch (...) { return false; }
    }

    bool MsgPackDeserializeContext::Read(const std::string& key, uint8_t& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v || v->type != msgpack::type::POSITIVE_INTEGER) return false;
        try {
            uint64_t val = v->as<uint64_t>();
            if (val > std::numeric_limits<uint8_t>::max()) return false;
            out = static_cast<uint8_t>(val);
            return true;
        }
        catch (...) { return false; }
    }

    // GLM 读取也加上空 key 支持
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::vec2& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::vec3& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::vec4& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::ivec2& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::ivec3& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }
    bool MsgPackDeserializeContext::Read(const std::string& key, glm::ivec4& out) const
    {
        const msgpack::object* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(*v, out);
    }

    // 保留的 UnpackVec 实现（无改动）
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
}