#include "MoveComponent.h"
#include "PlayAreaComponent.h"
#include "GameObject.h"
#include "Application.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace DirectX::SimpleMath;

static float LerpExpFactor(float k, float dt)
{
    return 1.0f - std::exp(-k * dt);
}

static float NormalizeAngleDelta(float a)
{
    while (a > XM_PI)
    {
        a -= XM_2PI;
    }
    while (a < -XM_PI)
    {
        a += XM_2PI;
    }
    return a;
}

void MoveComponent::Initialize()
{
    if (GetOwner())
    {
		//前フレームのyawを保存
        m_prevYaw = GetOwner()->GetRotation().y;
    }
    else
    {
        m_prevYaw = 0.0f;
    }

    m_visualPitchTilt = 0.0f;
    m_collisionCorrectedThisFrame = false;
}

void MoveComponent::Uninit()
{

}


void MoveComponent::Update(float dt)
{
    using namespace DirectX::SimpleMath;

    // カメラ or オーナーがなければ何もしない
    if (!m_camera || !GetOwner()) { return; }
    if (dt <= 0.0f) { return; }

    GameObject* owner = GetOwner();

    // ★★★ ここが今回一番大事な「追加部分」 ★★★
    // 前フレームの CollisionManager::CheckCollisions で溜めた押し出しを
    // いちばん最初に位置に反映してしまう
    Vector3 pos = owner->GetPosition();
    if (m_hasPushThisFrame)
    {
        // 位置補正
        pos += m_totalPushThisFrame;

        // 押し出し方向にまだ突っ込んでいる速度があれば消す
        if (m_velocity.Dot(m_totalPushThisFrame) < 0.0f)
        {
            m_velocity = Vector3::Zero;
        }
        if (m_externalVelocity.Dot(m_totalPushThisFrame) < 0.0f)
        {
            m_externalVelocity = Vector3::Zero;
        }

        // 一度使ったらリセット（次の衝突まで 0）
        m_totalPushThisFrame = Vector3::Zero;
        m_hasPushThisFrame = false;
    }
    // ★★★ ここまで追加 ★★★

    // 位置補正済みの pos からスタートする
    Vector3 rot = owner->GetRotation();

    float currentYaw = rot.y;
    float currentPitch = m_currentPitch;

    // -------------- ブーストまわり（ここはそのままでOK） --------------
    bool keyDown = Input::IsKeyPressed(m_boostKey);

    bool startBoost = false;
    if (keyDown && !m_prevBoostKeyDown && m_cooldownTimer <= 0.0f && !m_isBoosting)
    {
        startBoost = true;
    }
    m_prevBoostKeyDown = keyDown;

    if (startBoost)
    {
        m_isBoosting = true;
        m_boostTimer = 0.0f;
        m_recoverTimer = -1.0f;
        m_cooldownTimer = m_boostCooldown;
        if (m_camera)
        {
            m_camera->SetBoostState(true);
        }
    }

    if (m_isBoosting)
    {
        m_boostTimer += dt;
        if (m_boostTimer >= m_boostSeconds)
        {
            m_isBoosting = false;
            m_recoverTimer = 0.0f;
            if (m_camera)
            {
                m_camera->SetBoostState(false);
            }
        }
    }

    if (m_cooldownTimer > 0.0f)
    {
        m_cooldownTimer -= dt;
        if (m_cooldownTimer < 0.0f) m_cooldownTimer = 0.0f;
    }

    // ----------------- 速度決定 -----------------
    float currentSpeed = m_baseSpeed;
    if (m_isBoosting)
    {
        currentSpeed = m_baseSpeed * m_boostMultiplier;
    }
    else if (m_recoverTimer >= 0.0f && m_recoverTimer < m_boostRecover)
    {
        m_recoverTimer += dt;
        float t = std::clamp(m_recoverTimer / m_boostRecover, 0.0f, 1.0f);
        float ease = 1.0f - (1.0f - t) * (1.0f - t);
        float currentMultiplier = 1.0f + (m_boostMultiplier - 1.0f) * (1.0f - ease);
        currentSpeed = m_baseSpeed * currentMultiplier;
    }

    // ----------------- 通常移動（レティクル方向を向く） -----------------
    Vector3 aimTarget = m_camera->GetAimPoint();
    Vector3 toTarget = aimTarget - pos;

    if (toTarget.LengthSquared() < 1e-6f)
    {
        // 目標点がほぼ現在位置なら、今の向きでまっすぐ進むだけ
        Vector3 forward(
            std::sin(currentYaw) * std::cos(currentPitch),
            std::sin(currentPitch),
            std::cos(currentYaw) * std::cos(currentPitch)
        );
        forward.Normalize();

        m_velocity = forward * currentSpeed + m_externalVelocity;

        // 位置更新
        pos += m_velocity * dt;

        // PlayArea 補正
        if (m_playArea)
        {
            pos = m_playArea->ResolvePosition(owner->GetPosition(), pos);
        }
        else
        {
            if (pos.y < -1.0f)
            {
                pos.y = -1.0f;
            }
        }

        // ★ ここではもう MTV 適用処理は書かない（上で済ませている）
        owner->SetPosition(pos);

        m_currentPitch = currentPitch;
        return;
    }

    // レティクル方向を目標方向とする
    Vector3 desired = toTarget;
    desired.Normalize();
    if (desired.LengthSquared() < 1e-6f)
    {
        desired = Vector3(0, 0, 1);
    }

    float targetYaw = std::atan2(desired.x, desired.z);
    float horiz = std::sqrt(desired.x * desired.x + desired.z * desired.z);
    float targetPitch = std::atan2(desired.y, horiz);

    // 差分を -PI..PI に正規化
    float deltaYaw = NormalizeAngleDelta(targetYaw - currentYaw);
    float deltaPitch = NormalizeAngleDelta(targetPitch - currentPitch);

    float yawAlpha = LerpExpFactor(m_rotSmoothK, dt);
    float pitchAlpha = LerpExpFactor(m_rotSmoothK, dt);

    float maxYawChange = m_rotateSpeed * dt;
    float maxPitchChange = m_pitchSpeed * dt;
    if (m_isBoosting)
    {
        float boostTurnFactor = 0.5f;
        maxYawChange *= boostTurnFactor;
        maxPitchChange *= boostTurnFactor;
    }

    float appliedYaw = std::clamp(deltaYaw * yawAlpha, -maxYawChange, maxYawChange);
    currentYaw += appliedYaw;

    float appliedPitch = std::clamp(deltaPitch * pitchAlpha, -maxPitchChange, maxPitchChange);
    currentPitch += appliedPitch;

    // 新しい前方ベクトル
    Vector3 newForward(
        std::sin(currentYaw) * std::cos(currentPitch),
        std::sin(currentPitch),
        std::cos(currentYaw) * std::cos(currentPitch)
    );
    if (newForward.LengthSquared() > 1e-6f) newForward.Normalize();
    else newForward = Vector3(0, 0, 1);

    // -------- ロール / 視覚ピッチ演出（ここはそのままでOK） --------
    Vector3 cross = newForward.Cross(desired);
    float   lateral = cross.y;

    float safeDt = max(1e-6f, dt);
    float yawDeltaForRoll = NormalizeAngleDelta(currentYaw - m_prevYaw);
    float yawSpeed = yawDeltaForRoll / safeDt;

    float speedRatio = (m_baseSpeed > 1e-6f) ? (currentSpeed / m_baseSpeed) : 1.0f;
    float fromYaw = yawSpeed * m_rollYawFactor;
    float fromLateral = -lateral * m_rollLateralFactor;
    float speedScale = std::clamp(speedRatio * m_rollSpeedScale, 0.5f, 2.0f);

    float rawRoll = (fromYaw + fromLateral) * speedScale;
    rawRoll = std::clamp(rawRoll, -m_maxVisualRoll, m_maxVisualRoll);

    float rollAlpha = LerpExpFactor(m_rollLerpK, dt);
    m_currentRoll = m_currentRoll + (rawRoll - m_currentRoll) * rollAlpha;

    float vertComponent = newForward.y * currentSpeed;
    float pitchTargetTilt = -std::atan(vertComponent / m_pitchSaturationFactor) * m_verticalTiltFactor;
    pitchTargetTilt = std::clamp(pitchTargetTilt, -m_maxVerticalTilt, m_maxVerticalTilt);

    float pitchTiltAlpha = LerpExpFactor(m_pitchTiltSmoothK, dt);
    float newVisualPitch = m_visualPitchTilt + (pitchTargetTilt - m_visualPitchTilt) * pitchTiltAlpha;
    const float maxDeltaRad =
        (m_maxPitchDeltaDegPerSec * (3.14159265358979323846f / 180.0f)) * dt;
    float delta = newVisualPitch - m_visualPitchTilt;
    if (delta > maxDeltaRad) delta = maxDeltaRad;
    if (delta < -maxDeltaRad) delta = -maxDeltaRad;
    m_visualPitchTilt += delta;

    // 回転セット
    rot.x = -currentPitch + m_visualPitchTilt;
    rot.y = currentYaw;
    rot.z = -m_currentRoll;
    owner->SetRotation(rot);
    m_prevYaw = currentYaw;

    // -------- 速度と位置更新 --------
    m_velocity = newForward * currentSpeed + m_externalVelocity;

    pos += m_velocity * dt;

    if (m_playArea)
    {
        pos = m_playArea->ResolvePosition(owner->GetPosition(), pos);
    }
    else
    {
        if (pos.y < -1.0f)
        {
            pos.y = -1.0f;
        }
    }

    // ★ ここにも MTV の適用処理は書かない（上で済ませている）

    owner->SetPosition(pos);

    m_currentPitch = currentPitch;
}

/*void MoveComponent::Update(float dt)
{
    using namespace DirectX::SimpleMath;

    // カメラ or オーナーがなければ何もしない
    if (!m_camera || !GetOwner()) { return; }
    if (dt <= 0.0f) { return; }

    GameObject* owner = GetOwner();

    // 現時点でのPlayerの位置・回転取得
    Vector3 pos = owner->GetPosition();
    Vector3 rot = owner->GetRotation();

    float currentYaw = rot.y;
    float currentPitch = m_currentPitch;

    // ----------------- ブースト入力まわり（あなたの元コードのまま） -----------------
    bool keyDown = Input::IsKeyPressed(m_boostKey);

    bool startBoost = false;
    if (keyDown && !m_prevBoostKeyDown && m_cooldownTimer <= 0.0f && !m_isBoosting)
    {
        startBoost = true;
    }
    m_prevBoostKeyDown = keyDown;

    if (startBoost)
    {
        m_isBoosting = true;
        m_boostTimer = 0.0f;
        m_recoverTimer = -1.0f;
        m_cooldownTimer = m_boostCooldown;
        if (m_camera)
        {
            m_camera->SetBoostState(true);
        }
    }

    if (m_isBoosting)
    {
        m_boostTimer += dt;
        if (m_boostTimer >= m_boostSeconds)
        {
            m_isBoosting = false;
            m_recoverTimer = 0.0f;
            if (m_camera)
            {
                m_camera->SetBoostState(false);
            }
        }
    }

    if (m_cooldownTimer > 0.0f)
    {
        m_cooldownTimer -= dt;
        if (m_cooldownTimer < 0.0f) m_cooldownTimer = 0.0f;
    }

    // ----------------- 速度決定 -----------------
    float currentSpeed = m_baseSpeed;
    if (m_isBoosting)
    {
        currentSpeed = m_baseSpeed * m_boostMultiplier;
    }
    else if (m_recoverTimer >= 0.0f && m_recoverTimer < m_boostRecover)
    {
        m_recoverTimer += dt;
        float t = std::clamp(m_recoverTimer / m_boostRecover, 0.0f, 1.0f);
        float ease = 1.0f - (1.0f - t) * (1.0f - t);
        float currentMultiplier = 1.0f + (m_boostMultiplier - 1.0f) * (1.0f - ease);
        currentSpeed = m_baseSpeed * currentMultiplier;
    }

    // ----------------- 通常移動（レティクル方向を向く） -----------------
    Vector3 aimTarget = m_camera->GetAimPoint();
    Vector3 toTarget = aimTarget - pos;

    if (toTarget.LengthSquared() < 1e-6f)
    {
        // 目標点がほぼ現在位置なら、今の向きでまっすぐ進むだけ
        Vector3 forward(
            std::sin(currentYaw) * std::cos(currentPitch),
            std::sin(currentPitch),
            std::cos(currentYaw) * std::cos(currentPitch)
        );
        forward.Normalize();

        m_velocity = forward * currentSpeed + m_externalVelocity;

        // 位置更新
        pos += m_velocity * dt;

        // PlayArea 補正
        if (m_playArea)
        {
            pos = m_playArea->ResolvePosition(owner->GetPosition(), pos);
        }
        else
        {
            if (pos.y < -1.0f)
            {
                pos.y = -1.0f;
            }
        }
        owner->SetPosition(pos);

        m_currentPitch = currentPitch;
        return;
    }

    // レティクル方向そのものを目標方向とする
    Vector3 desired = toTarget;
    desired.Normalize();
    if (desired.LengthSquared() < 1e-6f)
    {
        desired = Vector3(0, 0, 1);
    }

    float targetYaw = std::atan2(desired.x, desired.z);
    float horiz = std::sqrt(desired.x * desired.x + desired.z * desired.z);
    float targetPitch = std::atan2(desired.y, horiz);

    // 差分を -PI..PI に正規化
    float deltaYaw = NormalizeAngleDelta(targetYaw - currentYaw);
    float deltaPitch = NormalizeAngleDelta(targetPitch - currentPitch);

    float yawAlpha = LerpExpFactor(m_rotSmoothK, dt);
    float pitchAlpha = LerpExpFactor(m_rotSmoothK, dt);

    float maxYawChange = m_rotateSpeed * dt;
    float maxPitchChange = m_pitchSpeed * dt;
    if (m_isBoosting)
    {
        float boostTurnFactor = 0.5f;
        maxYawChange *= boostTurnFactor;
        maxPitchChange *= boostTurnFactor;
    }

    float appliedYaw = std::clamp(deltaYaw * yawAlpha, -maxYawChange, maxYawChange);
    currentYaw += appliedYaw;

    float appliedPitch = std::clamp(deltaPitch * pitchAlpha, -maxPitchChange, maxPitchChange);
    currentPitch += appliedPitch;

    // 新しい前方ベクトル
    Vector3 newForward(
        std::sin(currentYaw) * std::cos(currentPitch),
        std::sin(currentPitch),
        std::cos(currentYaw) * std::cos(currentPitch)
    );
    if (newForward.LengthSquared() > 1e-6f) newForward.Normalize();
    else newForward = Vector3(0, 0, 1);

    // -------- ここから下はロール演出 / ピッチ演出（あなたが使っているもの） --------
    // ※ ここは今まで動いていたなら、そのままコピペでOK

    // ロール演出
    Vector3 cross = newForward.Cross(desired);
    float   lateral = cross.y;

    float safeDt = max(1e-6f, dt);
    float yawDeltaForRoll = NormalizeAngleDelta(currentYaw - m_prevYaw);
    float yawSpeed = yawDeltaForRoll / safeDt;

    float speedRatio = (m_baseSpeed > 1e-6f) ? (currentSpeed / m_baseSpeed) : 1.0f;
    float fromYaw = yawSpeed * m_rollYawFactor;
    float fromLateral = -lateral * m_rollLateralFactor;
    float speedScale = std::clamp(speedRatio * m_rollSpeedScale, 0.5f, 2.0f);

    float rawRoll = (fromYaw + fromLateral) * speedScale;
    rawRoll = std::clamp(rawRoll, -m_maxVisualRoll, m_maxVisualRoll);

    float rollAlpha = LerpExpFactor(m_rollLerpK, dt);
    m_currentRoll = m_currentRoll + (rawRoll - m_currentRoll) * rollAlpha;

    // 垂直ピッチ演出
    float vertComponent = newForward.y * currentSpeed;
    float pitchTargetTilt = -std::atan(vertComponent / m_pitchSaturationFactor) * m_verticalTiltFactor;
    pitchTargetTilt = std::clamp(pitchTargetTilt, -m_maxVerticalTilt, m_maxVerticalTilt);

    float pitchTiltAlpha = LerpExpFactor(m_pitchTiltSmoothK, dt);
    float newVisualPitch = m_visualPitchTilt + (pitchTargetTilt - m_visualPitchTilt) * pitchTiltAlpha;

    const float maxDeltaRad =
        (m_maxPitchDeltaDegPerSec * (3.14159265358979323846f / 180.0f)) * dt;
    float delta = newVisualPitch - m_visualPitchTilt;
    if (delta > maxDeltaRad)  delta = maxDeltaRad;
    if (delta < -maxDeltaRad) delta = -maxDeltaRad;
    m_visualPitchTilt += delta;

    // 回転セット
    rot.x = -currentPitch + m_visualPitchTilt;
    rot.y = currentYaw;
    rot.z = -m_currentRoll;
    owner->SetRotation(rot);
    m_prevYaw = currentYaw;

    // -------- 速度と位置更新 --------
    m_velocity = newForward * currentSpeed + m_externalVelocity;

    // ※ ここでは衝突のことは考えず「理想の位置」まで動かす
    pos += m_velocity * dt;

    if (m_playArea)
    {
        pos = m_playArea->ResolvePosition(owner->GetPosition(), pos);
    }
    else
    {
        if (pos.y < -1.0f)
        {
            pos.y = -1.0f;
        }
    }

    if (m_hasPushThisFrame)
    {
        // 位置をまとめて修正
        pos += m_totalPushThisFrame;

        // 押し出し方向に向かっている速度を 0 にする（再び突っ込まないように）
        if (m_velocity.Dot(m_totalPushThisFrame) < 0.0f)
        {
            m_velocity = Vector3::Zero;
        }

        // 次フレームのためリセット
        m_totalPushThisFrame = Vector3::Zero;
        m_hasPushThisFrame = false;
    }

    // 最終的に SetPosition
    owner->SetPosition(pos);

    m_currentPitch = currentPitch;
}*/

void MoveComponent::HandleCollisionCorrection(
    const DirectX::SimpleMath::Vector3& push,
    const DirectX::SimpleMath::Vector3& contactNormal)
{
    using namespace DirectX::SimpleMath;

    GameObject* owner = GetOwner();
    if (!owner) { return; }

    // 押し出しがほぼゼロなら無視
    if (push.LengthSquared() < 1e-8f)
    {
        return;
    }

    // --- 押し出し方向ベクトル（法線）を決める ---
    Vector3 n;

    if (contactNormal.LengthSquared() > 1e-8f)
    {
        n = contactNormal;
        n.Normalize();
    }
    else
    {
        // contactNormal が変なら push 方向から作る
        Vector3 tmp = push;
        if (tmp.LengthSquared() > 1e-8f)
        {
            tmp.Normalize();
        }
        else
        {
            tmp = Vector3::Up;
        }
        n = tmp;
    }

    //位置補正
    m_totalPushThisFrame += push;
    m_hasPushThisFrame = true;

    //壁の内側に向かう速度成分を削る
    auto KillInwardComponent = [&](Vector3& v)
        {
            if (v.LengthSquared() < 1e-8f) return;

            float vn = v.Dot(n);
            // vn < 0 → 法線の逆向き（中に向かっている）
            if (vn < 0.0f)
            {
                v = v - n * vn;
            }
        };

    KillInwardComponent(m_velocity);
    KillInwardComponent(m_externalVelocity);
}


/*void MoveComponent::HandleCollisionCorrection(const DirectX::SimpleMath::Vector3& push,
                                              const DirectX::SimpleMath::Vector3& contactNormal)
{
    GameObject* owner = GetOwner();
    if (!owner) { return; }

    //位置補正(めり込みを押し出す)
    //現在の位置＋必要な押し出し量
    owner->SetPosition(owner->GetPosition() + push);

    //速度補正（壁に向かう成分を消す）
    //壁の法線方向へ突っ込む速度を調べる
    float vn = m_velocity.Dot(contactNormal);

    //もし vn < 0(すこしでも壁の方向へ進んでいる)なら
    if (vn < 0.0f)
    {
        //法線方向の成分を除く
        m_velocity = m_velocity - contactNormal * vn;
    }

    //外部インパルス(外部力)がある場合も同様に処理
    float vn2 = m_externalVelocity.Dot(contactNormal);

    //もし vn2 < 0(すこしでも壁の方向へ進んでいる)なら
    if (vn2 < 0.0f)
    {
        //同じように法線方向の成分を除く
        m_externalVelocity = m_externalVelocity - contactNormal * vn2;
    }
}*/



void MoveComponent::ApplyCollisionPush()
{
    using namespace DirectX::SimpleMath;

    if (!m_hasPushThisFrame || !GetOwner())
    {
        // 何もなければリセットだけ
        m_totalPushThisFrame = Vector3::Zero;
        m_hasPushThisFrame = false;
        return;
    }

    // 1フレーム分まとめて位置を押し出す
    Vector3 pos = GetOwner()->GetPosition();
    pos += m_totalPushThisFrame;
    GetOwner()->SetPosition(pos);

    // 次フレームに持ち越さない
    m_totalPushThisFrame = Vector3::Zero;
    m_hasPushThisFrame = false;
}
