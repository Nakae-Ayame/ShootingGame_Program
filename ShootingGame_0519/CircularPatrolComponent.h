#pragma once
#include "Component.h"
#include <numbers>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class CirculPatrolComponent : public Component
{
	CirculPatrolComponent() = default;
	~CirculPatrolComponent() override = default;

	void Initialize() override;
	void Update(float dt) override;

    // 設定
    void SetCenter(const Vector3& c) { m_center = c; }
    void SetRadius(float r)
    {
        if (r < 0.0f) { m_radius = 0.0f; }
        else { m_radius = r; }
    }
    void SetAngularSpeed(float radPerSec) { m_angularSpeed = radPerSec; } // ラジアン/秒
    void SetClockwise(bool cw) { m_clockwise = cw; }
    void SetRotateToTangent(bool v) { m_rotateToTangent = v; }
    void SetStartAngle(float rad) { m_angle = rad; }
    //void SetEnabled(bool v) { m_enabled = v; }

    //bool IsEnabled() const { return m_enabled; }

private:
    Vector3 m_Center = Vector3::Zero;      //回る時の中心点(ワールド座標)
    float m_Radius = 5.0f;                 //回る時の半径
    float m_Angle = 0.0f;                  //現在回っている時の角度
    float m_AngularSpeed = 3.14 * 0.5f;    //一秒間で何度回転するか
    bool m_Clockwise = true;               //回る方向 true なら 時計回り、 false なら反時計回り
    bool m_RotateToTangent = true;         //
    //bool m_enabled = true;
};