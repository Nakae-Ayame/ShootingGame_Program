#pragma once
#include <SimpleMath.h>

class GameObject;
class ColliderComponent;

struct RaycastHit
{
    DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 normal   = DirectX::SimpleMath::Vector3::Up;
    float distance = 0.0f;

    GameObject* hitObject = nullptr;
    ColliderComponent* hitCollider = nullptr;
};