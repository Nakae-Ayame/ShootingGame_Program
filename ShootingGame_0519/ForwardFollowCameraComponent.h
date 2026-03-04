#pragma once
#include "CameraComponentBase.h"
#include "ForwardMoveComponent.h"

class GameObject;

/// <summary>
/// 前に進み続けるようなシューティングゲームの
/// カメラの動き用のコンポーネント
/// イメージはスターフォックス
/// </summary>
class ForwardFollowCameraComponent : public CameraComponentBase
{
public:
    void Initialize() override;
    void Update(float dt) override;
    void Draw(float dt) override;

    //-------------Set関数--------------
    void SetTarget(GameObject* target) { m_target = target; }
    void SetBackDistance(float dist) { m_backDistance = dist; }
    void SetHeight(float height) { m_height = height; }
    void SetSideOffset(float offset) { m_sideOffset = offset; }
    void SetRailProvider(ForwardMoveComponent* forwardComp) { m_forwardComp = forwardComp; }

    //-------------ICameraViewProvider実装--------------
    DirectX::SimpleMath::Vector3 GetForward() const override { return m_forward; }
    DirectX::SimpleMath::Vector3 GetRight() const override { return m_right; }
    DirectX::SimpleMath::Vector3 GetAimPoint() const override { return m_aimPoint; }
    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; }

    DirectX::SimpleMath::Vector3 GetAimDirectionFromReticle() const override;
    DirectX::SimpleMath::Vector2 GetReticleScreen() const override { return m_reticleScreen; }

    // ※GetView/GetProjはBaseにあるけど、念のためoverrideで明示しておく（抽象回避が確実）
    DirectX::SimpleMath::Matrix GetView() const override { return m_ViewMatrix; }
    DirectX::SimpleMath::Matrix GetProj() const override { return m_ProjectionMatrix; }

private:
    //--------------Target関連------------------
    GameObject* m_target = nullptr;
    ForwardMoveComponent* m_forwardComp = nullptr;

    //--------------Cameraパラメータ------------------
    float m_backDistance = 20.0f;
    float m_height = 1.5f;
    float m_sideOffset = 1.0f;
    float m_lookAhead = 30.0f;

    //--------------Camera状態（Baseを触れないのでここで持つ）------------------
    DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 m_forward = DirectX::SimpleMath::Vector3::Forward;
    DirectX::SimpleMath::Vector3 m_right = DirectX::SimpleMath::Vector3::Right;
    DirectX::SimpleMath::Vector3 m_aimPoint = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector2 m_reticleScreen = DirectX::SimpleMath::Vector2::Zero;
};
