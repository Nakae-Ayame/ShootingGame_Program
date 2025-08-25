#pragma once
#include "Component.h"
#include "Input.h"
#include "ICameraViewProvider.h"
#include <SimpleMath.h>

class MoveComponent : public Component
{
public:
    MoveComponent() = default;
    ~MoveComponent() override = default;

    void Initialize() override;
    void Update() override;

    //移動速度のセット関数
    void SetSpeed(float speed) { m_speed = speed; }

    //移動の前後左右を決めるために使うカメラのセット関数
    void SetCameraView(ICameraViewProvider* camera) { m_camera = camera; }

private: 
    //ユニット/秒
    float m_speed = 1.0f;

    //カメラの向きを取得する用のポインタ
    ICameraViewProvider* m_camera = nullptr;
    
    //Y軸回転速度（ラジアン/秒）
    float m_rotateSpeed = 4.0f; 
};
