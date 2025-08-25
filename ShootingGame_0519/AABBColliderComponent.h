#pragma once
#include "ColliderComponent.h"
#include <DirectXMath.h>
#include <SimpleMath.h>
//-------------------------------------------------------
//AABBによる当たり判定(軸に平行な当たり判定)のクラス
//回転の事は考えず、X,Y,Z軸に対して並行のままで考える
//-------------------------------------------------------
class AABBColliderComponent : public ColliderComponent
{
public:
    AABBColliderComponent() : ColliderComponent(ColliderType::AABB) {}

    //当たり判定の大きさを指定する関数
    //(全体の幅・高さ・奥行き)
    void SetSize(const Vector3& size) { m_Size = size; }

    //GameObjectの位置のゲット関数
    Vector3 GetCenter() const override
    {
        return GetOwner()->GetPosition();
    }

    //当たり判定用のサイズのゲット関数(m_Size)
    Vector3 GetSize() const override
    {
        return m_Size;
    }

    //AABBは回転を考慮しないのでIdentity(回転なし)にしておく
    DirectX::SimpleMath::Matrix GetRotationMatrix() const override
    {
        return DirectX::SimpleMath::Matrix::Identity;
    }

    //中央から半分のサイズを引いて最小座標を出す
    Vector3 GetMin() const
    {
        return GetCenter() - m_Size * 0.5f;
    }

    //中央から半分のサイズを引いて最大座標を出す
    Vector3 GetMax() const
    {
        return GetCenter() + m_Size * 0.5f;
    }

private:

    //幅、高さ、奥行の大きさをそれぞれ設定できる変数
    Vector3 m_Size = Vector3(1, 1, 1);
};


//例：中心が(10, 3, 5)、サイズが(4, 2, 6) なら、
//半サイズ = (2, 1, 3)
//Min = (8, 2, 2)、Max = (12, 4, 8)
