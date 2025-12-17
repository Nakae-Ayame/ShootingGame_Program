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

	//---------------------SetŠÖ”ŠÖ˜A------------------------
	void SetCameraComponent(const std::shared_ptr<CameraComponentBase>& cameraComponent);
	template<class T>
	std::shared_ptr<T> AddCameraComponent()
	{
		auto comp = std::make_shared<T>();
		AddComponent(comp);
		m_CameraComponent = comp;
		return comp;
	}

	//---------------------GetŠÖ”ŠÖ˜A-------------------------
	std::shared_ptr<CameraComponentBase> GetCameraComponent() const { return m_CameraComponent; }
	std::shared_ptr<FollowCameraComponent> GetFollowCameraComponent() const { return std::dynamic_pointer_cast<FollowCameraComponent>(m_CameraComponent);};

private:
	std::shared_ptr<CameraComponentBase> m_CameraComponent;
};
