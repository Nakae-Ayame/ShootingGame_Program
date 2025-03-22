#include "Renderer.h"
#include "Camera.h"
#include "Application.h"
#include "input.h"
#include "Game.h"
//#include ""

using namespace DirectX::SimpleMath;
using namespace std;

//=======================================
//初期化処理
//=======================================
void Camera::Init()
{
	m_Position = Vector3(0.0f, 20.0f, 0.0f);
	m_Target = Vector3(0.0f, 0.0f, 0.0f);
	m_CameraDirection = 3.14f;
}


//=======================================
//更新処理
//=======================================
void Camera::Update()
{
	m_Position.y += 0.2f; // 回転速度
	//m_Position.z += 0.7f; // 回転速度
	//SetPosition(m_Position.x, m_Position.y, m_Position.z);
}

//=======================================
//描画処理
//=======================================
void Camera::Draw()
{
	
}

//=======================================
//終了処理
//=======================================
void Camera::Uninit()
{

}

void Camera::SetCamera(int mode)
{
	if (mode == 0) 
	{
		// ビュー変換後列作成
		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
		m_ViewMatrix = DirectX::XMMatrixLookAtLH(m_Position, m_Target, up); // 左手系にした
		// DIRECTXTKのメソッドは右手系
		// 右手系にすると３角形頂点が反時計回りになるので描画されなくなるので注意
		// このコードは確認テストのために残す
		// m_ViewMatrix = m_ViewMatrix.CreateLookAt(m_Position, m_Target, up);					

		Renderer::SetViewMatrix(&m_ViewMatrix);

		//プロジェクション行列の生成
		constexpr float fieldOfView = DirectX::XMConvertToRadians(45.0f);    // 視野角
		float aspectRatio = static_cast<float>(Application::GetWidth()) / static_cast<float>(Application::GetHeight());	// アスペクト比	

		float nearPlane = 1.0f;       // ニアクリップ
		float farPlane = 1000.0f;      // ファークリップ
		Matrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, nearPlane, farPlane); 
		Renderer::SetProjectionMatrix(&projectionMatrix);
	}
	else if (mode == 1)
	{
		//ビュー変換行列制作
		Vector3 pos = { 0.0f, 20.0f, -130.0f };
		Vector3 tgt = { 0.0f, 0.0f, 0.0f };
		Vector3 up = { 0.0f, 1.0f, 0.0f };
		m_ViewMatrix = DirectX::XMMatrixLookAtLH(pos, tgt, up);
		Renderer::SetViewMatrix(&m_ViewMatrix);

		//プロジェクト行列の生成
		float nearPlane = 1.0f;
		float farPlane = 1000.0f;
		Matrix projectionMatrix = DirectX::XMMatrixOrthographicLH(static_cast<float>(Application::GetWidth()), static_cast<float>(Application::GetHeight()), nearPlane, farPlane);

		projectionMatrix = DirectX::XMMatrixTranspose(projectionMatrix);
		Renderer::SetProjectionMatrix(&projectionMatrix);
	}
}

