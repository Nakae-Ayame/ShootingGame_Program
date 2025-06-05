#pragma once
#include <DirectXMath.h>
using namespace DirectX;
//--------------------------------------------------------------------------------
//BaseCamera関数
//主なカメラの元になるクラスを作りたい
//--------------------------------------------------------------------------------
class BaseCamera
{
public:
	BaseCamera() = default;
	virtual ~BaseCamera() = default;

	virtual void Update(uint64_t delta) = 0;

	virtual XMMATRIX GetViewMatrix() const = 0;

	virtual XMMATRIX GetProjectionMatrix() const = 0;

	virtual XMFLOAT3 GetPosition() const = 0;

	virtual XMFLOAT3 GetForward() const = 0;
};

/*Cameraクラスで最低限必要な物

位置：XMFLOAT3 m_Position

方向：float m_Yaw, float m_Pitch

ビュー行列：XMMATRIX GetViewMatrix()

投影行列：XMMATRIX GetProjectionMatrix()
ビュー・投影行列の更新
Renderer::Draw()内などで
camera.GetViewMatrix()
camera.GetProjectionMatrix()
定数バッファに行列を転送

private:
	XMFLOAT3 m_Position;  //カメラの位置
	float m_Yaw;		  //カメラの水平方向(左右)の回転角度
	float m_Pitch;		  //カメラの垂直方向(上下)の回転角度
*/
