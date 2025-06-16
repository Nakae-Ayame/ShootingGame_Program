#include <iostream>
#include "GameScene.h"
#include "Input.h"
#include "renderer.h"
#include "Application.h"

void GameScene::Init()
{
    // レンダラー初期化はアプリケーション側で済んでいる想定
    m_FreeCamera.Init();

    // Model のロード
    m_Model = std::make_unique<Model>();
    if (!m_Model->LoadFromFile("asset/Model/rabbit.fbx"))
    {
        // 読み込み失敗時はログ出力など
        std::cout << "モデル読み込みに失敗しました\n";
        OutputDebugStringA("モデル読み込みに失敗しました：Assets/ship.obj\n");
    }

    /*std::vector<VERTEX_3D> vertices;
    std::vector<UINT> indices;

    // Z=+1 の平面（XZ面をY軸方向に広げる）
    VERTEX_3D v0{ {-0.5f, +0.5f, +1.0f}, {0, 0, -1}, {1,1,1,1}, {0,0}, {}, {}, {}, 0 };
    VERTEX_3D v1{ {-0.5f, -0.5f, +1.0f}, {0, 0, -1}, {1,1,1,1}, {0,1}, {}, {}, {}, 0 };
    VERTEX_3D v2{ {+0.5f, -0.5f, +1.0f}, {0, 0, -1}, {1,1,1,1}, {1,1}, {}, {}, {}, 0 };
    VERTEX_3D v3{ {+0.5f, +0.5f, +1.0f}, {0, 0, -1}, {1,1,1,1}, {1,0}, {}, {}, {}, 0 };

    int base = (int)vertices.size();
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);

    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);


    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = static_cast<UINT>(sizeof(VERTEX_3D) * vertices.size());
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = vertices.data();
    Renderer::GetDevice()->CreateBuffer(&bd, &initData, m_VB.GetAddressOf());

    D3D11_BUFFER_DESC ibd{};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iData{};
    iData.pSysMem = indices.data();
    Renderer::GetDevice()->CreateBuffer(&ibd, &iData, m_IB.GetAddressOf());

    m_IndexCount = (UINT)indices.size();*/
}

void GameScene::Update(uint64_t deltatime)
{

	Input::Update();

	/*if (Input::IsMouseRightPressed())
	{
		std::cout << "右クリックできています\n";
	}*/
	
	m_FreeCamera.Update(deltatime);

    // カメラの位置を表示
    /*const Vector3& pos = m_FreeCamera.GetPosition();
    std::cout << "カメラ位置: (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";*/
}

void GameScene::Draw(uint64_t deltatime)
{
    SRT tr;
    tr.pos = { 0.0f, 0.0f, 5.0f };          // Z＝5 の位置に配置
    tr.rot = { 0.0f, XMConvertToRadians(180.0f), 0.0f }; // Yaw 180°
    tr.scale = { 2.0f, 2.0f, 2.0f };          // 等倍

    m_Model->Draw(tr);
    /*auto dc = Renderer::GetDeviceContext();

    Matrix4x4 world = Matrix4x4::Identity;
    world = world.Transpose();
    dc->UpdateSubresource(Renderer::GetWorldBuffer(), 0, nullptr, &world, 0, 0);

    /*Vector3 eye     = { 0, 0, -3 };
    Vector3 target  = { 0, 0,  0 };
    Vector3 up      = { 0, 1,  0 };
    Matrix4x4 view = Matrix4x4::CreateLookAt(eye, target, up).Transpose();
    dc->UpdateSubresource(Renderer::GetViewBuffer(), 0, nullptr, &view, 0, 0);

    float aspect = static_cast<float>(Application::GetWidth()) / Application::GetHeight();
    Matrix4x4 proj = Matrix4x4::CreatePerspectiveFieldOfView(PI / 4, aspect, 0.1f, 100.0f).Transpose();
    dc->UpdateSubresource(Renderer::GetProjectionBuffer(), 0, nullptr, &proj, 0, 0);

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    dc->IASetVertexBuffers(0, 1, m_VB.GetAddressOf(), &stride, &offset);
    dc->IASetIndexBuffer(m_IB.Get(), DXGI_FORMAT_R32_UINT, 0);
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    dc->DrawIndexed(m_IndexCount, 0, 0);*/
}

void GameScene::Uninit()
{
    // 特に解放するものがあれば
    m_Model.reset();
}