#pragma once

#include<Core/Camera/CameraSnapshot.h>

namespace Online::Core
{
    struct CameraState
    {
        float Zoom = 1.0f;
        float Rotation = 0.0f;
        bool  IsWorld = true;

        inline CameraSnapshot BuildSnapshot(glm::vec2 position) const
        {
            CameraSnapshot snapshot;
            snapshot.Position = position;
            snapshot.Zoom = Zoom;
            snapshot.Rotation = Rotation;
            snapshot.IsWorld = IsWorld;
            return snapshot;
        }
    };
}