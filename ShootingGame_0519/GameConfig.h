#pragma once

//--------------Player設定用構造体------------------
struct PlayerConfig
{
    float moveSpeed = 35.0f;
    float boostSpeed = 70.0f;
    float turnSpeed = 2.0f;
    float hpMax = 100.0f;
};

//--------------Camera設定用構造体------------------
struct CameraConfig
{
    float distance = 20.0f;
    float height = 6.0f;
    float fovDeg = 60.0f;
    float sensitivity = 1.0f;
};

