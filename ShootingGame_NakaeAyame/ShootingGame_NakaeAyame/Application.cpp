#include "Application.h"
#include "Game.h"
#include <chrono>
#include <thread>

const auto ClassName = TEXT("2025 中江文瞳 就職作品");     //ウィンドウクラス名.
const auto WindowName = TEXT("2025 中江文瞳 就職作品");    //ウィンドウ名.

HINSTANCE  Application::m_hInst;        // インスタンスハンドル
HWND       Application::m_hWnd;         // ウィンドウハンドル
uint32_t   Application::m_Width;        // ウィンドウの横幅
uint32_t   Application::m_Height;       // ウィンドウの縦幅

//-----------------------------------------------------------------------------
// 実行
//-----------------------------------------------------------------------------
void Application::Run()
{
    if (AppInit())  //アプリケーションの初期化処理を正常に完了させられたら
    {
        MainLoop();
    }
    TermApp();
}

//-----------------------------------------------------------------------------
// メインループ
//-----------------------------------------------------------------------------
void Application::MainLoop()
{
    MSG msg = {};

    Game::Init();

    // FPS計測用変数
    int fpsCounter = 0;
    long long oldTick = GetTickCount64(); // 前回計測時の時間
    long long nowTick = oldTick; // 今回計測時の時間

    // FPS固定用変数
    LARGE_INTEGER liWork; // workがつく変数は作業用変数
    long long frequency;// どれくらい細かく時間をカウントできるか
    QueryPerformanceFrequency(&liWork);
    frequency = liWork.QuadPart;
    // 時間（単位：カウント）取得
    QueryPerformanceCounter(&liWork);
    long long oldCount = liWork.QuadPart;// 前回計測時の時間
    long long nowCount = oldCount;// 今回計測時の時間


    // ゲームループ
    while (1)
    {
        // 新たにメッセージがあれば
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // ウィンドウプロシージャにメッセージを送る
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // 「WM_QUIT」メッセージを受け取ったらループを抜ける
            if (msg.message == WM_QUIT) {
                break;
            }
        }
        else
        {
            QueryPerformanceCounter(&liWork);// 現在時間を取得
            nowCount = liWork.QuadPart;
            // 1/60秒が経過したか？
            if (nowCount >= oldCount + frequency / 60) {

                // ゲーム処理実行
                Game::Update();

                // カメラセット
                Game::Draw();

                fpsCounter++; // ゲーム処理を実行したら＋１する
                oldCount = nowCount;
            }


        }
    }

    // テストプレーン終了処理
   // plane.Uninit();
    Game::Uninit();
}

//-----------------------------------------------------------------------------
// 初期化処理
//-----------------------------------------------------------------------------
bool Application::AppInit()
{
    // ウィンドウの初期化.
    if (!WindInit())
    {
        return false;
    }

    // 正常終了.
    return true;
}

bool Application::WindInit()
{
    // インスタンスハンドルを取得.
    auto hInst = GetModuleHandle(nullptr);
    if (hInst == nullptr)
    {
        return false;
    }

    // ウィンドウの設定.
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wc.hCursor = LoadCursor(hInst, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = ClassName;
    wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

    // ウィンドウの登録.
    if (!RegisterClassEx(&wc))
    {
        return false;
    }

    // インスタンスハンドル設定.
    m_hInst = hInst;

    // ウィンドウのサイズを設定.
    RECT rc = {};
    rc.right = static_cast<LONG>(m_Width);
    rc.bottom = static_cast<LONG>(m_Height);

    // ウィンドウサイズを調整.
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRect(&rc, style, FALSE);

    // ウィンドウを生成.
    m_hWnd = CreateWindowEx(
        0,
        //        WS_EX_TOPMOST,
        ClassName,
        WindowName,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        m_hInst,
        nullptr);

    if (m_hWnd == nullptr)
    {
        return false;
    }

    // ウィンドウを表示.
    ShowWindow(m_hWnd, SW_SHOWNORMAL);

    // ウィンドウを更新.
    UpdateWindow(m_hWnd);

    // ウィンドウにフォーカスを設定.
    SetFocus(m_hWnd);

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
// 終了処理
//-----------------------------------------------------------------------------
void Application::TermApp()
{
    // ウィンドウの終了処理.
    TermWnd();
}

void Application::TermWnd()
{
    // ウィンドウの登録を解除.
    if (m_hInst != nullptr)
    {
        UnregisterClass(ClassName, m_hInst);
    }

    m_hInst = nullptr;
    m_hWnd = nullptr;
}

//-----------------------------------------------------------------------------
// ウィンドウプロシージャ処理
//-----------------------------------------------------------------------------
LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:// ウィンドウ破棄のメッセージ
    {
        PostQuitMessage(0);// 「WM_QUIT」メッセージを送る　→　アプリ終了
    }
    break;

    case WM_CLOSE:  // 「x」ボタンが押されたら
    {
        int res = MessageBoxA(NULL, "終了しますか？", "確認", MB_OKCANCEL);
        if (res == IDOK) {
            DestroyWindow(hWnd);  // 「WM_DESTROY」メッセージを送る
        }
    }
    break;

    case WM_KEYDOWN: //キー入力があったメッセージ
    {
        if (LOWORD(wParam) == VK_ESCAPE) { //入力されたキーがESCAPEなら
            PostMessage(hWnd, WM_CLOSE, wParam, lParam);//「WM_CLOSE」を送る
        }
    }
    break;

    default:
    {   // 受け取ったメッセージに対してデフォルトの処理を実行
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;
    }

    }

    return 0;
}
