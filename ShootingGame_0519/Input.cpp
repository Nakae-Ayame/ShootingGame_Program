#include "Input.h"

BYTE Input::m_CurrentKeys[256];
BYTE Input::m_PreviousKeys[256];

POINT Input::m_CurrentMousePos;   //現在のフレームのマウスの座標
POINT Input::m_PreviousMousePos;  //前のフレームのマウスの座標

float Input::m_MouseSensitivity = 0.005f;   //マウス感度

void Input::Update()
{
    //前のフレームのキー入力を写して保存
    memcpy(m_PreviousKeys, m_CurrentKeys, sizeof(m_CurrentKeys));
    //現在のキー状態を取得
    if (!GetKeyboardState(m_CurrentKeys))
    {
        // エラー処理、ログ出力など
        OutputDebugStringA("キーが取れていません！！\n");
    }

    //前のフレームのマウスの座標を写して保存
    m_PreviousMousePos = m_CurrentMousePos;
    //現在のフレームのマウスの座標を取得
    if (!GetCursorPos(&m_CurrentMousePos))
    {
        OutputDebugStringA("GetCursorPos() Failed\n");
    }
}

bool Input::IsKeyDown(unsigned char key)
{
    return (m_CurrentKeys[key] & 0x80) != 0;
}

bool Input::IsKeyPressed(unsigned char key)
{
    return (m_CurrentKeys[key] & 0x80) != 0 && (m_PreviousKeys[key] & 0x80) == 0;
}

POINT Input::GetMouseDelta()
{
    POINT delta;
    //カメラのX座標の移動量を差で計算する
    delta.x = m_CurrentMousePos.x - m_PreviousMousePos.x;
    //カメラのY座標の移動量を差で計算する
    delta.y = m_CurrentMousePos.y - m_PreviousMousePos.y;
    return delta;
}

POINT Input::GetMousePosition()
{
    return m_CurrentMousePos;
}