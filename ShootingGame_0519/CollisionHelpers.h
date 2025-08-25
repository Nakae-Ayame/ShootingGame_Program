#pragma once
#include <SimpleMath.h>
//----------------------------------------------------------------
// 当たり判定を取る際に使えるような、数学系の処理をまとめておく
// ヘッダーファイルです
//----------------------------------------------------------------
using namespace DirectX::SimpleMath;

inline void ExtractAxesFromRotation(const Matrix& rot, Vector3 outAxes[3])
{
    outAxes[0] = Vector3(rot._11, rot._12, rot._13); // Right
    outAxes[1] = Vector3(rot._21, rot._22, rot._23); // Up
    outAxes[2] = Vector3(rot._31, rot._32, rot._33); // Forward

    outAxes[0].Normalize();
    outAxes[1].Normalize();
    outAxes[2].Normalize();
}
