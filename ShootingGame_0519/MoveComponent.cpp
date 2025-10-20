#include "MoveComponent.h"
#include "GameObject.h"
#include "Application.h"
#include <iostream>
//#include <DirectXMath.h>

using namespace DirectX::SimpleMath;

constexpr float XM_PI = 3.14159265f;
constexpr float XM_2PI = 6.28318530f;

void MoveComponent::Initialize()
{


}

void MoveComponent::Update(float dt)
{
    if (!m_camera)
    {
        return;
    }

    //目標点（ワールド）をカメラから取得
    Vector3 aimTarget = m_camera->GetAimPoint();

    //現在の位置と向き
    Vector3 pos = GetOwner()->GetPosition();
    Vector3 rot = GetOwner()->GetRotation(); // rot.x=pitch, rot.y=yaw, rot.z=roll の想定
    float currentYaw = rot.y;
    float currentPitch = m_currentPitch;

    //目標方向（水平のみ）
    Vector3 toTarget = aimTarget - pos;
    float distSq = toTarget.LengthSquared();
    if (toTarget.LengthSquared() < 1e-6f)
    {
        Vector3 forward = Vector3(std::sin(currentYaw) * std::cos(currentPitch),
            std::sin(currentPitch),
            std::cos(currentYaw) * std::cos(currentPitch));
        forward.Normalize();
        pos += forward * m_speed * dt;
        GetOwner()->SetPosition(pos);
        return;
    }

    Vector3 toDir = toTarget;
    toTarget.Normalize();

    //目標 yaw を求める（forward 定義と整合）
    float targetYaw = std::atan2(toTarget.x, toTarget.z);

    //pitch = asin(y) でも良いが、安定のため水平距離を使う：
    float horiz = std::sqrt(toDir.x * toDir.x + toDir.z * toDir.z);
    float targetPitch = std::atan2(toDir.y, horiz); // 正の値は上向き

    //yaw 差のラップ（-pi..pi に収める）
    float deltaYaw = targetYaw - currentYaw;
    while (deltaYaw > XM_PI) deltaYaw -= XM_2PI;
    while (deltaYaw < -XM_PI) deltaYaw += XM_2PI;

    // 7) pitch 差（-pi..pi で良い）
    float deltaPitch = targetPitch - currentPitch;
    while (deltaPitch > XM_PI) deltaPitch -= XM_2PI;
    while (deltaPitch < -XM_PI) deltaPitch += XM_2PI;

    // 8) 角速度上限で回す（スムーズ回転）
    float maxYawTurn = m_rotateSpeed * dt;
    float appliedYaw = std::clamp(deltaYaw, -maxYawTurn, maxYawTurn);
    currentYaw += appliedYaw;

    float maxPitchTurn = m_pitchSpeed * dt;
    float appliedPitch = std::clamp(deltaPitch, -maxPitchTurn, maxPitchTurn);
    currentPitch += appliedPitch;

    // 9) 前方ベクトルを yaw/pitch から作る
    Vector3 currentForward = Vector3(
        std::sin(currentYaw) * std::cos(currentPitch),  // x
        std::sin(currentPitch),                         // y
        std::cos(currentYaw) * std::cos(currentPitch)   // z
    );
    if (currentForward.LengthSquared() > 1e-6f)
        currentForward.Normalize();
    else
        currentForward = Vector3(0, 0, 1);

    // 10) ロール（バンク）演出 — 横方向の差分を使う
    // 横方向の成分（cross の y 成分が横の符号を示す）
    Vector3 toDirHoriz = toDir;                   // toDir は単位長
    // we only need the horizontal part for bank sign, but cross currentForward x toDir gives lateral
    Vector3 cross = currentForward.Cross(toDir);
    float lateral = cross.y; // 正なら右（符号はプロジェクトの慣習で調整）
    constexpr float DegToRad = 3.14159265358979323846f / 180.0f;
    float maxBankAngle = 20.0f * DegToRad;
    float desiredRoll = -lateral * maxBankAngle;
    float bankSmooth = 6.0f;
    m_currentRoll = m_currentRoll + (desiredRoll - m_currentRoll) * std::min(1.0f, bankSmooth * dt);

    // 11) 回転と位置を反映（rot.x = pitch, rot.y = yaw, rot.z = roll）
    rot.x = -currentPitch;
    rot.y = currentYaw;
    rot.z = m_currentRoll;
    GetOwner()->SetRotation(rot);

    // 12) 前進（現在の forward 方向に進むことで上下斜め移動する）
    pos += currentForward * m_speed * dt;
    GetOwner()->SetPosition(pos);

    // 13) 内部ピッチを保存
    m_currentPitch = currentPitch;

    /*std::cout << "ターゲットピッチ：" << targetPitch << std::endl;
    std::cout << "カレントピッチ　：" << currentPitch << std::endl;
    std::cout << "ローテーション　：" << rot.x << std::endl;*/
    std::cout << std::endl;
}
