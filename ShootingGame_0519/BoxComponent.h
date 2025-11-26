#pragma once
#include "Component.h"
#include "Primitive.h"
#include <SimpleMath.h>
#include <memory>

using namespace DirectX::SimpleMath;

class BoxComponent : public Component
{
public:
    BoxComponent() = default;
    ~BoxComponent() override = default;

    void Initialize() override;
    void Update(float dt) override {}
    void Draw(float alpha) override;

    // Unit box を使う設計なので、描画サイズは GameObject のスケールで制御します。
    // それでも個別に見た目の色やUVなどを設定したければここで拡張可能。

private:
    // 共有 Primitive（ユニットボックス） ? 全インスタンスで共有して GPU リソースの重複を避ける
    static std::shared_ptr<Primitive> s_sharedPrimitive;
};

