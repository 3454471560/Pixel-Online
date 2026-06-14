#include <vector>
#include <Serialize/Serializable.h>


namespace Online::Net
{
    struct WorldSnapshot
    {
        std::vector<std::byte> sceneData;
        uint32_t localPlayerNetId = 0;

        std::vector<std::byte> SerializePayload() const
        {
            std::vector<std::byte> out;
            out.reserve(sizeof(uint32_t) + sceneData.size());

            const std::byte* idBytes = reinterpret_cast<const std::byte*>(&localPlayerNetId);
            out.insert(out.end(), idBytes, idBytes + sizeof(uint32_t));

            out.insert(out.end(), sceneData.begin(), sceneData.end());
            return out;
        }

        bool DeserializeFromPayload(const std::vector<std::byte>& body)
        {
            const size_t idLen = sizeof(uint32_t);
            if (body.size() < idLen)
            {
                return false;
            }

            std::memcpy(&localPlayerNetId, body.data(), idLen);

            sceneData.assign(body.begin() + idLen, body.end());
            return true;
        }
    };
}