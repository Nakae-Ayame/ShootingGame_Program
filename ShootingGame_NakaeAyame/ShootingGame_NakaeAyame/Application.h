#pragma once
#include    <Windows.h>
#include    <cstdint>

class Application
{
public:
   //------------
   //関数
   //------------
    Application(uint32_t width, uint32_t height) //Applicationクラスのコンストラクタ
    {
        m_Height = height;
        m_Width = width;

        timeBeginPeriod(1);
    }

    ~Application()//Applicationクラスのデストラクタ
    {
        timeEndPeriod(1);
    }
    
   
    static uint32_t GetWidth()  //幅を取得する関数
    {
        return m_Width;
    }

   
    static uint32_t GetHeight() //高さを取得する関数
    {
        return m_Height;
    }

    
    static HWND GetWindow()     //ウインドウハンドルを返す関数
    {
        return m_hWnd;
    }

    static void Run();          //アプリケーションのループ関数 

   //------------
   //変数
   //------------


private:
    //------------
    //関数
    //------------
    
    static void MainLoop();       //ゲームシーンなどのループを回す関数
    static bool WindInit();       //ウィンドウの開始処理関数
    static bool AppInit();        //アプリケーションの開始処理関数
    static void TermApp();        //アプリケーションの終了処理関数
    static void TermWnd();        //ウィンドウの終了処理関数
   
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);  //ウィンドウプロシージャ関数

    //------------
    //変数
    //------------
    static uint32_t    m_Width;        // ウィンドウの横幅
    static uint32_t    m_Height;       // ウィンドウの縦幅
    static HINSTANCE   m_hInst;        // インスタンスハンドル
    static HWND        m_hWnd;         // ウィンドウハンドル

};


