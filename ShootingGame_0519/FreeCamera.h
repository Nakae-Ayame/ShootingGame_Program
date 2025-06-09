#pragma once
#include "BaseCamera.h"
#include "Input.h"

//      
//FreeCamera (6/4実装開始)
//
class FreeCamera :public BaseCamera
{
public:

    FreeCamera(float aspect)
    {
        m_AspectRatio = aspect;
        m_Position = { 0,0,-5 };
        m_Yaw = 0;
        m_Pitch = 0;
        UpdateViewVectors();
    }

    void Update(uint64_t delta) override;
    //-----------------------------ゲッター関数-----------------------------
    DirectX::XMMATRIX GetViewMatrix() const override;
    DirectX::XMMATRIX GetProjectionMatrix() const override;
    DirectX::XMFLOAT3 GetPosition() const override { return m_Position; }
    DirectX::XMFLOAT3 GetForward()  const override { return m_Forward; }
    //-----------------------------セッター関数-----------------------------
    void SetPosition(const DirectX::XMFLOAT3& pos) { m_Position = pos; }

private: 
    //ビューベクトルを更新する関数
    void UpdateViewVectors();

    XMFLOAT3 m_Position;  //カメラの位置
    float m_Yaw;		  //カメラの水平方向(左右)の回転角度
    float m_Pitch;		  //カメラの垂直方向(上下)の回転角度

    //-----------------------------方向ベクトル-----------------------------
    DirectX::XMFLOAT3 m_Forward = { 0, 0, 1 };  //カメラの前方向ベクトル
    DirectX::XMFLOAT3 m_Right   = { 1, 0, 0 };  //カメラの右方向ベクトル
    DirectX::XMFLOAT3 m_Up      = { 0, 1, 0 };  //カメラの上方向ベクトル

    //プロジェクション
    float m_AspectRatio;
    float m_FOV = DirectX::XMConvertToRadians(60.0f);
    float m_Near = 0.1f;
    float m_Far = 1000.0f;

   
};
