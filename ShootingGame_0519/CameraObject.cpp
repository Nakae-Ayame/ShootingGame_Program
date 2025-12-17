#include "CameraObject.h"

void CameraObject::Initialize()
{
    GameObject::Initialize();
}

void CameraObject::SetCameraComponent(const std::shared_ptr<CameraComponentBase>& cameraComponent)
{
    if (!cameraComponent) { return; }

    m_CameraComponent = cameraComponent;
    AddComponent(m_CameraComponent);
}