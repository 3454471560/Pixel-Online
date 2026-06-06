#pragma once
#include <Game/Component/Component.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/epsilon.hpp>

#include <vector>
#include <algorithm>

namespace Online::Game
{
    struct Transform : public Component
    {
    public:
        inline void Serialize(Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("localPos", localPosition);
            ctx.Write("localRot", localRotation);
            ctx.Write("localScale", localScale);
        }
        inline void Deserialize(const Serialize::DeserializeContext& ctx) override
        {
            ctx.Read("localPos", localPosition);
            ctx.Read("localRot", localRotation);
            ctx.Read("localScale", localScale);
            localDirty = true;
            MarkDirty();
        }

        glm::vec2 WorldToLocalPosition(const glm::vec2& worldPos) const
        {
            const glm::vec2& refPos = GetWorldPosition();
            const float refAngle = GetWorldRotation();
            glm::vec2 delta = worldPos - refPos;
            float cosA = cosf(refAngle);
            float sinA = sinf(refAngle);
            return { delta.x * cosA - delta.y * sinA,
                     delta.x * sinA + delta.y * cosA };
        }

        float WorldToLocalRotation(float worldAngle) const
        {
            return worldAngle - GetWorldRotation();
        }

        inline glm::vec2 GetLocalPosition() const noexcept { return localPosition; }

        inline float GetLocalRotation() const noexcept { return -localRotation; }

        inline glm::vec2 GetLocalScale() const noexcept { return localScale; }

        inline const glm::mat4& GetLocalMatrix() const noexcept
        {
            if (localDirty)
            {
                glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(localPosition, 0.0f));
                glm::mat4 r = glm::rotate(glm::mat4(1.0f), localRotation, glm::vec3(0.0f, 0.0f, 1.0f));
                glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(localScale, 1.0f));
                localMatrix = t * r * s;
                localDirty = false;
            }
            return localMatrix;
        }

        inline glm::vec2 GetWorldPosition() const noexcept
        {
            return glm::vec2(GetWorldMatrix()[3]);
        }

        inline float GetWorldRotation() const noexcept
        {
            const glm::mat4& wm = GetWorldMatrix();
            float ccw = std::atan2f(wm[0][1], wm[0][0]);
            return -ccw;
        }

        inline glm::vec2 GetWorldScale() const noexcept
        {
            const glm::mat4& worldMat = GetWorldMatrix();
            glm::vec2 s = glm::vec2(glm::length(worldMat[0]), glm::length(worldMat[1]));

            const float multiplier = 10000.0f;
            s.x = std::round(s.x * multiplier) / multiplier;
            s.y = std::round(s.y * multiplier) / multiplier;

            return s;
        }

        inline float GetWorldScaleAverage() const
        {
            const glm::mat4& worldMat = GetWorldMatrix();
            glm::vec2 s = glm::vec2(glm::length(worldMat[0]), glm::length(worldMat[1]));

            const float multiplier = 10000.0f;
            s.x = std::round(s.x * multiplier) / multiplier;
            s.y = std::round(s.y * multiplier) / multiplier;

            return (s.x + s.y) / 2;
        }

        inline const glm::mat4& GetWorldMatrix() const noexcept
        {
            if (worldDirty)
            {
                glm::mat4 parentWorldMatrix = glm::mat4(1.0f);
                if (parentTransform)
                    parentWorldMatrix = parentTransform->GetWorldMatrix();

                worldMatrix = parentWorldMatrix * GetLocalMatrix();
                worldDirty = false;
            }
            return worldMatrix;
        }

        inline void SetLocalPosition(glm::vec2 pos) noexcept
        {
            if (glm::all(glm::epsilonEqual(localPosition, pos, 0.0001f)))
                return;
            localPosition = pos;
            localDirty = true;
            MarkDirty();
            MarkLocalDirty();
        }

        inline void SetLocalRotation(float cwRad) noexcept
        {
            float internalRad = -cwRad;
            if (glm::epsilonEqual(localRotation, internalRad, 0.0001f))
                return;
            localRotation = internalRad;
            localDirty = true;
            MarkDirty();
            MarkLocalDirty();
        }

        inline void SetLocalScale(glm::vec2 scale) noexcept
        {
            if (glm::all(glm::epsilonEqual(localScale, scale, 0.0001f)))
                return;
            localScale = scale;
            localDirty = true;
            MarkDirty();
            MarkLocalDirty();
        }

        inline void SetLocalFromMatrix(const glm::mat4& matrix) noexcept
        {
            localPosition = glm::vec2(matrix[3]);
            localRotation = std::atan2f(matrix[0][1], matrix[0][0]);
            localScale = glm::vec2(glm::length(matrix[0]), glm::length(matrix[1]));
            localDirty = true;
            MarkDirty();
        }

        inline void SetWorldMatrix(const glm::mat4& matrix) noexcept
        {
            glm::mat4 parentWorldMatrix = glm::mat4(1.0f);
            if (parentTransform)
                parentWorldMatrix = parentTransform->GetWorldMatrix();

            glm::mat4 localMatrix = glm::inverse(parentWorldMatrix) * matrix;
            SetLocalFromMatrix(localMatrix);
        }

        inline void SetWorldPosition(glm::vec2 worldPos) noexcept
        {
            glm::mat4 parentWorldMat = glm::mat4(1.0f);
            if (parentTransform)
                parentWorldMat = parentTransform->GetWorldMatrix();

            glm::mat4 invParent = glm::inverse(parentWorldMat);
            glm::vec4 localPos4 = invParent * glm::vec4(worldPos, 0.0f, 1.0f);
            glm::vec2 localPos = glm::vec2(localPos4);

            SetLocalPosition(localPos);
        }

        inline void SetWorldRotation(float targetCW) noexcept
        {
            const glm::mat4& currentWorld = GetWorldMatrix();
            glm::vec2 pos(currentWorld[3]);
            glm::vec2 scale(glm::length(currentWorld[0]), glm::length(currentWorld[1]));

            float ccw = -targetCW;
            glm::mat4 newWorld = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f))
                * glm::rotate(glm::mat4(1.0f), ccw, glm::vec3(0.0f, 0.0f, 1.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(scale, 1.0f));
            SetWorldMatrix(newWorld);
        }

        inline Transform* GetParent() const noexcept { return parentTransform; }
        inline const std::vector<Transform*>& GetChildren() const noexcept { return children; }
        inline size_t GetChildCount() const noexcept { return children.size(); }
        inline Transform* GetChildAt(size_t index) const noexcept { return index < children.size() ? children[index] : nullptr; }
        inline bool IsRoot() const noexcept { return parentTransform == nullptr; }

        inline void SetParent(Transform* newParent) noexcept
        {
            if (parentTransform == newParent) return;

            if (parentTransform)
            {
                auto& oldParentChildren = parentTransform->children;
                auto it = std::find(oldParentChildren.begin(), oldParentChildren.end(), this);
                if (it != oldParentChildren.end())
                    oldParentChildren.erase(it);
            }

            parentTransform = newParent;

            if (newParent)
                newParent->children.push_back(this);

            MarkDirty();
        }

        inline void RemoveFromParent() noexcept { SetParent(nullptr); }

        inline void MarkDirty() noexcept
        {
            if (worldDirty) return;
            worldDirty = true;
            for (Transform* child : children)
                child->MarkDirty();
        }

        inline bool IsDirty() const noexcept { return worldDirty; }

        inline void Reset() noexcept
        {
            localPosition = glm::vec2(0.0f);
            localRotation = 0.0f;
            localScale = glm::vec2(1.0f);
            localMatrix = glm::mat4(1.0f);
            worldMatrix = glm::mat4(1.0f);
            localDirty = true;
            worldDirty = true;

            RemoveFromParent();
            children.clear();
        }

        inline void MarkLocalDirty() const noexcept 
        { 
            ColliderDirty = true; 
        }

        inline bool IsLocalDirty() const noexcept 
        { 
            return ColliderDirty;
        }

        inline void ClearLocalDirty() const noexcept 
        { 
            ColliderDirty = false;
        }
    private:
        glm::vec2 localPosition = glm::vec2(0.0f);
        float     localRotation = 0.0f; 
        glm::vec2 localScale = glm::vec2(1.0f);
        mutable glm::mat4 localMatrix = glm::mat4(1.0f);
        mutable bool localDirty = true;

        mutable glm::mat4 worldMatrix = glm::mat4(1.0f);
        mutable bool worldDirty = true;

        Transform* parentTransform = nullptr;
        std::vector<Transform*> children;

        mutable bool ColliderDirty = false;
    };
}