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

using namespace DirectX;
using namespace DirectX::SimpleMath;

FollowCameraComponent::FollowCameraComponent()
{
    // スプリングの初期設定（通常時）
    m_normalStiffness = 12.0f;
    m_normalDamping = 6.0f;
    m_Spring.SetStiffness(m_normalStiffness);
    m_Spring.SetDamping(m_normalDamping);
    m_Spring.SetMass(1.0f);

    // プロジェクション
    float width = static_cast<float>(Application::GetWidth());
    float height = static_cast<float>(Application::GetHeight());
    m_ProjectionMatrix = Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(45.0f), width / height, 0.1f, 1000.0f);

    // ブースト関連初期値
    m_boostRequested = false;
    m_boostBlend = 0.0f;
    m_boostBlendSpeed = 6.0f;

	//カメラ振動用の乱数初期化
    //m_shakeRng.seed(123456789ULL);
}

void FollowCameraComponent::SetTarget(GameObject* target)
{
    m_Target = target;
    if (target)
    {
        Vector3 initial = target->GetPosition() + Vector3(0, m_DefaultHeight, -m_DefaultDistance);
        m_Spring.Reset(initial);
        m_PrevPlayerYaw = m_Target->GetRotation().y;
        m_CurrentTurnOffset = 0.0f;
    }
}

void FollowCameraComponent::SetBoostState(bool isBoosting)
{
    // ブースト中かどうかのStateをセット
    m_boostRequested = isBoosting;
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
    if (!m_Target)
    {
        return GetForward();
    }

    Vector3 cameraPos = m_Spring.GetPosition();
    Vector3 targetPos = m_Target->GetPosition();
    Matrix provisionalView = Matrix::CreateLookAt(cameraPos, targetPos, Vector3::Up);

    float screenW = static_cast<float>(Application::GetWidth());
    float screenH = static_cast<float>(Application::GetHeight());
    float sx = m_ReticleScreen.x;
    float sy = m_ReticleScreen.y;

    XMMATRIX projXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_ProjectionMatrix));
    XMMATRIX viewXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&provisionalView));
    XMMATRIX worldXM = XMMatrixIdentity();

    XMVECTOR nearScreen = XMVectorSet(sx, sy, 0.0f, 1.0f);
    XMVECTOR farScreen = XMVectorSet(sx, sy, 1.0f, 1.0f);

    XMVECTOR nearWorldV = XMVector3Unproject(
        nearScreen, 0.0f, 0.0f, screenW, screenH, 0.0f, 1.0f, projXM, viewXM, worldXM);
    XMVECTOR farWorldV = XMVector3Unproject(
        farScreen, 0.0f, 0.0f, screenW, screenH, 0.0f, 1.0f, projXM, viewXM, worldXM);

    Vector3 nearWorld(XMVectorGetX(nearWorldV), XMVectorGetY(nearWorldV), XMVectorGetZ(nearWorldV));
    Vector3 farWorld(XMVectorGetX(farWorldV), XMVectorGetY(farWorldV), XMVectorGetZ(farWorldV));

    Vector3 dir = farWorld - nearWorld;
    if (dir.LengthSquared() > 1e-6f)
    {
        dir.Normalize();
    }
    else
    {
        dir = GetForward();
    }

    // --- 垂直スケール適用 ---
    dir.y *= m_VerticalAimScale;
    if (dir.LengthSquared() > 1e-6f)
    {
        dir.Normalize();
    }
    else
    {
        // もしスケールでゼロに近くなったらフォワードを返す
        dir = GetForward();
    }

    return dir;
}

void FollowCameraComponent::Update(float dt)
{
    if (!m_Target)
    {
        return;
    }

    m_IsAiming = Input::IsMouseRightDown();
    POINT delta = Input::GetMouseDelta();
    m_Yaw += delta.x * m_Sensitivity;
    m_Pitch += delta.y * m_Sensitivity;
    m_Pitch = std::clamp(m_Pitch, m_PitchLimitMin, m_PitchLimitMax);
    m_Yaw = std::clamp(m_Yaw, -m_YawLimit, m_YawLimit);

    // ブーストブレンドを目標に向かって滑らかに変化
    float targetBlend;
    if (m_boostRequested)
    {
        targetBlend = 1.0f;
    }
    else
    {
        targetBlend = 0.0f;
    }

    float blendDelta = (targetBlend - m_boostBlend);
    float maxStep = std::min(1.0f, m_boostBlendSpeed * dt);
    if (fabs(blendDelta) <= maxStep)
    {
        m_boostBlend = targetBlend;
    }
    else
    {
        if (blendDelta > 0.0f)
        {
            m_boostBlend += maxStep;
        }
        else
        {
            m_boostBlend -= maxStep;
        }
    }

    // カメラ位置の更新（今回はブーストで距離のみ変動）
    UpdateCameraPosition(dt);

    // カメラと追尾対象のPos取得
    Vector3 cameraPos = m_Spring.GetPosition();
    cameraPos += m_shakeOffset;
    Vector3 targetPos = m_Target->GetPosition();

    float screenW = static_cast<float>(Application::GetWidth());
    float screenH = static_cast<float>(Application::GetHeight());
    float sx = m_ReticleScreen.x;
    float sy = m_ReticleScreen.y;

    Matrix provisionalView = Matrix::CreateLookAt(cameraPos, targetPos, Vector3::Up);
    XMVECTOR nearScreen = XMVectorSet(sx, sy, 0.0f, 1.0f);
    XMVECTOR farScreen = XMVectorSet(sx, sy, 1.0f, 1.0f);

    XMMATRIX projXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_ProjectionMatrix));
    XMMATRIX viewXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&provisionalView));
    XMMATRIX worldXM = XMMatrixIdentity();

    XMVECTOR nearWorldV = XMVector3Unproject(
        nearScreen, 0.0f, 0.0f, screenW, screenH, 0.0f, 1.0f, projXM, viewXM, worldXM);
    XMVECTOR farWorldV = XMVector3Unproject(
        farScreen, 0.0f, 0.0f, screenW, screenH, 0.0f, 1.0f, projXM, viewXM, worldXM);

    Vector3 nearWorld(XMVectorGetX(nearWorldV), XMVectorGetY(nearWorldV), XMVectorGetZ(nearWorldV));
    Vector3 farWorld(XMVectorGetX(farWorldV), XMVectorGetY(farWorldV), XMVectorGetZ(farWorldV));

    Vector3 rayDir = farWorld - nearWorld;
    if (rayDir.LengthSquared() > 1e-6f)
    {
        rayDir.Normalize();
    }

    Vector3 camForward = GetForward();

    // ※変更点：ブーストで AimPlaneDistance を変えない（注視計算は常に同じ）
    float localAimPlaneDist = m_AimPlaneDistance;

    Vector3 planePoint = cameraPos + camForward * localAimPlaneDist;
    float denom = rayDir.Dot(camForward);

    Vector3 worldTarget;
    const float EPS = 1e-5f;
    if (fabs(denom) < EPS)
    {
        worldTarget = nearWorld + rayDir * localAimPlaneDist;
    }
    else
    {
        float t = (planePoint - nearWorld).Dot(camForward) / denom;
        worldTarget = nearWorld + rayDir * t;
    }

    const float aimLerp = 12.0f;
    m_AimPoint = m_AimPoint + (worldTarget - m_AimPoint) * std::min(1.0f, aimLerp * dt);

    Vector3 aimDir = m_AimPoint - targetPos;
    if (aimDir.LengthSquared() > 1e-6f)
    {
        aimDir.Normalize();
    }
    else
    {
        aimDir = GetForward();
    }

    // --- 垂直スケールをここで適用してから正規化 ---
    aimDir.y = 0;
    if (aimDir.LengthSquared() > 1e-6f)
    {
        aimDir.Normalize();
    }
    else
    {
        aimDir = GetForward();
    }

    Vector3 rawLookTarget = targetPos + aimDir * m_LookAheadDistance + Vector3(0.0f, (m_DefaultHeight + m_AimHeight) * 0.5f, 0.0f);

    Vector3 targetRot = m_Target->GetRotation();
    Matrix  playerRot = Matrix::CreateRotationY(targetRot.y);
    Vector3 localRight = Vector3::Transform(Vector3::Right, playerRot);

    // ※変更点：lookOffsetMul はブーストに依存させない（常に 0.6）
    float lookOffsetMul = 0.6f;
    rawLookTarget += localRight * (m_CurrentTurnOffset * lookOffsetMul);

    m_LookTarget = m_LookTarget + (rawLookTarget - m_LookTarget) * std::min(1.0f, m_LookAheadLerp * dt);

    m_ViewMatrix = Matrix::CreateLookAt(cameraPos, m_LookTarget, Vector3::Up);

    Renderer::SetViewMatrix(m_ViewMatrix);
    Renderer::SetProjectionMatrix(m_ProjectionMatrix);
}

void FollowCameraComponent::UpdateCameraPosition(float dt)
{
    if (!m_Target)
    {
        return;
    }

    float dist;
    float height;

    if (m_IsAiming)
    {
        dist = m_AimDistance;
        height = m_AimHeight;
    }
    else
    {
        dist = m_DefaultDistance;
        height = m_DefaultHeight;
    }

    // ブースト時は目標距離を少し伸ばす（カメラが後ろに残る印象）
    dist += m_boostBlend * m_boostAimDistanceAdd;

    Vector3 targetPos = m_Target->GetPosition();
    Vector3 targetRot = m_Target->GetRotation();
    float playerYaw = targetRot.y;

    Matrix playerRot = Matrix::CreateRotationY(playerYaw);
    Vector3 baseOffset = Vector3(0.0f, 0.0f, -dist);
    Vector3 rotatedOffset = Vector3::Transform(baseOffset, playerRot);

    Vector3 desiredPos = targetPos + rotatedOffset + Vector3(0.0f, height, 0.0f);

    float desiredY = desiredPos.y;
    float maxY = targetPos.y + 6.0f;
    if (desiredY > maxY)
    {
        desiredY = maxY;
    }
    if (desiredY < -8.0f)
    {
        desiredY = -8.0f;
    }
    desiredPos.y = desiredY;

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
    while (yawDelta > XM_PI)
    {
        yawDelta -= XM_2PI;
    }
    while (yawDelta < -XM_PI)
    {
        yawDelta += XM_2PI;
    }

    float safeDt = std::max(1e-6f, dt);
    float yawSpeed = yawDelta / safeDt;

    // ※変更点：boostScale を常に 1.0 に（ブーストで横オフセットを変えない）
    float boostScale = 1.0f;
    float turnLateral = yawSpeed * m_TurnOffsetScale * boostScale;
    float turnMax = m_TurnOffsetMax * boostScale;
    turnLateral = std::clamp(turnLateral, -turnMax, turnMax);

    m_CurrentTurnOffset = m_CurrentTurnOffset + (turnLateral - m_CurrentTurnOffset) * std::min(1.0f, m_TurnOffsetLerp * dt);

    // カメラ本体の横移動（プレイヤーの向きに応じてカメラを横にずらす）
    desiredPos += localRight * m_CurrentTurnOffset;

    // 画面上下がワールドでどこかを計算するために、
    // カメラ前方方向と target までの距離を使う
    Vector3 camToTarget = targetPos - desiredPos;
    float distAlongForward = camToTarget.Length(); // >0

    // 安全対策
    if (distAlongForward < 1e-6f) distAlongForward = 1e-6f;

    // 垂直方向の半分のワールド長（カメラが target 平面で見る半高さ）
    // m_Fov はラジアン
    float halfHeight = std::tan(m_Fov * 0.5f) * distAlongForward;

    // PlayArea が渡されていれば Y 方向でクランプする
    if (m_playArea)
    {
        float boundsMinY = m_playArea->GetBoundsMin().y; // 下限（PlayAreaComponent に GetBoundsMin を用意）
        float boundsMaxY = m_playArea->GetBoundsMax().y; // 上限

        // カメラの Y が満たすべき範囲
        float minCameraY = boundsMinY + halfHeight;
        float maxCameraY = boundsMaxY - halfHeight;

        // もし領域が狭くて半高さの2倍より小さい場合は中央へ寄せる（オーバーラップ防止）
        if (minCameraY > maxCameraY)
        {
            float mid = (minCameraY + maxCameraY) * 0.5f;
            minCameraY = maxCameraY = mid;
        }

        // クランプ
        desiredPos.y = std::clamp(desiredPos.y, minCameraY, maxCameraY);
    }

    // スプリングを更新（追従用）
    m_Spring.Update(desiredPos, dt);

	m_PrevPlayerYaw = playerYaw;

    //振動計算
    m_shakeOffset = Vector3::Zero;

	if (m_shakeTimeRemaining > 0.0f && m_Target)
    {
       //残り時間計算
		float t = m_shakeTimeRemaining / std::max(1e-6f, m_shakeTotalDuration);
        float falloff = t;

		//振動位相更新
		m_shakePhase += dt * m_shakeFrequency * 2.0f * XM_PI;

        //雑な複合波形
        float sample = std::sin(m_shakePhase)
            + 0.5f * std::sin(3.0f * m_shakePhase + 1.7f);
		
		float offsetAmount = m_shakeMagnitude  * sample * falloff;

        //描画時の右方向
		Vector3 cameraPosTemp = m_Spring.GetPosition();

        Vector3 forward = m_Target->GetPosition() - cameraPosTemp;
        if (forward.LengthSquared() > 1e-6f)
        {
            forward.Normalize();
            // DirectXMath で Up x forward を計算
            XMVECTOR vForward = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&forward));
            XMVECTOR vUp = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&Vector3::Up));
            XMVECTOR vRight = XMVector3Cross(vUp, vForward);
            Vector3 camRight;
            XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&camRight), vRight);

            if (camRight.LengthSquared() > 1e-6f)
            {
                camRight.Normalize();
                // **m_shakeOffset に格納（描画時に加算する）**
                m_shakeOffset = camRight * offsetAmount;
            }
        }

        // タイマーを減らす（ここで行う）
        m_shakeTimeRemaining -= dt;
        if (m_shakeTimeRemaining <= 0.0f)
        {
            // リセット
            m_shakeMagnitude = 0.0f;
            m_shakeTimeRemaining = 0.0f;
            m_shakeTotalDuration = 0.0f;
            m_shakePhase = 0.0f;
            m_shakeOffset = Vector3::Zero;
        }
    }
}


void FollowCameraComponent::Shake(float magnitude, float duration)
{
    if (magnitude <= 0.0f || duration <= 0.0f) { return; }

    // 既存のシェイクより強ければ上書き、残り時間も延長できるように最大値を取る
    m_shakeMagnitude     = std::max(m_shakeMagnitude, magnitude);
    m_shakeTimeRemaining = std::max(m_shakeTimeRemaining, duration);
    m_shakeTotalDuration = std::max(m_shakeTotalDuration, duration);

    // フェーズはリセット（任意でランダムにしても良い）
    m_shakePhase = 0.0f;
}
