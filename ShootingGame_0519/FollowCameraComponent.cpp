#define NOMINMAX
#include <Windows.h>   // もしくは <windows.h>
#include <d3d11.h>     // DirectX を使うなら
#include <algorithm>   // std::max/std::min
#include <DirectXMath.h>
#include <iostream>
#include "FollowCameraComponent.h"
#include "Renderer.h"
#include "Application.h"
#include "Input.h"
#include <memory>
#include <SimpleMath.h>
#include <algorithm>

using namespace DirectX;
using namespace DirectX::SimpleMath;

//constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = PI * 2.0f;

FollowCameraComponent::FollowCameraComponent()
{
    // スプリング設定
    m_Spring.SetStiffness(12.0f);
    m_Spring.SetDamping(6.0f);
    m_Spring.SetMass(1.0f);

    // プロジェクション行列
    float width = static_cast<float>(Application::GetWidth());
    float height = static_cast<float>(Application::GetHeight());
    m_ProjectionMatrix = Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(45.0f), width / height, 0.1f, 1000.0f);
}

//カメラが追う対象をセットする関数
void FollowCameraComponent::SetTarget(GameObject* target)
{
    m_Target = target;
    if (target)
    {
        //ターゲット位置 + (高さ, 後方) のオフセットで初期カメラ位置を決める
        Vector3 initial = target->GetPosition() + Vector3(0, m_DefaultHeight, -m_DefaultDistance);
        m_Spring.Reset(initial);

        // prev yaw を初期化（初回の大きなジャンプを防ぐ）
        m_PrevPlayerYaw = m_Target->GetRotation().y;
        m_CurrentTurnOffset = 0.0f;
    }
}

//
void FollowCameraComponent::Update(float dt)
{
    if (!m_Target) { return; }

    // エイムとマウスの処理（既存）
    m_IsAiming = Input::IsMouseRightDown();
    POINT delta = Input::GetMouseDelta();
    m_Yaw += delta.x * m_Sensitivity;
    m_Pitch += delta.y * m_Sensitivity;
    m_Pitch = std::clamp(m_Pitch, m_PitchLimitMin, m_PitchLimitMax);
    m_Yaw = std::clamp(m_Yaw, -m_YawLimit, m_YawLimit);

    // カメラ位置更新
    UpdateCameraPosition(dt);

    //カメラの現在位置とターゲット位置
    Vector3 cameraPos = m_Spring.GetPosition();
    Vector3 targetPos = m_Target->GetPosition();

    //レティクルのワールド位置
    float screenW = static_cast<float>(Application::GetWidth());
    float screenH = static_cast<float>(Application::GetHeight());
    float sx = m_ReticleScreen.x;
    float sy = m_ReticleScreen.y;

    //カメラの位置から注視点までのビュー変換行列を作成
    Matrix   provisionalView = Matrix::CreateLookAt(cameraPos, targetPos, Vector3::Up);
    XMVECTOR nearScreen = XMVectorSet(sx, sy, 0.0f, 1.0f);
    XMVECTOR farScreen  = XMVectorSet(sx, sy, 1.0f, 1.0f);

    XMMATRIX projXM  = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_ProjectionMatrix));
    XMMATRIX viewXM  = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&provisionalView));
    XMMATRIX worldXM = XMMatrixIdentity();

    //スクリーン座標からワールド空間にする
    XMVECTOR nearWorldV = XMVector3Unproject(
        nearScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM,
        viewXM,
        worldXM);

    XMVECTOR farWorldV = XMVector3Unproject(
        farScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM,
        viewXM,
        worldXM);


    Vector3 nearWorld(XMVectorGetX(nearWorldV), XMVectorGetY(nearWorldV), XMVectorGetZ(nearWorldV));
    Vector3 farWorld(XMVectorGetX(farWorldV), XMVectorGetY(farWorldV), XMVectorGetZ(farWorldV));

    //ニアからファーまでのベクトル
    Vector3 rayDir = farWorld - nearWorld;

    //よほど短くなければ正規化する
    if (rayDir.LengthSquared() > 1e-6f)
    {
        rayDir.Normalize();
    }

    //カメラの前方向ベクトル
    Vector3 camForward = GetForward();

    //カメラからAimPlaneDistance分だけ前方向に進んだ地点
    Vector3 planePoint = cameraPos + camForward * m_AimPlaneDistance;

    //ニアからファーまでのレイがカメラ前方向にどれだけ向かっているか
    float denom = rayDir.Dot(camForward);

    Vector3 worldTarget;

    //絶対値が極端に小さければ
    if (fabs(denom) < 1e-5f)
    {
        //ニア(ワールド)からAimPlaneDistance分だけニアからファー方向に進んだ点
        worldTarget = nearWorld + rayDir * m_AimPlaneDistance;
    }
    else
    {
        //
        float t = (planePoint - nearWorld).Dot(camForward) / denom;
        worldTarget = nearWorld + rayDir * t;
    }

    //どんなもんの速さでカメラ追従させるか
    const float aimLerp = 12.0f;

    //レティクルが示すワールド座標
    m_AimPoint = m_AimPoint + (worldTarget - m_AimPoint) * std::min(1.0f, aimLerp * dt);

    //計算
    Vector3 aimDir = m_AimPoint - targetPos;
    if (aimDir.LengthSquared() > 1e-6f)
    {
        aimDir.Normalize();
    }
    else
    {
        aimDir = GetForward();
    }

    Vector3 rawLookTarget = targetPos + aimDir * m_LookAheadDistance + Vector3(0.0f, (m_DefaultHeight + m_AimHeight) * 0.5f, 0.0f);

    float lookBoost = 1.5f;

    Vector3 targetRot = m_Target->GetRotation();
    Matrix playerRot = Matrix::CreateRotationY(targetRot.y);
    Vector3 localRight = Vector3::Transform(Vector3::Right, playerRot);
    rawLookTarget += localRight * (m_CurrentTurnOffset * lookBoost);

    //平滑化
    m_LookTarget = m_LookTarget + (rawLookTarget - m_LookTarget) * std::min(1.0f, m_LookAheadLerp * dt);

    //ビュー行列を作る
    m_ViewMatrix = Matrix::CreateLookAt(cameraPos, m_LookTarget, Vector3::Up);

    //レンダラへ
    Renderer::SetViewMatrix(m_ViewMatrix);
    Renderer::SetProjectionMatrix(m_ProjectionMatrix);
}


void FollowCameraComponent::UpdateCameraPosition(float dt)
{
    if (!m_Target) { return; }

    // 距離/高さ（エイム時に変える既存処理）
    float dist = m_IsAiming ? m_AimDistance : m_DefaultDistance;
    float height = m_IsAiming ? m_AimHeight : m_DefaultHeight;

    Vector3 targetPos = m_Target->GetPosition();
    Vector3 targetRot = m_Target->GetRotation();
    float playerYaw = targetRot.y;

    Matrix playerRot = Matrix::CreateRotationY(playerYaw);
    Vector3 baseOffset = Vector3(0.0f, 0.0f, -dist);
    Vector3 rotatedOffset = Vector3::Transform(baseOffset, playerRot);

    Vector3 desiredPos = targetPos + rotatedOffset + Vector3(0.0f, height, 0.0f);

    // レティクルによる横シフト（既存）
    float screenW = static_cast<float>(Application::GetWidth());
    float normX = 0.0f;
    if (screenW > 1.0f)
    {
        normX = (m_ReticleScreen.x - (screenW * 0.5f)) / (screenW * 0.5f);
    }
    Vector3 localRight = Vector3::Transform(Vector3::Right, playerRot);
    float lateral = normX * m_ScreenOffsetScale;
    lateral = std::clamp(lateral, -m_MaxScreenOffset, m_MaxScreenOffset);
    desiredPos += localRight * lateral;

    float yawDelta = playerYaw - m_PrevPlayerYaw;
    while (yawDelta > PI)
    {
        yawDelta -= TWO_PI;
    }
    while (yawDelta < -PI)
    {
        yawDelta += TWO_PI;
    }

    float safeDt = std::max(1e-6f, dt);
    float yawSpeed = yawDelta / safeDt;

    //右に回るなら offset > 0 -> カメラをプレイヤーの右へ移動 
    //見た目上プレイヤーは左へ寄る
    float turnLateral = yawSpeed * m_TurnOffsetScale;
    turnLateral = std::clamp(turnLateral, -m_TurnOffsetMax, m_TurnOffsetMax);

    // 滑らかにする（lerp）
    m_CurrentTurnOffset = m_CurrentTurnOffset + (turnLateral - m_CurrentTurnOffset) * std::min(1.0f, m_TurnOffsetLerp * dt);

    //カメラ位置に横オフセットを追加
    desiredPos += localRight * m_CurrentTurnOffset;

    // 更新
    m_PrevPlayerYaw = playerYaw;

    // スプリングで滑らかに追従
    m_Spring.Update(desiredPos, dt);
}

Vector3 FollowCameraComponent::GetForward() const
{
    return m_ViewMatrix.Invert().Forward();
}

Vector3 FollowCameraComponent::GetRight() const
{
    return m_ViewMatrix.Invert().Right();
}

Vector3 FollowCameraComponent::GetAimDirectionFromReticle() const
{
    return { 0,0,0 };
}