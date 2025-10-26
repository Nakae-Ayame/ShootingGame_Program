#pragma once
#include "Component.h"
#include "renderer.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <string>
#include <vector>
#include <memory>
#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>
#include <filesystem>

class ModelComponent : public Component
{
public:
    //コンストラクタ
    ModelComponent() = default;
    ModelComponent(const std::string& filepath);

    //デストラクタ
    ~ModelComponent() override = default;

    //モデルファイル読み込み関数
    void LoadModel(const std::string& filepath);

    //初期化
    void Initialize() override;

    //更新
    void Update(float dt) override;

    //描画
    void Draw(float alpha) override;

    //
    void SetColor(const Color& color);

private:
    // 内部処理
    void ProcessNode(aiNode* node, const aiScene* scene);
    void ProcessMesh(aiMesh* mesh, const aiScene* scene);
    void LoadMaterials(const aiScene* scene); // シーン内マテリアル一覧を処理
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTextureFromMaterial(aiMaterial* mat, aiTextureType t);

    // スキニング（将来のためのデータ）
    struct BoneInfo
    {
        std::string name;                 // ボーン名
        aiMatrix4x4 offsetMatrix;         // inverse bind pose
        // 最終変換行列 (アニメーション計算後にここに入れる)
        aiMatrix4x4 finalTransform;
    };

    // 1メッシュ分のデータ
    struct MeshData
    {
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
        UINT indexCount = 0;

        MATERIAL material; // 既存の MATERIAL 型に合わせて格納
        // 複数テクスチャ対応（必要に応じて増やす）
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvDiffuse;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvNormal;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSpecular;

        // スキニング用：このメッシュに含まれるボーンの名前リスト / インデックスは globalBoneMap を参照
        std::vector<std::string> boneNames;
    };

    std::vector<MeshData> m_meshes;

    // Assimp のインポータとシーンをメンバに持って lifetime を延ばす
    Assimp::Importer m_importer;
    const aiScene* m_scene = nullptr;

    // ボーンマップ（ボーン名 -> グローバルインデックス）
    std::vector<BoneInfo> m_boneInfos;
    std::unordered_map<std::string, int> m_boneNameToIndex;

    // マテリアルリスト（シーン単位）
    std::vector<MATERIAL> m_materials;

    // ファイルパス関連
    std::string m_filepath;
    std::string m_directory;

    // 将来アニメーション時に使用する定数バッファのハンドル等をここに置く (TODO)
    // ComPtr<ID3D11Buffer> m_cbBones;  // ボーン配列を送るための定数バッファなど

    // ヘルパー
    bool FileExists(const std::string& path);
};
