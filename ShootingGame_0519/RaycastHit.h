#pragma once
#include <SimpleMath.h>

class GameObject;
class ColliderComponent;

struct RaycastHit
{
    //--------------ヒット結果関連------------------
    DirectX::SimpleMath::Vector3 point = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 normal = DirectX::SimpleMath::Vector3::Up;
    float distance = 0.0f;

    //--------------参照先関連------------------
    GameObject* hitObject = nullptr;
    ColliderComponent* hitCollider = nullptr;
};