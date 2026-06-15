#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <span>
#include <glm.hpp>

#include <Game/Character/RoleID.h>
#include <Phys/Common/BodyType.h>
#include <Phys/Common/ColliderShape.h>
#include <Serialize/Serializable.h>
#include <Net/Common/NetCommon.h>

namespace Online::Net
{
    struct EntityFullData : public Online::Serialize::Serializable
    {
        uint32_t netId = 0;
        int ownerConnId = -1;
        std::string entityName;
        std::string entityTag;

        glm::vec2 position{};
        glm::vec2 scale{};
        float rotation = 0.0f;

        bool hasRigidbody = false;
        float gravityScale = 1.0f;
        bool fixedRotation = false;
        Physics::BodyType bodyType{};

        bool hasCollider = false;
        Physics::ColliderShape shape{};
        glm::vec2 halfSize{};
        float radius = 0.0f;
        uint32_t layerBits = 0;
        float density = 1.0f;
        float friction = 0.0f;
        float restitution = 0.0f;

        std::vector<uint32_t> scriptIds;

        bool hasCharacter = false;
        Game::RoleID roleId;


        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("netId", netId);
            ctx.Write("ownerConnId", ownerConnId);
            ctx.Write("entityName", entityName);
            ctx.Write("entityTag", entityTag);

            ctx.Write("position", position);
            ctx.Write("scale", scale);
            ctx.Write("rotation", rotation);

            ctx.Write("hasRigidbody", hasRigidbody);
            ctx.Write("gravityScale", gravityScale);
            ctx.Write("fixedRotation", fixedRotation);
            ctx.Write("bodyType", static_cast<int>(bodyType));

            ctx.Write("hasCollider", hasCollider);
            ctx.Write("colliderShape", static_cast<int>(shape));
            ctx.Write("halfSize", halfSize);
            ctx.Write("radius", radius);
            ctx.Write("layerBits", layerBits);
            ctx.Write("density", density);
            ctx.Write("friction", friction);
            ctx.Write("restitution", restitution);

            ctx.Write("hasCharacter", hasCharacter);
            ctx.Write("RoleID", static_cast<uint8_t>(roleId));

            ctx.BeginArray("scriptIds");
            for (uint32_t sid : scriptIds)
            {
                ctx.WriteArrayItem(sid);
            }
            ctx.EndArray();
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            ctx.Read("netId", netId);
            ctx.Read("ownerConnId", ownerConnId);
            ctx.Read("entityName", entityName);
            ctx.Read("entityTag", entityTag);

            ctx.Read("position", position);
            ctx.Read("scale", scale);
            ctx.Read("rotation", rotation);

            ctx.Read("hasRigidbody", hasRigidbody);
            ctx.Read("gravityScale", gravityScale);
            ctx.Read("fixedRotation", fixedRotation);

            int bodyTypeVal = 0;
            if (ctx.Read("bodyType", bodyTypeVal))
            {
                bodyType = static_cast<Physics::BodyType>(bodyTypeVal);
            }

            ctx.Read("hasCollider", hasCollider);

            int shapeVal = 0;
            if (ctx.Read("colliderShape", shapeVal))
            {
                shape = static_cast<Physics::ColliderShape>(shapeVal);
            }

            ctx.Read("halfSize", halfSize);
            ctx.Read("radius", radius);
            ctx.Read("layerBits", layerBits);
            ctx.Read("density", density);
            ctx.Read("friction", friction);
            ctx.Read("restitution", restitution);

            scriptIds.clear();
            const auto& scriptCtx = ctx.GetSubContext("scriptIds");
            size_t arrSize = 0;
            if (scriptCtx.GetArraySize("", arrSize))
            {
                scriptIds.reserve(arrSize);
                for (size_t i = 0; i < arrSize; ++i)
                {
                    uint32_t sid = 0;
                    if (scriptCtx.GetArrayElement(i).Read("", sid))
                    {
                        scriptIds.push_back(sid);
                    }
                }
            }

            ctx.Read("hasCharacter", hasCharacter);
            uint8_t roleId;
            ctx.Read("RoleID", roleId);
            this->roleId = static_cast<Game::RoleID>(roleId);
        }

        std::vector<std::byte> SerializePayload() const
        {
            std::vector<std::byte> out;
            SerializeToBytes(out);
            return out;
        }

        bool DeserializeFromPayload(std::span<const std::byte> payload)
        {
            return DeserializeFromBytes(payload);
        }
    };
}