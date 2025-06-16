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
    ComPtr<ID3D11Buffer> vb;   //モデルの座標データ(頂点)
    ComPtr<ID3D11Buffer> ib;   //モデルのインデックスデータ(どことどこを繋ぐのかどうか)
    UINT indexCount = 0;       //このメッシュに対して何インデックス分描くかという数
};

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
    void ProcessNode(aiNode* node, const aiScene* scene);
    MeshPart ProcessMesh(aiMesh* mesh);
};
