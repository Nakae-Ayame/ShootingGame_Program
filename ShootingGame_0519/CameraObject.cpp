#include "CameraObject.h"

void CameraObject::Initialize()
{
    GameObject::Initialize();
}

void CameraObject::SetCameraComponent(const std::shared_ptr<CameraComponentBase>& cameraComponent)
{
    if (!cameraComponent) { return; }

    m_cameraComponent = cameraComponent;
    AddComponent(m_cameraComponent);
}