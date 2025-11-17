// MoveComponent.h
#pragma once
#include "Component.h"
#include "Input.h"
#include "ICameraViewProvider.h"
#include <SimpleMath.h>
#include <fstream>
#include <functional>

class PlayAreaComponent;

class MoveComponent : public Component
{
public:
    MoveComponent() = default;
    ~MoveComponent() override = default;

    void Initialize() override;
    void Update(float dt) override;
    void Uninit() override;

    //移動速度のセット関数
    void SetSpeed(float speed) { m_baseSpeed = speed; }

    //移動の前後左右を決めるために使うカメラのセット関数
    void SetCameraView(ICameraViewProvider* camera) { m_camera = camera; }

    void SetPlayArea(PlayAreaComponent* playArea) { m_playArea = playArea; }

    // Obstacle tester callback:
    // return true if hit; outHitNormal and outHitDist filled; length is ray length
    // signature: bool(start, dir, length, outHitNormal, outHitDist)
    void SetObstacleTester(std::function<bool(const DirectX::SimpleMath::Vector3&,
        const DirectX::SimpleMath::Vector3&,
        float,
        DirectX::SimpleMath::Vector3&,
        float&)> tester)
    {
        m_obstacleTester = std::move(tester);
    }

private:
    int m_frameCounter = 0;

    //スピード/秒
    float m_speed = 45.0f;

    //カメラの向きを取得する用のポインタ
    ICameraViewProvider* m_camera = nullptr;

    float m_rotateSpeed = 10.0f;  //（未使用のまま残すが、角速度上限として保有）
    float m_rotSmoothK = 8.0f;    // 回転の指数遅延係数（大きいほど素早く追従）

    float m_currentRoll = 0.0f;   //現在のロール

    float m_pitchSpeed = 3.5f;    //ピッチ回転速度（代替の制限用に残す）

    float m_currentPitch = 0.0f;  //現在のピッチ

    //-------------------------ブースト関連変数----------------------------
    float m_boostMultiplier = 2.5f;   // 何倍速くなるか

    float m_boostSeconds = 1.0f;   // ブーストが持続する秒数

    float m_boostRecover = 0.8f;      //通常速度に戻るまでの秒数

    float m_boostCooldown = 0.5f;     // 次ブーストできるまでのクールダウン

    bool  m_isBoosting = false;       //ブースト中かどうか 

    float m_boostTimer = 0.0f;        // ブースト経過

    float m_recoverTimer = 0.0f;      // 回復の進捗

    float m_cooldownTimer = 0.0f;

    float m_baseSpeed = 25.0f;        // 元のスピード格納用

    int   m_boostKey = VK_SHIFT;      //トリガーキー(変更可)

    bool  m_prevBoostKeyDown = false; //エッジ検出用

    bool  m_lockDuringBoost = false;   // ブースト中は向きをロックするフラグ

    float m_lockedYaw = 0.0f;          // ブースト開始時の yaw を保持

    float m_lockedPitch = 0.0f;        // ブースト開始時の pitch を保持

    PlayAreaComponent* m_playArea = nullptr;

    //-------------------------障害物回避用変数-------------------------
    float m_predictTimeBase = 0.12f;
    float m_predictTimeFactor = 0.18f; // predictTime = base + (speed/baseSpeed) * factor
    float m_avoidRange = 5.0f;
    float m_avoidWeight = 1.6f;
    float m_forwardWeight = 0.6f;
    float m_inputWeight = 0.9f;
    int   m_avoidSamples = 5; // number of yaw samples for obstacle checking

    // obstacle tester callback (optional)
    std::function<bool(const DirectX::SimpleMath::Vector3& /*start*/,
        const DirectX::SimpleMath::Vector3& /*dir*/,
        float /*length*/,
        DirectX::SimpleMath::Vector3& /*outNormal*/,
        float& /*outDist*/)> m_obstacleTester;
};
