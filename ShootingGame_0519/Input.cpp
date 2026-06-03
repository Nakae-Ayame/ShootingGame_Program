#include "Input.h"
#include "Application.h"

BYTE Input::m_currentKeys[256];
BYTE Input::m_previousKeys[256];

POINT Input::m_currentMousePos;   //現在のフレームのマウスの座標
POINT Input::m_previousMousePos;  //前のフレームのマウスの座標

BYTE Input::m_currentMouseButtons[3];   //現在のフレームのマウスの座標
BYTE Input::m_previousMouseButtons[3];  //前のフレームのマウスの座標

float Input::m_mouseSensitivity = 0.005f;   //マウス感度

void Input::Update()
{
    // キー
    memcpy(m_previousKeys, m_currentKeys, sizeof(m_currentKeys));
    // マウスボタン
    memcpy(m_previousMouseButtons, m_currentMouseButtons, sizeof(m_currentMouseButtons));
    // マウス座標
    m_previousMousePos = m_currentMousePos;

    //現在のキー状態を取得
    if (!GetKeyboardState(m_currentKeys))
    {
        // エラー処理、ログ出力など
        OutputDebugStringA("キーが取れていません！！\n");
    }


    //現在のフレームのマウスの座標を取得
    if (!GetCursorPos(&m_currentMousePos))
    {
        //OutputDebugStringA("GetCursorPos() Failed\n");
    }
    else
    {
        // ウィンドウ左上を (0,0) とするクライアント座標に変換
        ScreenToClient(Application::GetWindow(), &m_currentMousePos);
    }


    m_currentMouseButtons[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1 : 0;
    m_currentMouseButtons[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 1 : 0;
    m_currentMouseButtons[2] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 1 : 0;
}

bool Input::IsKeyDown(unsigned char key)
{
    return (m_currentKeys[key] & 0x80) != 0;
}

// Input.cpp
bool Input::IsKeyPressed(unsigned char key)
{
    return ((m_currentKeys[key] & 0x80) != 0) && ((m_previousKeys[key] & 0x80) == 0);
}

POINT Input::GetMouseDelta()
{
    POINT delta;
    //カメラのX座標の移動量を差で計算する
    delta.x = m_currentMousePos.x - m_previousMousePos.x;
    //カメラのY座標の移動量を差で計算する
    delta.y = m_currentMousePos.y - m_previousMousePos.y;
    return delta;
}

POINT Input::GetMousePosition()
{
    return m_currentMousePos;
}

bool Input::IsMouseRightDown()
{
    return m_currentMouseButtons[1] != 0;
}

bool Input::IsMouseRightPressed()
{
    return m_currentMouseButtons[1] != 0 && m_previousMouseButtons[1] == 0;
}

bool Input::IsMouseLeftDown()
{
    return m_currentMouseButtons[0] != 0;
}

bool Input::IsMouseLeftPressed()
{
    return m_currentMouseButtons[0] != 0 && m_previousMouseButtons[0] == 0;
}

void Input::Reset()
{
    // 全キー・マウス状態をゼロにリセット
    ZeroMemory(m_currentKeys, sizeof(m_currentKeys));
    ZeroMemory(m_previousKeys, sizeof(m_previousKeys));

    ZeroMemory(m_currentMouseButtons, sizeof(m_currentMouseButtons));
    ZeroMemory(m_previousMouseButtons, sizeof(m_previousMouseButtons));

    m_currentMousePos.x = m_currentMousePos.y = 0;
    m_previousMousePos.x = m_previousMousePos.y = 0;
}