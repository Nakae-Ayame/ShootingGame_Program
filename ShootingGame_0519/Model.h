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

    int materialIndex = -1; // マテリアル参照用
};

struct Material
{
    Vector4 diffuseColor = { 1, 1, 1, 1 };
    std::string name;                   //マテリアル名

    std::string texPath_Diffuse;        //拡散テクスチャのファイルパス
    std::string texPath_Normal;         //法線マップのファイルパス
    std::string texPath_Specular;       //スペキュラ鏡面反射テクスチャのファイルパス。

    ComPtr<ID3D11ShaderResourceView> texSRV_Diffuse;        //拡散テクスチャのシェーダーリソースビュー
    ComPtr<ID3D11ShaderResourceView> texSRV_Normal;         //法線マップのシェーダーリソースビュー
    ComPtr<ID3D11ShaderResourceView> texSRV_Specular;       //スペキュラテクスチャのシェーダーリソースビュー
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

    Material LoadMaterial(aiMaterial* aimat);

    //Transformを渡して描画する関数
    void Draw(const DirectX::XMMATRIX& worldMatrix);

private:
    std::vector<MeshPart> meshes_;
    std::vector<Material> materials_;

    std::string modelDirectory_;
    void ProcessNode(aiNode* node, const aiScene* scene);
    MeshPart ProcessMesh(aiMesh* mesh, const aiScene* scene);
};
