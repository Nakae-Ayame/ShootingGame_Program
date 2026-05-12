#pragma once

#include "commontypes.h"
#include <SimpleMath.h>

namespace SMS = DirectX::SimpleMath;

class ICameraViewProvider
{
public:
    virtual ~ICameraViewProvider() = default;

    //--------Getä÷êî-------
    virtual SMS::Vector3 GetForward() const = 0;
    virtual SMS::Vector3 GetRight() const = 0;
    virtual SMS::Vector3 GetUp() const = 0;

    virtual SMS::Vector3 GetPosition() const = 0;

    virtual SMS::Vector3 GetAimPoint() const = 0;
    virtual SMS::Vector3 GetAimDirectionFromReticle() const = 0;

    virtual SMS::Vector3 GetShootRayOrigin() const = 0;
    virtual SMS::Vector3 GetShootRayDir() const = 0;

    virtual SMS::Vector2 GetReticleScreen() const = 0;

    virtual SMS::Matrix GetView() const = 0;
    virtual SMS::Matrix GetProj() const = 0;

    //--------Setä÷êî-------
    virtual void SetBoostState(bool isBoosting) {}
};