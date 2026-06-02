#include <Serialize/Backend/Json/CJsonContext.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <limits>
#include <cmath>

namespace Online::Serialize
{
    static cJSON* GetCurrentObject(CJsonSerializeContext::WriteState& state)
    {
        if (state.type != CJsonSerializeContext::WriteState::Object)
            throw std::runtime_error("CJsonSerializeContext: Not currently writing to an object. Did you forget EndArray/EndObject?");
        return state.node;
    }

    static void AddItemToArrayInternal(CJsonSerializeContext::WriteState& state, cJSON* item)
    {
        if (state.type != CJsonSerializeContext::WriteState::Array)
            throw std::runtime_error("CJsonSerializeContext: WriteArrayItem called without being in an array");

        if (!cJSON_AddItemToArray(state.node, item))
        {
            cJSON_Delete(item);
            throw std::runtime_error("CJsonSerializeContext: Failed to add item to array");
        }
    }

    static const cJSON* ArrayAt(const cJSON* arr, int idx)
    {
        return cJSON_GetArrayItem(const_cast<cJSON*>(arr), idx);
    }

    CJsonSerializeContext::CJsonSerializeContext()
    {
        root.reset(cJSON_CreateObject());
        if (!root)
            throw std::runtime_error("CJsonSerializeContext: Failed to create root object");

        writeStack.push_back({ WriteState::Object, root.get() });
    }

    CJsonSerializeContext::CJsonSerializeContext(cJSON* targetNode, CJsonSerializeContext* parent)
        : parent(parent), isSubContext(true)
    {
        if (!targetNode)
            throw std::runtime_error("CJsonSerializeContext: Cannot create subcontext for null node");

        writeStack.push_back({ WriteState::Object, targetNode });
    }

    void CJsonSerializeContext::EnsureRootObject()
    {
        if (isSubContext ? !parent->root : !root)
            throw std::runtime_error("CJsonSerializeContext: root is null");
    }

    void CJsonSerializeContext::OpenFile(const std::filesystem::path& file)
    {
        if (isSubContext)
            throw std::runtime_error("CJsonSerializeContext: sub context cannot OpenFile");

        // 异常安全：先创建所有临时对象
        CJsonPtr newRoot(cJSON_CreateObject());
        if (!newRoot)
            throw std::runtime_error("CJsonSerializeContext: Failed to create root object");

        std::vector<WriteState> newStack;
        newStack.push_back({ WriteState::Object, newRoot.get() });

        // 全部成功后再替换原有状态
        this->file = file;
        root = std::move(newRoot);
        subs.clear();
        writeStack = std::move(newStack);
    }

    void CJsonSerializeContext::WriteItem(const std::string& key, cJSON* item)
    {
        if (writeStack.empty()) throw std::runtime_error("CJsonSerializeContext: Write stack is empty");

        cJSON* currentObj = GetCurrentObject(writeStack.back());

        if (!item)
            throw std::runtime_error("CJsonSerializeContext: null json item");

        // 覆盖已有同名键
        cJSON* old = cJSON_DetachItemFromObject(currentObj, key.c_str());
        if (old) cJSON_Delete(old);

        // 添加失败检查并释放节点
        if (!cJSON_AddItemToObject(currentObj, key.c_str(), item))
        {
            cJSON_Delete(item);
            throw std::runtime_error("CJsonSerializeContext: Failed to add item to object: " + key);
        }
    }

    void CJsonSerializeContext::BeginObject(const std::string& key)
    {
        if (writeStack.empty()) throw std::runtime_error("CJsonSerializeContext: Stack empty");

        cJSON* currentObj = GetCurrentObject(writeStack.back());

        cJSON* old = cJSON_DetachItemFromObject(currentObj, key.c_str());
        if (old) cJSON_Delete(old);

        cJSON* obj = cJSON_CreateObject();
        if (!obj)
            throw std::runtime_error("CJsonSerializeContext: Failed to create object: " + key);

        if (!cJSON_AddItemToObject(currentObj, key.c_str(), obj))
        {
            cJSON_Delete(obj);
            throw std::runtime_error("CJsonSerializeContext: Failed to add object: " + key);
        }

        writeStack.push_back({ WriteState::Object, obj });
    }

    void CJsonSerializeContext::EndObject()
    {
        if (writeStack.empty() || writeStack.back().type != WriteState::Object)
            throw std::runtime_error("CJsonSerializeContext: EndObject called without matching BeginObject");

        // 禁止弹出根对象
        if (writeStack.size() == 1)
            throw std::runtime_error("CJsonSerializeContext: Cannot end root object");

        writeStack.pop_back();
    }

    void CJsonSerializeContext::BeginArray(const std::string& key)
    {
        if (writeStack.empty()) throw std::runtime_error("CJsonSerializeContext: Stack empty");

        cJSON* currentObj = GetCurrentObject(writeStack.back());

        cJSON* old = cJSON_DetachItemFromObject(currentObj, key.c_str());
        if (old) cJSON_Delete(old);

        cJSON* arr = cJSON_CreateArray();
        if (!arr)
            throw std::runtime_error("CJsonSerializeContext: Failed to create array: " + key);

        if (!cJSON_AddItemToObject(currentObj, key.c_str(), arr))
        {
            cJSON_Delete(arr);
            throw std::runtime_error("CJsonSerializeContext: Failed to add array: " + key);
        }

        writeStack.push_back({ WriteState::Array, arr });
    }

    void CJsonSerializeContext::EndArray()
    {
        if (writeStack.empty() || writeStack.back().type != WriteState::Array)
            throw std::runtime_error("CJsonSerializeContext: EndArray called without matching BeginArray");
        writeStack.pop_back();
    }

    SerializeContext& CJsonSerializeContext::GetSubContext(const std::string& key)
    {
        if (subs.count(key) == 0)
        {
            if (writeStack.empty())
                throw std::runtime_error("CJsonSerializeContext: Write stack is empty");

            cJSON* currentObj = GetCurrentObject(writeStack.back());

            cJSON* subObj = cJSON_GetObjectItemCaseSensitive(currentObj, key.c_str());
            if (!subObj || !cJSON_IsObject(subObj))
            {
                cJSON* old = cJSON_DetachItemFromObject(currentObj, key.c_str());
                if (old) cJSON_Delete(old);

                subObj = cJSON_CreateObject();
                if (!subObj)
                    throw std::runtime_error("CJsonSerializeContext: Failed to create subcontext object");

                cJSON_AddItemToObject(currentObj, key.c_str(), subObj);
            }

            subs[key] = (ONLINE_NEW(CJsonSerializeContext, subObj, this));
        }

        return *subs[key];
    }

    SerializeContext& CJsonSerializeContext::WriteArrayObjectBegin()
    {
        if (writeStack.empty() || writeStack.back().type != WriteState::Array)
            throw std::runtime_error("CJsonSerializeContext: WriteArrayObjectBegin must be called inside an array");

        cJSON* obj = cJSON_CreateObject();
        if (!obj)
            throw std::runtime_error("CJsonSerializeContext: Failed to create array object");

        if (!cJSON_AddItemToArray(writeStack.back().node, obj))
        {
            cJSON_Delete(obj);
            throw std::runtime_error("CJsonSerializeContext: Failed to add array object");
        }

        std::string tempKey = "__array_obj_" + std::to_string(reinterpret_cast<uintptr_t>(obj));

        subs[tempKey] = (ONLINE_NEW(CJsonSerializeContext, obj, this));
        return *subs[tempKey];
    }

    void CJsonSerializeContext::WriteArrayObjectEnd()
    {
        // 已废弃：不再需要任何操作，保留仅为向后兼容
    }

    cJSON* CJsonSerializeContext::PackVec(glm::vec2 v)
    {
        cJSON* arr = cJSON_CreateArray();
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.x));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.y));
        return arr;
    }
    cJSON* CJsonSerializeContext::PackVec(glm::vec3 v)
    {
        cJSON* arr = cJSON_CreateArray();
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.x));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.y));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.z));
        return arr;
    }
    cJSON* CJsonSerializeContext::PackVec(glm::vec4 v)
    {
        cJSON* arr = cJSON_CreateArray();
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.x));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.y));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.z));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.w));
        return arr;
    }
    cJSON* CJsonSerializeContext::PackVec(glm::ivec2 v)
    {
        cJSON* arr = cJSON_CreateArray();
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.x));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.y));
        return arr;
    }
    cJSON* CJsonSerializeContext::PackVec(glm::ivec3 v)
    {
        cJSON* arr = cJSON_CreateArray();
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.x));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.y));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.z));
        return arr;
    }
    cJSON* CJsonSerializeContext::PackVec(glm::ivec4 v)
    {
        cJSON* arr = cJSON_CreateArray();
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.x));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.y));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.z));
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.w));
        return arr;
    }

    void CJsonSerializeContext::WriteArrayItem(const std::string& value)
    {
        AddItemToArrayInternal(writeStack.back(), cJSON_CreateString(value.c_str()));
    }
    void CJsonSerializeContext::WriteArrayItem(const char* value)
    {
        AddItemToArrayInternal(writeStack.back(), cJSON_CreateString(value ? value : ""));
    }
    void CJsonSerializeContext::WriteArrayItem(float value)
    {
        AddItemToArrayInternal(writeStack.back(), cJSON_CreateNumber(value));
    }
    void CJsonSerializeContext::WriteArrayItem(int value)
    {
        AddItemToArrayInternal(writeStack.back(), cJSON_CreateNumber(value));
    }
    void CJsonSerializeContext::WriteArrayItem(bool value)
    {
        AddItemToArrayInternal(writeStack.back(), cJSON_CreateBool(value ? 1 : 0));
    }
    void CJsonSerializeContext::WriteArrayItem(uint32_t value)
    {
        AddItemToArrayInternal(writeStack.back(), cJSON_CreateNumber(static_cast<double>(value)));
    }
    void CJsonSerializeContext::WriteArrayItem(glm::vec2 value)
    {
        AddItemToArrayInternal(writeStack.back(), PackVec(value));
    }
    void CJsonSerializeContext::WriteArrayItem(glm::vec3 value)
    {
        AddItemToArrayInternal(writeStack.back(), PackVec(value));
    }
    void CJsonSerializeContext::WriteArrayItem(glm::vec4 value)
    {
        AddItemToArrayInternal(writeStack.back(), PackVec(value));
    }
    void CJsonSerializeContext::WriteArrayItem(glm::ivec2 value)
    {
        AddItemToArrayInternal(writeStack.back(), PackVec(value));
    }
    void CJsonSerializeContext::WriteArrayItem(glm::ivec3 value)
    {
        AddItemToArrayInternal(writeStack.back(), PackVec(value));
    }
    void CJsonSerializeContext::WriteArrayItem(glm::ivec4 value)
    {
        AddItemToArrayInternal(writeStack.back(), PackVec(value));
    }

    void CJsonSerializeContext::Write(const std::string& key, const std::string& value)
    {
        WriteItem(key, cJSON_CreateString(value.c_str()));
    }
    void CJsonSerializeContext::Write(const std::string& key, const char* value)
    {
        Write(key, std::string(value ? value : ""));
    }
    void CJsonSerializeContext::Write(const std::string& key, float value)
    {
        WriteItem(key, cJSON_CreateNumber(value));
    }
    void CJsonSerializeContext::Write(const std::string& key, glm::vec2 value)
    {
        WriteItem(key, PackVec(value));
    }
    void CJsonSerializeContext::Write(const std::string& key, glm::vec3 value)
    {
        WriteItem(key, PackVec(value));
    }
    void CJsonSerializeContext::Write(const std::string& key, glm::vec4 value)
    {
        WriteItem(key, PackVec(value));
    }
    void CJsonSerializeContext::Write(const std::string& key, int value)
    {
        WriteItem(key, cJSON_CreateNumber(value));
    }
    void CJsonSerializeContext::Write(const std::string& key, glm::ivec2 value)
    {
        WriteItem(key, PackVec(value));
    }
    void CJsonSerializeContext::Write(const std::string& key, glm::ivec3 value)
    {
        WriteItem(key, PackVec(value));
    }
    void CJsonSerializeContext::Write(const std::string& key, glm::ivec4 value)
    {
        WriteItem(key, PackVec(value));
    }
    void CJsonSerializeContext::Write(const std::string& key, bool value)
    {
        WriteItem(key, cJSON_CreateBool(value ? 1 : 0));
    }
    void CJsonSerializeContext::Write(const std::string& key, uint32_t value)
    {
        WriteItem(key, cJSON_CreateNumber(static_cast<double>(value)));
    }

    std::string CJsonSerializeContext::ToString() const
    {
        // 主上下文使用自己的root，子上下文使用父上下文的root
        cJSON* finalRoot = isSubContext ? parent->root.get() : root.get();

        if (!finalRoot)
            throw std::runtime_error("CJsonSerializeContext: Root is null");

        char* finalPrinted = formatted
            ? cJSON_Print(finalRoot)
            : cJSON_PrintUnformatted(finalRoot);

        if (!finalPrinted)
            throw std::runtime_error("CJsonSerializeContext: final print failed");

        std::string out(finalPrinted);
        cJSON_free(finalPrinted);
        return out;
    }

    Blob CJsonSerializeContext::ToBytes() const
    {
        const std::string s = ToString();
        Blob out;
        out.resize(s.size());
        std::memcpy(out.data(), s.data(), s.size());
        return out;
    }

    void CJsonSerializeContext::Save()
    {
        if (isSubContext)
            throw std::runtime_error("CJsonSerializeContext: sub context cannot Save");
        if (file.empty())
            throw std::runtime_error("CJsonSerializeContext: no file path set");

        if (!file.parent_path().empty() && !std::filesystem::exists(file.parent_path()))
            std::filesystem::create_directories(file.parent_path());

        std::ofstream ofs(file, std::ios::binary | std::ios::trunc);
        if (!ofs) throw std::runtime_error("CJsonSerializeContext: failed to open file - " + file.string());

        const std::string s = ToString();
        ofs.write(s.data(), static_cast<std::streamsize>(s.size()));
        ofs.flush();
    }

    CJsonDeserializeContext::CJsonDeserializeContext()
    {
        owner = std::make_shared<CJsonPtr>(CJsonPtr{});
    }

    CJsonDeserializeContext::CJsonDeserializeContext(std::shared_ptr<CJsonPtr> owner, const cJSON* obj)
        : owner(std::move(owner)), obj(obj), isSubContext(true)
    {
    }

    void CJsonDeserializeContext::LoadFile(const std::filesystem::path& file)
    {
        if (isSubContext)
            throw std::runtime_error("CJsonDeserializeContext: sub context cannot LoadFile");

        std::ifstream ifs(file, std::ios::binary);
        if (!ifs) throw std::runtime_error("CJsonDeserializeContext: failed to open file - " + file.string());

        std::ostringstream oss;
        oss << ifs.rdbuf();
        ParseString(oss.str());
    }

    void CJsonDeserializeContext::ParseBytes(std::span<const std::byte> data)
    {
        if (isSubContext)
            throw std::runtime_error("CJsonDeserializeContext: sub context cannot ParseBytes");

        const char* ptr = reinterpret_cast<const char*>(data.data());
        const size_t sz = data.size();

        cJSON* root = cJSON_ParseWithLength(ptr, sz);
        if (!root)
            throw std::runtime_error("CJsonDeserializeContext: cJSON_ParseWithLength failed");

        // 关键修复：创建新的shared_ptr，而不是修改原有内容
        owner = std::make_shared<CJsonPtr>(CJsonPtr(root));
        obj = owner->get();
        subs.clear();
    }

    void CJsonDeserializeContext::ParseString(const std::string& data)
    {
        ParseBytes(std::span<const std::byte>(reinterpret_cast<const std::byte*>(data.data()), data.size()));
    }

    const cJSON* CJsonDeserializeContext::Find(const std::string& key) const
    {
        if (!obj || !cJSON_IsObject(obj)) return nullptr;
        return cJSON_GetObjectItemCaseSensitive(obj, key.c_str());
    }

    const DeserializeContext& CJsonDeserializeContext::GetSubContext(const std::string& key) const
    {
        if (subs.count(key) == 0)
        {
            const cJSON* v = Find(key);
            if (v && (cJSON_IsObject(v) || cJSON_IsArray(v)))
                subs[key] = std::make_unique<CJsonDeserializeContext>(owner, v);
            else
                subs[key] = std::make_unique<CJsonDeserializeContext>(owner, nullptr);
        }
        return *subs[key];
    }

    bool CJsonDeserializeContext::HasSubContext(const std::string& key) const
    {
        const cJSON* v = Find(key);
        return v && (cJSON_IsObject(v) || cJSON_IsArray(v));
    }

    std::vector<std::string> CJsonDeserializeContext::GetAllSubKeys() const
    {
        std::vector<std::string> keys;
        if (!obj || !cJSON_IsObject(obj)) return keys;

        for (const cJSON* child = obj->child; child; child = child->next)
        {
            if (child->string)
                keys.emplace_back(child->string);
        }
        return keys;
    }

    bool CJsonDeserializeContext::GetArraySize(const std::string& key, size_t& outSize) const
    {
        const cJSON* target = nullptr;

        if (key.empty())
            target = obj;
        else
            target = Find(key);

        if (!target || !cJSON_IsArray(target))
            return false;

        outSize = cJSON_GetArraySize(target);
        return true;
    }

    bool CJsonDeserializeContext::HasArrayElement(size_t index) const
    {
        if (!obj || !cJSON_IsArray(obj)) return false;
        return index < static_cast<size_t>(cJSON_GetArraySize(const_cast<cJSON*>(obj)));
    }

    const DeserializeContext& CJsonDeserializeContext::GetArrayElement(size_t index) const
    {
        std::string key = std::to_string(index);

        if (subs.count(key) == 0)
        {
            if (!obj || !cJSON_IsArray(obj))
            {
                subs[key] = std::make_unique<CJsonDeserializeContext>(owner, nullptr);
            }
            else
            {
                size_t arraySize = cJSON_GetArraySize(const_cast<cJSON*>(obj));
                if (index >= arraySize)
                {
                    subs[key] = std::make_unique<CJsonDeserializeContext>(owner, nullptr);
                }
                else
                {
                    const cJSON* elem = ArrayAt(obj, static_cast<int>(index));
                    subs[key] = std::make_unique<CJsonDeserializeContext>(owner, elem);
                }
            }
        }
        return *subs[key];
    }

    bool CJsonDeserializeContext::UnpackVec(const cJSON* arr, glm::vec2& v)
    {
        if (!arr || !cJSON_IsArray(arr) || cJSON_GetArraySize(arr) != 2) return false;
        const cJSON* x = ArrayAt(arr, 0);
        const cJSON* y = ArrayAt(arr, 1);
        if (!cJSON_IsNumber(x) || !cJSON_IsNumber(y)) return false;
        v.x = static_cast<float>(x->valuedouble);
        v.y = static_cast<float>(y->valuedouble);
        return true;
    }
    bool CJsonDeserializeContext::UnpackVec(const cJSON* arr, glm::vec3& v)
    {
        if (!arr || !cJSON_IsArray(arr) || cJSON_GetArraySize(arr) != 3) return false;
        const cJSON* x = ArrayAt(arr, 0);
        const cJSON* y = ArrayAt(arr, 1);
        const cJSON* z = ArrayAt(arr, 2);
        if (!cJSON_IsNumber(x) || !cJSON_IsNumber(y) || !cJSON_IsNumber(z)) return false;
        v.x = static_cast<float>(x->valuedouble);
        v.y = static_cast<float>(y->valuedouble);
        v.z = static_cast<float>(z->valuedouble);
        return true;
    }
    bool CJsonDeserializeContext::UnpackVec(const cJSON* arr, glm::vec4& v)
    {
        if (!arr || !cJSON_IsArray(arr) || cJSON_GetArraySize(arr) != 4) return false;
        const cJSON* x = ArrayAt(arr, 0);
        const cJSON* y = ArrayAt(arr, 1);
        const cJSON* z = ArrayAt(arr, 2);
        const cJSON* w = ArrayAt(arr, 3);
        if (!cJSON_IsNumber(x) || !cJSON_IsNumber(y) || !cJSON_IsNumber(z) || !cJSON_IsNumber(w)) return false;
        v.x = static_cast<float>(x->valuedouble);
        v.y = static_cast<float>(y->valuedouble);
        v.z = static_cast<float>(z->valuedouble);
        v.w = static_cast<float>(w->valuedouble);
        return true;
    }
    bool CJsonDeserializeContext::UnpackVec(const cJSON* arr, glm::ivec2& v)
    {
        if (!arr || !cJSON_IsArray(arr) || cJSON_GetArraySize(arr) != 2) return false;
        const cJSON* x = ArrayAt(arr, 0);
        const cJSON* y = ArrayAt(arr, 1);
        if (!cJSON_IsNumber(x) || !cJSON_IsNumber(y)) return false;
        v.x = static_cast<int>(x->valuedouble);
        v.y = static_cast<int>(y->valuedouble);
        return true;
    }
    bool CJsonDeserializeContext::UnpackVec(const cJSON* arr, glm::ivec3& v)
    {
        if (!arr || !cJSON_IsArray(arr) || cJSON_GetArraySize(arr) != 3) return false;
        const cJSON* x = ArrayAt(arr, 0);
        const cJSON* y = ArrayAt(arr, 1);
        const cJSON* z = ArrayAt(arr, 2);
        if (!cJSON_IsNumber(x) || !cJSON_IsNumber(y) || !cJSON_IsNumber(z)) return false;
        v.x = static_cast<int>(x->valuedouble);
        v.y = static_cast<int>(y->valuedouble);
        v.z = static_cast<int>(z->valuedouble);
        return true;
    }
    bool CJsonDeserializeContext::UnpackVec(const cJSON* arr, glm::ivec4& v)
    {
        if (!arr || !cJSON_IsArray(arr) || cJSON_GetArraySize(arr) != 4) return false;
        const cJSON* x = ArrayAt(arr, 0);
        const cJSON* y = ArrayAt(arr, 1);
        const cJSON* z = ArrayAt(arr, 2);
        const cJSON* w = ArrayAt(arr, 3);
        if (!cJSON_IsNumber(x) || !cJSON_IsNumber(y) || !cJSON_IsNumber(z) || !cJSON_IsNumber(w)) return false;
        v.x = static_cast<int>(x->valuedouble);
        v.y = static_cast<int>(y->valuedouble);
        v.z = static_cast<int>(z->valuedouble);
        v.w = static_cast<int>(w->valuedouble);
        return true;
    }
    bool CJsonDeserializeContext::Read(const std::string& key, std::string& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v || !cJSON_IsString(v) || !v->valuestring) return false;
        out = v->valuestring;
        return true;
    }
    bool CJsonDeserializeContext::Read(const std::string& key, float& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v || !cJSON_IsNumber(v)) return false;
        out = static_cast<float>(v->valuedouble);
        return true;
    }
    bool CJsonDeserializeContext::Read(const std::string& key, glm::vec2& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(v, out);
    }
    bool CJsonDeserializeContext::Read(const std::string& key, glm::vec3& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(v, out);
    }
    bool CJsonDeserializeContext::Read(const std::string& key, glm::vec4& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(v, out);
    }
    bool CJsonDeserializeContext::Read(const std::string& key, int& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v || !cJSON_IsNumber(v)) return false;

        double d = v->valuedouble;
        if (d != std::floor(d)) return false; // 必须是整数
        if (d < static_cast<double>(std::numeric_limits<int>::min()) ||
            d > static_cast<double>(std::numeric_limits<int>::max()))
            return false;

        out = static_cast<int>(d);
        return true;
    }
    bool CJsonDeserializeContext::Read(const std::string& key, glm::ivec2& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(v, out);
    }
    bool CJsonDeserializeContext::Read(const std::string& key, glm::ivec3& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(v, out);
    }
    bool CJsonDeserializeContext::Read(const std::string& key, glm::ivec4& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v) return false;
        return UnpackVec(v, out);
    }
    bool CJsonDeserializeContext::Read(const std::string& key, bool& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v || !cJSON_IsBool(v)) return false;
        out = cJSON_IsTrue(v);
        return true;
    }
    bool CJsonDeserializeContext::Read(const std::string& key, uint32_t& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v || !cJSON_IsNumber(v)) return false;

        double d = v->valuedouble;
        if (d != std::floor(d)) return false; // 必须是整数
        if (d < 0.0) return false;
        if (d > static_cast<double>(std::numeric_limits<uint32_t>::max())) return false;

        out = static_cast<uint32_t>(d);
        return true;
    }
    bool CJsonDeserializeContext::Read(const std::string& key, uint8_t& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v || !cJSON_IsNumber(v)) return false;

        double d = v->valuedouble;
        if (d != std::floor(d)) return false; // 必须是整数
        if (d < 0.0) return false;
        if (d > static_cast<double>(std::numeric_limits<uint8_t>::max())) return false;

        out = static_cast<uint8_t>(d);
        return true;
    }
    bool CJsonDeserializeContext::Read(const std::string& key, uint16_t& out) const
    {
        const cJSON* v = key.empty() ? obj : Find(key);
        if (!v || !cJSON_IsNumber(v)) return false;

        double d = v->valuedouble;
        if (d != std::floor(d)) return false; // 必须是整数
        if (d < 0.0) return false;
        if (d > static_cast<double>(std::numeric_limits<uint16_t>::max())) return false;

        out = static_cast<uint16_t>(d);
        return true;
    }
}