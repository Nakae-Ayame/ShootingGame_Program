// MoveComponent.cpp
#include "MoveComponent.h"
#include "PlayAreaComponent.h"
#include "GameObject.h"
#include "Application.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace DirectX::SimpleMath;

constexpr float XM_PI = 3.14159265f;
constexpr float XM_2PI = 6.28318530f;

static float LerpExpFactor(float k, float dt)
{
    // return interpolation alpha = 1 - exp(-k*dt)
    return 1.0f - std::exp(-k * dt);
}

// normalize angle difference to [-PI, PI]
static float NormalizeAngleDelta(float a)
{
    while (a > XM_PI) a -= XM_2PI;
    while (a < -XM_PI) a += XM_2PI;
    return a;
}

void MoveComponent::Initialize()
{

}

void MoveComponent::Uninit()
{

}

void MoveComponent::Update(float dt)
{
    // カメラがなければ処理しない
    if (!m_camera)
    {
        return;
    }

    // 現時点でのPlayerの位置・回転取得
    Vector3 pos = GetOwner()->GetPosition();
    Vector3 rot = GetOwner()->GetRotation();

    // 現時点でのyaw/pitch取得
    float currentYaw = rot.y;
    float currentPitch = m_currentPitch;

    // ブースト用の入力取得
    bool keyDown = Input::IsKeyPressed(m_boostKey);

    // -----------------ブースト開始判定---------------------
    bool startBoost = false;

    // ブーストキーが押されていて、前フレームでは押されておらず、
    // クールダウン中でなく、現在ブースト中でない場合
    if (keyDown && !m_prevBoostKeyDown && m_cooldownTimer <= 0.0f && !m_isBoosting)
    {
        // ブースト開始
        startBoost = true;
    }

    // 前フレームのキー状態を保存
    m_prevBoostKeyDown = keyDown;

    // -----------------ブースト開始処理---------------------
    if (startBoost)
    {
        // ブースト中にする
        m_isBoosting = true;

        // NOTE:
        // ここで "完全に向きをロックする" 処理を削除しました。
        // （以前は m_lockDuringBoost = true; m_lockedYaw = currentYaw; ... として
        //  ブースト中は向きを完全固定していました）
        // 今回は入力を受け付けつつ、ブースト時の回転速度を制限して安定させます。

        // ブーストのタイマーを初期化
        m_boostTimer = 0.0f;

        // ブースト回復のタイマーを初期化
        m_recoverTimer = -1.0f;

        m_cooldownTimer = m_boostCooldown;

        if (m_camera)
        {
            m_camera->SetBoostState(true); // カメラにブースト開始を通知
        }
    }

    // -----------------ブースト時間経過判定---------------------
    if (m_isBoosting)
    {
        m_boostTimer += dt;
        if (m_boostTimer >= m_boostSeconds)
        {
            m_isBoosting = false;
            m_recoverTimer = 0.0f; // 回復フェーズ開始

            if (m_camera)
            {
                m_camera->SetBoostState(false); // カメラにブースト終了を通知
            }
        }
    }

    // -----------------クールダウン減算---------------------
    if (m_cooldownTimer > 0.0f)
    {
        m_cooldownTimer -= dt;
        if (m_cooldownTimer < 0.0f)
        {
            m_cooldownTimer = 0.0f;
        }
    }

    // -----------------速度決定---------------------
    float currentSpeed = m_baseSpeed;
    if (m_isBoosting)
    {
        // ブースト中は速さに倍率を付ける
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

    // -----------------通常移動処理---------------------

    // 注視点取得
    Vector3 aimTarget = m_camera->GetAimPoint();

    // 目標点までのベクトル計算
    Vector3 toTarget = aimTarget - pos;

    // 目標点が近い場合の処理
    if (toTarget.LengthSquared() < 1e-6f)
    {
        // 目標点が近い場合は現在向きのまま前進
        Vector3 forward = Vector3(std::sin(currentYaw) * std::cos(currentPitch),
            std::sin(currentPitch),
            std::cos(currentYaw) * std::cos(currentPitch));
        forward.Normalize();
        pos += forward * currentSpeed * dt;
        GetOwner()->SetPosition(pos);
        return;
    }

    // 現在の前方ベクトル計算
    Vector3 currentForward = Vector3(
        std::sin(currentYaw) * std::cos(currentPitch),
        std::sin(currentPitch),
        std::cos(currentYaw) * std::cos(currentPitch)
    );

    if (currentForward.LengthSquared() > 1e-6f)
    {
        currentForward.Normalize();
    }
    else
    {
        currentForward = Vector3(0, 0, 1);
    }

    // 未来位置予測
    float speedRatio = (m_baseSpeed > 0.0001f) ? (currentSpeed / m_baseSpeed) : 1.0f;
    float predictTime = m_predictTimeBase + speedRatio * m_predictTimeFactor;
    float predictDist = currentSpeed * predictTime;
    Vector3 futurePos = pos + currentForward * predictDist;

    // compute avoidDir by sampling several yaw offsets ahead
    Vector3 avoidDir(0, 0, 0);
    if (m_obstacleTester && m_avoidSamples > 0)
    {
        // sample angles in range [-30deg, +30deg]
        const float maxAngleRad = 30.0f * XM_PI / 180.0f;
        for (int i = 0; i < m_avoidSamples; ++i)
        {
            float t;
            if (m_avoidSamples == 1)
            {
                t = 0.0f;
            }
            else
            {
                t = float(i) / float(m_avoidSamples - 1);
            }

            float sampleAngle = -maxAngleRad + t * (2.0f * maxAngleRad);
            float sampleYaw = currentYaw + sampleAngle;

            // build a sample direction using sampleYaw and currentPitch (rotate horizontally)
            Vector3 sampleDir = Vector3(
                std::sin(sampleYaw) * std::cos(currentPitch),
                std::sin(currentPitch),
                std::cos(sampleYaw) * std::cos(currentPitch)
            );
            sampleDir.Normalize();

            float rayLen = m_avoidRange + predictDist * 0.5f; // look distance scaled by speed/predict
            Vector3 hitNormal;
            float hitDist = 0.0f;
            bool hit = m_obstacleTester(futurePos, sampleDir, rayLen, hitNormal, hitDist);
            if (hit)
            {
                // weight stronger when closer
                float strength = std::clamp((rayLen - hitDist) / rayLen, 0.0f, 1.0f);
                // push away from obstacle along hit normal if provided, else opposite of sampleDir
                Vector3 away;
                if (hitNormal.LengthSquared() > 0.0001f)
                {
                    away = hitNormal;
                }
                else
                {
                    away = -sampleDir;
                }

                // ensure upward component doesn't dominate
                away.y = std::clamp(away.y, -0.5f, 0.5f);
                away.Normalize();
                avoidDir += away * (strength * strength); // squared to smooth onset
            }
        }

        if (avoidDir.LengthSquared() > 1e-6f)
        {
            avoidDir.Normalize();
        }
    }

    // base desired direction is toward aim target
    Vector3 toDir = toTarget;
    toDir.Normalize();

    // combine forward, input(toDir), avoid with weights
    Vector3 desired = toDir * m_inputWeight + currentForward * m_forwardWeight + avoidDir * m_avoidWeight;
    if (desired.LengthSquared() > 1e-6f)
    {
        desired.Normalize();
    }
    else
    {
        desired = currentForward;
    }

    // compute target yaw/pitch based on desired
    float targetYaw = std::atan2(desired.x, desired.z);
    float horiz = std::sqrt(desired.x * desired.x + desired.z * desired.z);
    float targetPitch = std::atan2(desired.y, horiz);

    // NOTE:
    // 以前は「ブースト中は向きを完全固定する」処理がここにありました。
    // 今回はブースト中でも入力/回避で方向を変えられるように、その強制をやめています。

    // compute shortest deltas
    float deltaYaw = NormalizeAngleDelta(targetYaw - currentYaw);
    float deltaPitch = NormalizeAngleDelta(targetPitch - currentPitch);

    // --- SMOOTH ROTATION: use exponential smoothing (frame-rate independent) ---
    float yawAlpha = LerpExpFactor(m_rotSmoothK, dt);   // 0..1
    float pitchAlpha = LerpExpFactor(m_rotSmoothK, dt);

    // optionally clamp maximum angular change per frame (safety cap)
    float maxYawChange = m_rotateSpeed * dt;   // retains old config if desired
    float maxPitchChange = m_pitchSpeed * dt;

    // If boosting, reduce max turn amount to avoid wild instant turns.
    if (m_isBoosting)
    {
        float boostTurnFactor = 0.5f; // 0..1, smaller = less turning while boosting; tune as needed
        maxYawChange = maxYawChange * boostTurnFactor;
        maxPitchChange = maxPitchChange * boostTurnFactor;
    }

    // apply smoothing but also cap by max change
    float appliedYaw = deltaYaw * yawAlpha;
    if (appliedYaw > maxYawChange)
    {
        appliedYaw = maxYawChange;
    }
    else if (appliedYaw < -maxYawChange)
    {
        appliedYaw = -maxYawChange;
    }
    currentYaw = currentYaw + appliedYaw;

    float appliedPitch = deltaPitch * pitchAlpha;
    if (appliedPitch > maxPitchChange)
    {
        appliedPitch = maxPitchChange;
    }
    else if (appliedPitch < -maxPitchChange)
    {
        appliedPitch = -maxPitchChange;
    }
    currentPitch = currentPitch + appliedPitch;

    // recompute forward from smoothed angles
    Vector3 newForward = Vector3(
        std::sin(currentYaw) * std::cos(currentPitch),
        std::sin(currentPitch),
        std::cos(currentYaw) * std::cos(currentPitch)
    );

    if (newForward.LengthSquared() > 1e-6f)
    {
        newForward.Normalize();
    }
    else
    {
        newForward = Vector3(0, 0, 1);
    }

    // ロール回転（銀行）は従来アルゴリズムを利用（短期的に滑らか化）
    Vector3 cross = newForward.Cross(desired); // use desired for lateral sense
    float lateral = cross.y;
    constexpr float DegToRad = 3.14159265358979323846f / 180.0f;
    float maxBankAngle = 20.0f * DegToRad;
    float desiredRoll = -lateral * maxBankAngle;
    float bankSmooth = 6.0f;
    float bankLerp = bankSmooth * dt;
    if (bankLerp > 1.0f)
    {
        bankLerp = 1.0f;
    }
    m_currentRoll = m_currentRoll + (desiredRoll - m_currentRoll) * bankLerp;

    // set rotation (note: original code uses rot.x = -pitch)
    rot.x = -currentPitch;
    rot.y = currentYaw;
    rot.z = m_currentRoll;
    GetOwner()->SetRotation(rot);

    // 前進処理（smoothed forward）
    pos += newForward * currentSpeed * dt;

    // PlayAreaがあるなら当てはめ（オプション）
    if (m_playArea)
    {
        pos = m_playArea->ResolvePosition(GetOwner()->GetPosition(), pos);
    }
    GetOwner()->SetPosition(pos);

    // 更新保存
    m_currentPitch = currentPitch;
}
