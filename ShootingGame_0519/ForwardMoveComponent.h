#pragma once
#include "Component.h"
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class ForwardMoveComponent : public Component
{
public:
    ForwardMoveComponent() = default;
    ~ForwardMoveComponent() override = default;

    void Initialize() override;
    void Update(float dt) override;
    void Uninit() override;

    //------------Set関数------------
	void SetWaypoints(const std::vector<Vector3>& waypoints) { m_waypoints = waypoints; }
	void SetSpeed(float speed) { m_speed = speed; }
    void SetRailForward(const Vector3& forward) { m_railForward = forward; }
    //------------Get関数------------
    Vector3 GetRailCenterPos() const { return m_railCenterPos; }
    Vector3 GetRailForward() const { return m_railForward; }
    Vector3 GetRailRight() const { return m_railRight; }
    Vector3 GetRailUp() const { return m_railUp; }

private:
    std::vector<Vector3> m_waypoints;           //道筋のポイント
    Vector3 m_railCenterPos = Vector3::Zero;    //レール中心（基準位置）
    Vector3 m_railForward = {0.0f,0.0f,1.0f};   //進む単位のベクトル
    Vector3 m_railUp = { 0.0f, 1.0f, 0.0f };    //上方向（固定）
    Vector3 m_railRight = { 1.0f, 0.0f, 0.0f }; //右方向（forward×up）

	float m_speed = 30.0f;  //移動速度
};
