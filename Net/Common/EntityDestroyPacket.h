#pragma once
#include <Net/Common/NetCommon.h>
#include <vector>
#include <cstdint>

namespace Online::Net
{
    struct EntityDestroyPacket
    {
        std::vector<uint32_t> netIds;

        // 序列化：写入数量 + 每个 netId
        std::vector<std::byte> SerializePayload() const
        {
            std::vector<std::byte> buffer;
            uint32_t count = static_cast<uint32_t>(netIds.size());
            auto countBytes = reinterpret_cast<const std::byte*>(&count);
            buffer.insert(buffer.end(), countBytes, countBytes + sizeof(count));
            for (uint32_t id : netIds)
            {
                auto idBytes = reinterpret_cast<const std::byte*>(&id);
                buffer.insert(buffer.end(), idBytes, idBytes + sizeof(id));
            }
            return buffer;
        }

        // 反序列化
        bool DeserializeFromPayload(const std::vector<std::byte>& data)
        {
            if (data.size() < sizeof(uint32_t)) return false;
            uint32_t count = *reinterpret_cast<const uint32_t*>(data.data());
            size_t offset = sizeof(uint32_t);
            if (data.size() < offset + count * sizeof(uint32_t)) return false;
            netIds.resize(count);
            for (uint32_t i = 0; i < count; ++i)
            {
                std::memcpy(&netIds[i], data.data() + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);
            }
            return true;
        }
    };
}