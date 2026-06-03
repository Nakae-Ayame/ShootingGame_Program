#pragma once
#include <wrl/client.h>
#include <xaudio2.h>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

class Sound
{
public:
    static bool Init();
    static void Update(float dt);
    static void Uninit();

    //--------BGM関連-------
    static bool PlayBgmWav(const std::wstring& filepath, float volume = 0.7f);
    static void StopBgm();
    static void FadeInBgm(float targetVolume, float durationSec);
    static void FadeOutBgm(float durationSec);

    //--------SE関連-------
    static bool PlaySeWav(const std::wstring& filepath, float volume = 1.0f);
    static void StopAllSe();
    static void ClearSeCache();

    //--------Set関数-------
    static void SetBgmVolume(float volume);
    static void SetSeVolume(float volume);

    //--------Get関数-------
    static float GetBgmVolume();
    static float GetSeVolume();

private:
    struct WavData
    {
        WAVEFORMATEX format{};
        std::vector<uint8_t> buffer;
    };

    struct SeVoiceEntry
    {
        IXAudio2SourceVoice* voice = nullptr;
        const WavData* wav = nullptr; // キャッシュ参照（バッファ寿命確保）
    };

    static bool LoadWavPcm(const std::wstring& filepath, WavData& outData);
    static const WavData* GetOrLoadSeWav(const std::wstring& filepath);

    static Microsoft::WRL::ComPtr<IXAudio2> m_xAudio2;
    static IXAudio2MasteringVoice* m_masterVoice;

    //--------------BGM関連------------------
    static IXAudio2SourceVoice* m_bgmVoice;
    static float m_bgmVolume;
    static WavData m_bgmData;

    //--------------BGMフェード関連------------------
    static bool m_isFading;
    static bool m_fadeIn;
    static float m_fadeTimer;
    static float m_fadeDuration;
    static float m_fadeStartVolume;
    static float m_fadeTargetVolume;

    //--------------SE関連------------------
    static float m_seVolume;
    static std::unordered_map<std::wstring, WavData> m_seCache;
    static std::vector<SeVoiceEntry> m_seVoices;
};

