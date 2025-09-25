#pragma once

class TransitionRenderer
{
public:
    static bool Init();
    static void Shutdown();

    // progress: 0.0 -> 1.0
    static void DrawFade(float progress);
    static void DrawIris(float progress);

private:
    // internal helpers
    static void DrawFullScreen(const float color[4]);
    static void DrawFullScreenIris(float centerX, float centerY, float radius, const float color[4]);

};


