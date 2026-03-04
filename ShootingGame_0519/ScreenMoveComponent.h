#pragma once
#include "Component.h"
#include "ForwardMoveComponent.h"
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

class ForwardMoveComponent;

class ScreenMoveComponent : public Component
{
public:
	void Initialize() override;
	void Update(float dt) override;
	void Uninit() override;

	void SetRailProvider(ForwardMoveComponent* forwardComp) { m_forwardComp = forwardComp; };
	void SetMoveRange(float range) { m_range = range; };
	void SetFollowPower(float power) { m_followPower = power; };
	void SetDamping(float damping) {m_damping = damping;};

private:
	float m_range = 8.0f;		 //画面で動ける範囲
	float m_damping = 0.88f;	 //動く時の慣性
	float m_followPower= 40.0f;  //マウスへの追従の強さ

	Vector2 m_targetOffset;   //レティクル目標
	Vector2 m_currentOffset;  //現在位置
	Vector2 m_velocity;       //慣性用

	ForwardMoveComponent* m_forwardComp;
};
