// FollowCameraComponent.cpp
#define NOMINMAX
#include <cmath> 
#include "FollowCameraComponent.h"
#include "Renderer.h"
#include "Application.h"
#include "Input.h"
#include "PlayAreaComponent.h"
#include <SimpleMath.h>
#include <algorithm>
#include <iostream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

Vector3 FollowCameraComponent::GetAimPoint() const
{
    float screenW = static_cast<float>(Application::GetWidth());
    float screenH = static_cast<float>(Application::GetHeight());
    if (screenW <= 1.0f || screenH <= 1.0f)
    {
        // 画面サイズがおかしいときはカメラ正面
        return GetPosition() + GetForward() * m_aimPlaneDistance;
    }

    float sx = m_reticleScreen.x;
    float sy = m_reticleScreen.y;

    // 実際に描画に使っている View / Proj をそのまま使う
    XMMATRIX projXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_ProjectionMatrix));
    XMMATRIX viewXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_ViewMatrix));
    XMMATRIX worldXM = XMMatrixIdentity();

    // 画面上の (sx,sy) から、near/far の２点をワールドに逆変換
    XMVECTOR nearScreen = XMVectorSet(sx, sy, 0.0f, 1.0f);
    XMVECTOR farScreen = XMVectorSet(sx, sy, 1.0f, 1.0f);

    XMVECTOR nearWorld = XMVector3Unproject(
        nearScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM, viewXM, worldXM
    );

    XMVECTOR farWorld = XMVector3Unproject(
        farScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM, viewXM, worldXM
    );

    // near→far 方向ベクトル（＝レティクルを通るカメラレイの方向）
    XMVECTOR dirW = XMVector3Normalize(farWorld - nearWorld);

    Vector3 dir;
    XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&dir), dirW);

    // カメラ位置を取得
    Vector3 camPos = m_spring.GetPosition();

    // カメラから一定距離だけ先の点を AimPoint とする（距離は適当に調整可）
    float dist = m_aimPlaneDistance; // 300.0f など
    return camPos + dir * dist;
}

FollowCameraComponent::FollowCameraComponent()
{
    m_normalStiffness = 12.0f;
    m_normalDamping   = 10.0f;
    m_spring.SetStiffness(m_normalStiffness);
    m_spring.SetDamping(m_normalDamping);
    m_spring.SetMass(1.0f);

    m_Fov = m_normalFov;
    UpdateProjectionMatrix();
}

void FollowCameraComponent::SetTarget(GameObject* target)
{
    m_target = target;
    if (target)
    {
        Vector3 initial = target->GetPosition() + Vector3(0, m_defaultHeight, -m_defaultDistance);
        m_spring.Reset(initial);
        m_prevPlayerYaw = m_target->GetRotation().y;
        m_currentTurnOffset = 0.0f;
    }
}

Vector3 FollowCameraComponent::GetAimDirectionFromReticle() const
{
    float screenW = static_cast<float>(Application::GetWidth());
    float screenH = static_cast<float>(Application::GetHeight());
    if (screenW <= 1.0f || screenH <= 1.0f)
    {
        return GetForward(); // フォールバック
    }

    float sx = m_reticleScreen.x;
    float sy = m_reticleScreen.y;

    // 描画に実際に使っている View / Proj 行列
    XMMATRIX projXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_ProjectionMatrix));
    XMMATRIX viewXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_ViewMatrix));
    XMMATRIX worldXM = XMMatrixIdentity();

    XMVECTOR nearScreen = XMVectorSet(sx, sy, 0.0f, 1.0f);
    XMVECTOR farScreen = XMVectorSet(sx, sy, 1.0f, 1.0f);

    XMVECTOR nearWorld = XMVector3Unproject(
        nearScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM, viewXM, worldXM
    );

    XMVECTOR farWorld = XMVector3Unproject(
        farScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM, viewXM, worldXM
    );

    XMVECTOR dirW = XMVector3Normalize(farWorld - nearWorld);

    Vector3 dir;
    XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&dir), dirW);
    return dir;
}

void FollowCameraComponent::ComputeReticleRay(const Matrix& view, Vector3* outOrigin, Vector3* outDir) const
{
    if (!outOrigin || !outDir)
    {
        return;
    }

    float screenW = static_cast<float>(Application::GetWidth());
    float screenH = static_cast<float>(Application::GetHeight());
    if (screenW <= 1.0f || screenH <= 1.0f)
    {
        *outOrigin = Vector3::Zero;
        *outDir = Vector3::Forward;
        return;
    }

    float sx = m_reticleScreen.x;
    float sy = m_reticleScreen.y;

    XMMATRIX projXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_ProjectionMatrix));
    XMMATRIX viewXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&view));
    XMMATRIX worldXM = XMMatrixIdentity();

    XMVECTOR nearScreen = XMVectorSet(sx, sy, 0.0f, 1.0f);
    XMVECTOR farScreen = XMVectorSet(sx, sy, 1.0f, 1.0f);

    XMVECTOR nearWorldV = XMVector3Unproject(
        nearScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM, viewXM, worldXM);

    XMVECTOR farWorldV = XMVector3Unproject(
        farScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM, viewXM, worldXM);

    Vector3 nearWorld(XMVectorGetX(nearWorldV), XMVectorGetY(nearWorldV), XMVectorGetZ(nearWorldV));
    Vector3 farWorld(XMVectorGetX(farWorldV), XMVectorGetY(farWorldV), XMVectorGetZ(farWorldV));

    Vector3 dir = farWorld - nearWorld;
    if (dir.LengthSquared() > 1e-6f)
    {
        dir.Normalize();
    }
    else
    {
        dir = Vector3::Forward;
    }

    *outOrigin = nearWorld;
    *outDir = dir;
}

Vector3 FollowCameraComponent::ComputeRayPlaneIntersection(const Vector3& rayOrigin,
                                                           const Vector3& rayDir,
                                                           const Vector3& planePoint,
                                                           const Vector3& planeNormal) const
{
    const float EPS = 1e-5f;

    float denom = rayDir.Dot(planeNormal);
    if (fabs(denom) < EPS)
    {
        return rayOrigin + rayDir * m_aimPlaneDistance;
    }

    float t = (planePoint - rayOrigin).Dot(planeNormal) / denom;
    return rayOrigin + rayDir * t;
}

/// <summary>
/// カメラのFOVをブースト状態によって更新するUpdate関数
/// </summary>
/// <param name="dt">デルタタイム</param>
void FollowCameraComponent::UpdateFov(float dt)
{
    float targetFov = m_normalFov;
    if (m_boostRequested)
    {
        targetFov = m_boostFov;
    }

    float t = std::min(1.0f, m_fovLerpSpeed * dt);
    m_Fov = m_Fov + (targetFov - m_Fov) * t;

    UpdateProjectionMatrix();
}

/// <summary>
/// 
/// </summary>
void FollowCameraComponent::UpdateProjectionMatrix()
{
    float width = static_cast<float>(Application::GetWidth());
    float height = static_cast<float>(Application::GetHeight());
    if (width <= 1.0f || height <= 1.0f){ return; }

    m_ProjectionMatrix = Matrix::CreatePerspectiveFieldOfView(
        m_Fov, width / height, 0.1f, 1000.0f);
}

/// <summary>
/// 
/// </summary>
/// <param name="dt"></param>
/// <param name="cameraPos"></param>
void FollowCameraComponent::UpdateLookTarget(float dt, const Vector3& cameraPos)
{
    if (!m_target){ return; }

    Vector3 targetPos = m_target->GetPosition();
    Vector3 aimDir = m_aimPoint - targetPos;

    if (aimDir.LengthSquared() > 1e-6f)
    {
        aimDir.Normalize();
    }
    else
    {
        Vector3 fallback = targetPos - cameraPos;
        if (fallback.LengthSquared() > 1e-6f)
        {
            fallback.Normalize();
        }
        else
        {
            fallback = Vector3::Forward;
        }
        aimDir = fallback;
    }

    aimDir.y *= m_verticalAimScale;
    if (aimDir.LengthSquared() > 1e-6f)
    {
        aimDir.Normalize();
    }

    Vector3 rawLookTarget = targetPos
        + aimDir * m_lookAheadDistance
        + Vector3(0.0f, (m_defaultHeight + m_aimHeight) * 0.5f, 0.0f);

    float screenH = static_cast<float>(Application::GetHeight());
    float normY = 0.0f;
    if (screenH > 1.0f)
    {
        normY = (m_reticleScreen.y - (screenH * 0.5f)) / (screenH * 0.5f);
    }

    float verticalOffset = -normY * m_LookVerticalScale;
    rawLookTarget += Vector3(0.0f, verticalOffset, 0.0f);

    Vector3 targetRot = m_target->GetRotation();
    Matrix playerRot = Matrix::CreateRotationY(targetRot.y);
    Vector3 localRight = Vector3::Transform(Vector3::Right, playerRot);

    float lookOffsetMul = 0.6f;
    rawLookTarget += localRight * (m_currentTurnOffset * lookOffsetMul);

    float t = std::min(1.0f, m_lookAheadLerp * dt);
    m_LookTarget = m_LookTarget + (rawLookTarget - m_LookTarget) * t;
}

void FollowCameraComponent::UpdateAimPoint(float dt, const Vector3& cameraPos)
{
    if (!m_target){ return; }

    float screenW = static_cast<float>(Application::GetWidth());
    float screenH = static_cast<float>(Application::GetHeight());
    if (screenW <= 1.0f || screenH <= 1.0f){ return; }

    Vector3 targetPos = m_target->GetPosition();
    
    Matrix provisionalView = Matrix::CreateLookAt(cameraPos, targetPos, Vector3::Up);

    Vector3 rayOrigin = Vector3::Zero;
    Vector3 rayDir = Vector3::Forward;
    ComputeReticleRay(provisionalView, &rayOrigin, &rayDir);

    Matrix invView = provisionalView.Invert();
    Vector3 camForward = invView.Forward();
    if (camForward.LengthSquared() > 1e-6f)
    {
        camForward.Normalize();
    }
    else
    {
        camForward = Vector3::Forward;
    }

    Vector3 planePoint = cameraPos + camForward * m_aimPlaneDistance;
    Vector3 planeNormal = camForward;

    Vector3 worldTarget = ComputeRayPlaneIntersection(rayOrigin, rayDir, planePoint, planeNormal);

    // AimPointをスムーズに追従
    const float aimLerp = 12.0f;
    float t = std::min(1.0f, aimLerp * dt);
    m_aimPoint = m_aimPoint + (worldTarget - m_aimPoint) * t;
}


void FollowCameraComponent::UpdateShake(float dt, const Vector3& cameraPos)
{
    m_shakeOffset = Vector3::Zero;

    if (m_shakeTimeRemaining <= 0.0f){ return; }

    if (!m_target){ return; }

    float safeTotal = std::max(1e-6f, m_shakeTotalDuration);
    float t = m_shakeTimeRemaining / safeTotal;
    float falloff = t;

    m_shakePhase += dt * m_shakeFrequency * 2.0f * XM_PI;

    float sampleX = std::sin(m_shakePhase);
    float sampleY = std::sin(m_shakePhase * 1.5f + 3.1f);

    float baseAmp = m_shakeMagnitude * falloff;

    Vector3 forward = m_target->GetPosition() - cameraPos;
    if (forward.LengthSquared() > 1e-6f)
    {
        forward.Normalize();
    }
    else
    {
        forward = Vector3::Forward;
    }

    XMVECTOR vForward = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&forward));
    XMVECTOR vUp = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&Vector3::Up));
    XMVECTOR vRight = XMVector3Cross(vUp, vForward);

    Vector3 camRight;
    XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&camRight), vRight);

    if (camRight.LengthSquared() > 1e-6f)
    {
        camRight.Normalize();
    }
    else
    {
        camRight = Vector3::Right;
    }

    Vector3 camUp = forward.Cross(camRight);
    if (camUp.LengthSquared() > 1e-6f)
    {
        camUp.Normalize();
    }
    else
    {
        camUp = Vector3::Up;
    }

    switch (m_shakeMode)
    {
        case ShakeMode::Horizontal:
        {
            m_shakeOffset = camRight * (baseAmp * (0.7f * sampleX + 0.3f * sampleY));
            break;
        }
        case ShakeMode::Vertical:
        {
            m_shakeOffset = camUp * (baseAmp * (0.7f * sampleY + 0.3f * sampleX));
            break;
        }
        case ShakeMode::ALL:
        {

            m_shakeOffset =
                camRight * (baseAmp * (0.7f * sampleX + 0.3f * sampleY)) +
                camUp * (baseAmp * (0.7f * sampleY + 0.3f * sampleX));
            break;
        }
    }

    m_shakeTimeRemaining -= dt;
    if (m_shakeTimeRemaining <= 0.0f)
    {
        m_shakeMagnitude = 0.0f;
        m_shakeTimeRemaining = 0.0f;
        m_shakeTotalDuration = 0.0f;
        m_shakePhase = 0.0f;
        m_shakeOffset = Vector3::Zero;
    }
}


void FollowCameraComponent::Update(float dt)
{
    if (!m_target){ return; }

    m_isAiming = Input::IsMouseRightDown();

    POINT delta = Input::GetMouseDelta();
    m_yaw += delta.x * m_sensitivity;
    m_pitch += delta.y * m_sensitivity;

    m_pitch = std::clamp(m_pitch, m_pitchLimitMin, m_pitchLimitMax);
    m_yaw = std::clamp(m_yaw, -m_yawLimit, m_yawLimit);

    UpdateCameraPosition(dt);

    Vector3 springPos = m_spring.GetPosition();
    UpdateShake(dt, springPos);

    Vector3 cameraPos = springPos + m_shakeOffset;

    UpdateAimPoint(dt, cameraPos);
    UpdateLookTarget(dt, cameraPos);

    UpdateFov(dt);

    m_ViewMatrix = Matrix::CreateLookAt(cameraPos, m_LookTarget, Vector3::Up);

    Renderer::SetViewMatrix(m_ViewMatrix);
    Renderer::SetProjectionMatrix(m_ProjectionMatrix);
}

void FollowCameraComponent::UpdateCameraPosition(float dt)
{
    if (!m_target) { return; }

    float desiredDist;
    float height;
    if (m_isAiming)
    {
        desiredDist = m_aimDistance;
        height = m_aimHeight;
    }
    else
    {
        desiredDist = m_defaultDistance;
        height = m_defaultHeight;
    }
    if (m_boostRequested)
    {
        //desiredDist += m_boostAimDistanceAdd; // ブースト時のみ距離を伸ばす
    }

    // 2) プレイヤー情報取得
    Vector3 targetPos = m_target->GetPosition();
    Vector3 targetRot = m_target->GetRotation();
    float playerYaw = targetRot.y;
    Matrix playerRot = Matrix::CreateRotationY(playerYaw);

    Vector3 baseOffset    = Vector3(0.0f, 0.0f, -desiredDist);
    Vector3 rotatedOffset = Vector3::Transform(baseOffset, playerRot);
    Vector3 baseDesired   = targetPos + rotatedOffset + Vector3(0.0f, height, 0.0f);

    // 4) lateral (reticle) および turn offset を計算して baseDesired に加える
    float screenW = static_cast<float>(Application::GetWidth());
    float normX = 0.0f;
    if (screenW > 1.0f) normX = (m_reticleScreen.x - (screenW * 0.5f)) / (screenW * 0.5f);

    Vector3 localRight = Vector3::Transform(Vector3::Right, playerRot);
    float lateral = normX * m_screenOffsetScale;
    lateral = std::clamp(lateral, -m_maxScreenOffset, m_maxScreenOffset);

    float yawDelta = playerYaw - m_prevPlayerYaw;
    while (yawDelta > XM_PI) yawDelta -= XM_2PI;
    while (yawDelta < -XM_PI) yawDelta += XM_2PI;
    float safeDt = std::max(1e-6f, dt);
    float yawSpeed = yawDelta / safeDt;
    float boostScale = 1.0f;
    float turnLateral = yawSpeed * m_turnOffsetScale * boostScale;
    float turnMax = m_turnOffsetMax * boostScale;
    turnLateral = std::clamp(turnLateral, -turnMax, turnMax);
    m_currentTurnOffset = m_currentTurnOffset + (turnLateral - m_currentTurnOffset) * std::min(1.0f, m_turnOffsetLerp * dt);

    Vector3 totalDesired = baseDesired;
    totalDesired += localRight * lateral;
    totalDesired += localRight * m_currentTurnOffset;

    // ============================================================
    // ★ ここに上下反転オフセットを追加 ★
    // ============================================================

     // Player の移動速度（世界座標）
    Vector3 vel = m_target->GetComponent<MoveComponent>()->GetCurrentVelocity();

    // 進行方向の上下成分だけ取り出す
    float vy = vel.y;

    // ▼あなたの理想補正：
    // 上に移動しているとき (vy > 0) → カメラの高さを下げる（Player が画面の下へ）
    // 下に移動しているとき (vy < 0) → カメラを上げる（Player が画面の上へ）
    float cameraYOffset = -vy * 0.15f;
    //                 ▲ここが一番大事な符号！

    // 強すぎないように制限
    cameraYOffset = std::clamp(cameraYOffset, -3.0f, 3.0f);

    // 目的のカメラ位置へ反映
    totalDesired.y += cameraYOffset;


    // ============================================================


    // 5) PlayArea による Y クランプ（高さはここで決定）
    // まず距離に基づく半高さを計算（今回のカメラからターゲットまでの距離は desiredDist）
    // だがスプリングや laterals により多少ずれる可能性があるため、ここは conservative な計算
    Vector3 camToTargetForHalfHeight = targetPos - totalDesired;
    float distAlongForward = camToTargetForHalfHeight.Length();
    if (distAlongForward < 1e-6f) distAlongForward = 1e-6f;
    float halfHeight = std::tan(m_Fov * 0.5f) * distAlongForward;

    if (m_playArea)
    {
        float boundsMinY = m_playArea->GetBoundsMin().y;
        float boundsMaxY = m_playArea->GetBoundsMax().y;
        float minCameraY = boundsMinY + halfHeight;
        float maxCameraY = boundsMaxY - halfHeight;
        if (minCameraY > maxCameraY) {
            float mid = (minCameraY + maxCameraY) * 0.5f;
            minCameraY = maxCameraY = mid;
        }
        totalDesired.y = std::clamp(totalDesired.y, minCameraY, maxCameraY);
    }

    // 6) 非ブースト時は「高さを保持したまま」XZ（水平）を再スケールして
    //    3D 距離が desiredDist になるように強制する（横オフセットを見せつつ距離は固定）
    /*if (!m_boostRequested)
    {*/
        Vector3 toCam = totalDesired - targetPos;
        float heightDiff = toCam.y;
        float distSq = desiredDist * desiredDist;
        float h2 = heightDiff * heightDiff;
        float desiredHoriz = 0.0f;
        if (distSq > h2 + 1e-6f) desiredHoriz = std::sqrt(distSq - h2);
        else desiredHoriz = 0.0f;

        Vector3 horizVec = Vector3(toCam.x, 0.0f, toCam.z);
        float horizLen = horizVec.Length();
        if (horizLen > 1e-6f)
        {
            float scale = desiredHoriz / horizLen;
            totalDesired.x = targetPos.x + horizVec.x * scale;
            totalDesired.z = targetPos.z + horizVec.z * scale;
            // totalDesired.y は既に PlayArea でクランプ済み
        }
        else
        {
            Vector3 forward = Vector3::Transform(Vector3::Forward, Matrix::CreateRotationY(targetRot.y));
            if (forward.LengthSquared() > 1e-6f) forward.Normalize();
            totalDesired.x = targetPos.x + forward.x * desiredHoriz;
            totalDesired.z = targetPos.z + forward.z * desiredHoriz;
        }
    /*}*/

    // 7) safety: スプリング位置が極端に遠ければリセット（初期化不足や過去のバグで巨大値が出るケース対策）
    Vector3 springPos = m_spring.GetPosition();
    const float ABSOLUTE_POS_LIMIT = 10000.0f; // 必要なら調整
    if (std::isnan(springPos.x) || std::isnan(springPos.y) || std::isnan(springPos.z) ||
        springPos.Length() > ABSOLUTE_POS_LIMIT)
    {
        // 初期化（瞬間的にカメラを desired に合わせる）
        m_spring.Reset(totalDesired);
        springPos = m_spring.GetPosition();
    }

    // 8) optional: スプリング差が大きければ一時的に stiffness を上げて追従を速める
    Vector3 desiredPos = totalDesired;
    float springDiff = (desiredPos - springPos).Length();
    const float DIFF_SNAP_THRESHOLD = desiredDist * 0.5f; // 調整可

    float stiffness = m_normalStiffness;
    float damping = m_normalDamping;

    // ★ブースト中は追従を強める（置いていかれ防止）
    if (m_boostRequested)
    {
        stiffness *= 1.6f;
        damping *= 1.2f;
    }

    // ★差が大きい時も追従を強める（ブースト中も含む）
    if (springDiff > DIFF_SNAP_THRESHOLD)
    {
        stiffness *= 1.8f;
        damping *= 1.0f;
    }

    m_spring.SetStiffness(stiffness);
    m_spring.SetDamping(damping);

    // 9) 最後にスプリング更新
    m_spring.Update(desiredPos, dt);

    

    // 10) prev yaw 更新
    m_prevPlayerYaw = playerYaw;

    // 11) 振動処理（既存ロジックを保持）
    m_shakeOffset = Vector3::Zero;
    if (m_shakeTimeRemaining > 0.0f && m_target)
    {
        float t = m_shakeTimeRemaining / std::max(1e-6f, m_shakeTotalDuration);
        float falloff = t;
        m_shakePhase += dt * m_shakeFrequency * 2.0f * XM_PI;
        float sampleX = std::sin(m_shakePhase);
        float sampleY = std::sin(m_shakePhase * 1.5f + 3.1f);
        float baseAmp = m_shakeMagnitude * falloff;

        Vector3 cameraPosTemp = m_spring.GetPosition();
        Vector3 forward = m_target->GetPosition() - cameraPosTemp;
        if (forward.LengthSquared() > 1e-6f)
        {
            forward.Normalize();
            XMVECTOR vForward = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&forward));
            XMVECTOR vUp = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&Vector3::Up));
            XMVECTOR vRight = XMVector3Cross(vUp, vForward);
            Vector3 camRight; XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&camRight), vRight);
            if (camRight.LengthSquared() > 1e-6f)
            {
                camRight.Normalize();
                Vector3 camUp = forward.Cross(camRight);
                if (camUp.LengthSquared() > 1e-6f) camUp.Normalize();

                switch (m_shakeMode)
                {
                case ShakeMode::Horizontal:
                    m_shakeOffset = camRight * (baseAmp * (0.7f * sampleX + 0.3f * sampleY));
                    break;
                case ShakeMode::Vertical:
                    m_shakeOffset = camUp * (baseAmp * (0.7f * sampleY + 0.3f * sampleX));
                    break;
                case ShakeMode::ALL:
                    m_shakeOffset = camRight * (baseAmp * (0.7f * sampleX + 0.3f * sampleY))
                        + camUp * (baseAmp * (0.7f * sampleY + 0.3f * sampleX));
                    break;
                }
            }
        }
        m_shakeTimeRemaining -= dt;
        if (m_shakeTimeRemaining <= 0.0f)
        {
            m_shakeMagnitude = 0.0f;
            m_shakeTimeRemaining = 0.0f;
            m_shakeTotalDuration = 0.0f;
            m_shakePhase = 0.0f;
            m_shakeOffset = Vector3::Zero;
        }
    }
}


/*void FollowCameraComponent::UpdateCameraPosition(float dt)
{
    if (!m_target){ return; }

    char buf[256];
    sprintf_s(buf, "DBG: dist=%.2f height=%.2f\n", m_defaultDistance, m_defaultHeight);
    OutputDebugStringA(buf);

    Vector3 targetPos = m_target->GetPosition();
    Vector3 targetRot = m_target->GetRotation();

    Matrix rotY = Matrix::CreateRotationY(targetRot.y);

    Vector3 back = Vector3::Transform(Vector3(0.0f, 0.0f, -m_defaultDistance), rotY);
    Vector3 desiredPos = targetPos + back + Vector3(0.0f, m_defaultHeight, 0.0f);

    //m_spring.Reset(desiredPos);
    //スプリングで自然追従
    m_spring.Update(desiredPos, dt);
}*/

void FollowCameraComponent::Shake(float magnitude, float duration , ShakeMode mode)
{
    if (magnitude <= 0.0f || duration <= 0.0f) { return; }

	m_shakeMagnitude     = std::max(m_shakeMagnitude, magnitude);
    m_shakeTimeRemaining = std::max(m_shakeTimeRemaining, duration);
    m_shakeTotalDuration = std::max(m_shakeTotalDuration, duration);

   m_shakeMode = mode;

    m_shakePhase = 0.0f;
}
