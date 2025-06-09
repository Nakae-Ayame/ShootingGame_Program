#include "FreeCamera.h"
#include <algorithm>  // std::clamp 

void FreeCamera::Update(uint64_t delta)
{
    Input::Update();

    // マウスで視線回転
    POINT d = Input::GetMouseDelta();
    m_Yaw += d.x * 0.002f;
    m_Pitch += d.y * 0.002f;
    m_Pitch = std::clamp(m_Pitch, -XM_PIDIV2 + 0.1f, +XM_PIDIV2 - 0.1f);

    // キーで移動
    XMVECTOR pos = XMLoadFloat3(&m_Position);
    XMVECTOR fwd = XMLoadFloat3(&m_Forward);
    XMVECTOR rt = XMLoadFloat3(&m_Right);

    float speed = 5.0f * (delta / 1000.0f);
    if (Input::IsKeyDown('W')) pos += fwd * speed;
    if (Input::IsKeyDown('S')) pos -= fwd * speed;
    if (Input::IsKeyDown('A')) pos -= rt * speed;
    if (Input::IsKeyDown('D')) pos += rt * speed;
    XMStoreFloat3(&m_Position, pos);

    UpdateViewVectors();

    // デバッグ出力
    char buf[128];
    sprintf_s(buf, "Cam Pos: %.2f, %.2f, %.2f  YawPitch: %.2f, %.2f\n",
        m_Position.x, m_Position.y, m_Position.z,
        XMConvertToDegrees(m_Yaw), XMConvertToDegrees(m_Pitch));
    OutputDebugStringA(buf);
}

void FreeCamera::UpdateViewVectors()
{
    // 前方向
    XMVECTOR fwd = XMVectorSet(
        cosf(m_Pitch) * sinf(m_Yaw),
        sinf(m_Pitch),
        cosf(m_Pitch) * cosf(m_Yaw),
        0);
    fwd = XMVector3Normalize(fwd);
    XMStoreFloat3(&m_Forward, fwd);

    // 右方向
    XMVECTOR upRef = XMVectorSet(0, 1, 0, 0);
    XMVECTOR rt = XMVector3Normalize(XMVector3Cross(upRef, fwd));
    XMStoreFloat3(&m_Right, rt);

    // 真の上方向
    XMVECTOR upv = XMVector3Cross(fwd, rt);
    XMStoreFloat3(&m_Up, upv);
}

XMMATRIX FreeCamera::GetViewMatrix() const
{
    return XMMatrixLookToLH(
        XMLoadFloat3(&m_Position),
        XMLoadFloat3(&m_Forward),
        XMLoadFloat3(&m_Up));
}

XMMATRIX FreeCamera::GetProjectionMatrix() const
{
    return XMMatrixPerspectiveFovLH(m_FOV, m_AspectRatio, m_Near, m_Far);
}


