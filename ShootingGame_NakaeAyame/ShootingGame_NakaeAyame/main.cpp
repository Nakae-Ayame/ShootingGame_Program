#include "main.h"
#include "Application.h"
//【命名規則】　2025/2/25
//変数：キャメルケース（小文字で始まり、大文字で単語を区切る）
//例 : myVariable, playerScore
// 
//定数名：すべて大文字で、単語間に_入れる
//例 : MAX_SPEED, PI_CONSTANT
// 
//クラス名・構造体名：パスカルケース（各単語の先頭を大文字にする）
//例 : PlayerCharacter, GameEngine
// 
//関数名：パスカルケース（各単語の先頭を大文字にする）
//例 : updatePosition, calculateDistance
// 
//メンバ変数：m_から始め、以降はキャメルケースと同じ
//例 : m_health, _score
// 
//グローバル変数：g_から始め、以降はキャメルケースと同じ
//例 : g_totalScore, g_playerList
// 
//名前空間：パスカルケース（各単語の先頭を大文字にする）
//例 : MathUtilities, GamePhysics
// 
//列挙型：パスカルケースで列挙型名を付け、各要素はすべて大文字で_を使う
// 例 enum class Direction {
//    LEFT,
//    RIGHT,
//  　TOP,
//    UNDER　};

//=======================================
//エントリーポイント
//=======================================
int main(void)
{

#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif//defined(DEBUG) || defined(_DEBUG)

    // アプリケーション実行
    Application app(SCREEN_WIDTH, SCREEN_HEIGHT);
    app.Run();

    return 0;
}