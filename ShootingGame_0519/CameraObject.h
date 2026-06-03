#pragma once
#include "GameObject.h"
#include <DirectXMath.h>
#include <SimpleMath.h>
#include "FollowCameraComponent.h"

using namespace DirectX;

class CameraObject : public GameObject
{
public:
	CameraObject() = default;
	~CameraObject() override = default;

	void Initialize() override;

	//---------------------Setä÷êîä÷òA------------------------
	void SetCameraComponent(const std::shared_ptr<CameraComponentBase>& cameraComponent);
	template<class T>
	std::shared_ptr<T> AddCameraComponent()
	{
		auto comp = std::make_shared<T>();
		AddComponent(comp);
		m_cameraComponent = comp;
		return comp;
	}

	//---------------------Getä÷êîä÷òA-------------------------
	std::shared_ptr<CameraComponentBase> GetCameraComponent() const { return m_cameraComponent; }
	std::shared_ptr<FollowCameraComponent> GetFollowCameraComponent() const { return std::dynamic_pointer_cast<FollowCameraComponent>(m_cameraComponent);};

private:
	std::shared_ptr<CameraComponentBase> m_cameraComponent;
};
