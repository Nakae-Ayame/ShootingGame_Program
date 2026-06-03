#pragma once
#include <Windows.h>

class Input
{
public:
    static void Update();                //毎フレーム更新

    // キーボード
    static bool IsKeyDown(unsigned char key);
    static bool IsKeyPressed(unsigned char key);//前フレームとの差分を見る

    //---------------------------マウス系処理---------------------------
    //マウスの移動量を計算する関数
    static POINT GetMouseDelta();
    //現在の位置を取得する関数
    static POINT GetMousePosition();

    //マウスの右クリックを押しているか
    static bool IsMouseRightDown();

    static bool IsMouseRightPressed();

    //マウスの左クリックを押しているか
    static bool IsMouseLeftDown();

    static bool IsMouseLeftPressed();

    static void Reset();

    //------------------------コントローラー系処理-----------------------
    //// XBox コントローラー
    //static bool  IsGamepadConnected();
    //static float GetLeftStickX(); // -1.0 ～ 1.0
    //static float GetLeftStickY();
    //static bool  IsGamepadButtonPressed(WORD button);

private:
    static BYTE m_currentKeys[256];
    static BYTE m_previousKeys[256];

    static POINT m_currentMousePos;   //現在のフレームのマウスの座標
    static POINT m_previousMousePos;  //前のフレームのマウスの座標

    static BYTE m_currentMouseButtons[3];  //現在のフレームのマウスのクリック状況
    static BYTE m_previousMouseButtons[3]; //前のフレームのマウスのクリック状況

    static float m_mouseSensitivity;//マウス感度
};
