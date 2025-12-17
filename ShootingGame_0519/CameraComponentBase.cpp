#include "CameraComponentBase.h"
#include "Application.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//ウィンドウサイズが変わった際に投影行列を作り直す関数
//--------------（必要な場合のみ）------------------
void CameraComponentBase::UpdateProjectionIfNeeded()
{
    int w = Application::GetWidth();
    int h = Application::GetHeight();
    if (w <= 1 || h <= 1) { return; }

    if (w == m_PrevScreenW && h == m_PrevScreenH && 
        m_ProjectionMatrix.Determinant() != 0.0f){ return; }

    m_PrevScreenW = w;
    m_PrevScreenH = h;

    float aspect = static_cast<float>(w) / static_cast<float>(h);
    m_ProjectionMatrix = Matrix::CreatePerspectiveFieldOfView(m_Fov, aspect, m_NearZ, m_FarZ);
}
