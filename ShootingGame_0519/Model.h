#pragma once
#include <string>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>
#include "CommonTypes.h"
#include "Transform.h"
#include <assimp/scene.h>

using Microsoft::WRL::ComPtr;

//-------------------------------------------
//1つのメッシュに必要なものを固めた構造体
//-------------------------------------------
struct MeshPart 
{
    ComPtr<ID3D11Buffer> vb;
    ComPtr<ID3D11Buffer> ib;
    UINT indexCount = 0;

    ComPtr<ID3D11ShaderResourceView> textureSRV;
};

//------------------------------------------------------------------
//Modelクラス
//3Dモデルの読み込み・保持・描画をしてる
//fbx や objをAssimpライブラリを使って読み込んでいる
// テクスチャ付きモデルも対応できる…はず
//------------------------------------------------------------------
class Model
{
public:
    Model() = default;  //コンストラクタ
    ~Model() = default; //デストラクタ

    //ファイルからモデルを読み込む関数
    bool LoadFromFile(const std::string& path);

    //Transformを渡して描画する関数
    void Draw(const SRT& transform);

private:
    std::vector<MeshPart> meshes_;

    std::string modelDirectory_;
    void ProcessNode(aiNode* node, const aiScene* scene);
    MeshPart ProcessMesh(aiMesh* mesh, const aiScene* scene);
};
