#pragma once
#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <future>
#include <mutex>
#include <assimp/scene.h>
#include "ModelCPUData.h" 
#include "ModelGPUData.h" 
//------------------------------------------------------------
//ResourceManagerクラス
//CPUキャッシュ(ModelData)とGPUキャッシュ(GPUModel)
//を持って、非同期などを提供できるクラス
//------------------------------------------------------------
class ResourceManager
{
	static ResourceManager& Get();
	//Assimpを呼んでCPUで読み込みできる分を
	//読み込み生成してキャッシュに入れておく関数
	std::future<std::shared_ptr<ModelCPU>> LoadModelCPUAsync(const std::string& path);

	//CPU キャッシュを同期取得する関数
	std::shared_ptr<ModelCPU>GetCPU(const std::string& path);
	bool HasCPU(const std::string& path);

	//読み込んでいたModelCPUの配列からから VB/IBを作り、テクスチャは CreateWICTextureFromFileなどで
	//SRV を作る。結果をGPUModel にて GPU キャッシュに入れる。
	std::shared_ptr<GPUModel> CreateGPUFromCPU(const std::shared_ptr<ModelCPU>& cpu);

	//GPUキャッシュを動機取得する関数
	std::shared_ptr<GPUModel> GetGPU(const std::string& path);
	bool HasGPU(const std::string& path);

private:
	ResourceManager() = default;
	~ResourceManager() = default;

	std::mutex m_mutex;
	std::unordered_map<std::string, std::shared_ptr<ModelCPU>> m_cpuCache;
	std::unordered_map<std::string, std::shared_ptr<GPUModel>> m_gpuCache;
};
