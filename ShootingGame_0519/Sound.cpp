#include "Sound.h"
#include <fstream>
#include <algorithm>

Microsoft::WRL::ComPtr<IXAudio2> Sound::m_xAudio2;
IXAudio2MasteringVoice* Sound::m_masterVoice = nullptr;
IXAudio2SourceVoice* Sound::m_bgmVoice = nullptr;
float Sound::m_bgmVolume = 0.7f;
Sound::WavData Sound::m_bgmData;

bool  Sound::m_isFading = false;
bool  Sound::m_fadeIn = true;
float Sound::m_fadeTimer = 0.0f;
float Sound::m_fadeDuration = 0.0f;
float Sound::m_fadeStartVolume = 0.0f;
float Sound::m_fadeTargetVolume = 0.0f;

float Sound::m_seVolume = 1.0f;
std::unordered_map<std::wstring, Sound::WavData> Sound::m_seCache;
std::vector<Sound::SeVoiceEntry> Sound::m_seVoices;

static uint32_t ReadU32(std::ifstream& ifs)
{
    uint32_t v = 0;
    ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

static uint16_t ReadU16(std::ifstream& ifs)
{
    uint16_t v = 0;
    ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

bool Sound::Init()
{
    HRESULT hr = XAudio2Create(m_xAudio2.ReleaseAndGetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_xAudio2->CreateMasteringVoice(&m_masterVoice);
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void Sound::Update(float dt)
{
    // ---- BGMフェード（前に作ったやつ）----
    if (m_isFading)
    {
        if (dt > 0.0f && m_bgmVoice)
        {
            m_fadeTimer += dt;

            float t = 1.0f;
            if (m_fadeDuration > 0.0f)
            {
                t = std::clamp(m_fadeTimer / m_fadeDuration, 0.0f, 1.0f);
            }

            float volume = m_fadeStartVolume + (m_fadeTargetVolume - m_fadeStartVolume) * t;
            SetBgmVolume(volume);

            if (t >= 1.0f)
            {
                m_isFading = false;
                if (!m_fadeIn)
                {
                    StopBgm();
                }
            }
        }
        else
        {
            m_isFading = false;
        }
    }

    // ---- SEの後始末（再生完了Voiceを破棄）----
    for (size_t i = 0; i < m_seVoices.size();)
    {
        IXAudio2SourceVoice* v = m_seVoices[i].voice;
        if (!v)
        {
            m_seVoices.erase(m_seVoices.begin() + i);
            continue;
        }

        XAUDIO2_VOICE_STATE st{};
        v->GetState(&st);

        if (st.BuffersQueued == 0)
        {
            v->Stop();
            v->FlushSourceBuffers();
            v->DestroyVoice();
            m_seVoices.erase(m_seVoices.begin() + i);
            continue;
        }

        ++i;
    }
}

void Sound::Uninit()
{
    StopAllSe();
    StopBgm();

    if (m_masterVoice)
    {
        m_masterVoice->DestroyVoice();
        m_masterVoice = nullptr;
    }

    m_xAudio2.Reset();
}


void Sound::FadeInBgm(float targetVolume, float durationSec)
{
    targetVolume = std::clamp(targetVolume, 0.0f, 1.0f);
    durationSec = max(durationSec, 0.0f);

    if (!m_bgmVoice)
    {
        return;
    }

    m_isFading = true;
    m_fadeIn = true;
    m_fadeTimer = 0.0f;
    m_fadeDuration = durationSec;

    m_fadeStartVolume = 0.0f;
    m_fadeTargetVolume = targetVolume;

    SetBgmVolume(0.0f);
}

void Sound::FadeOutBgm(float durationSec)
{
    durationSec = max(durationSec, 0.0f);

    if (!m_bgmVoice)
    {
        return;
    }

    m_isFading = true;
    m_fadeIn = false;
    m_fadeTimer = 0.0f;
    m_fadeDuration = durationSec;

    m_fadeStartVolume = m_bgmVolume;
    m_fadeTargetVolume = 0.0f;
}

const Sound::WavData* Sound::GetOrLoadSeWav(const std::wstring& filepath)
{
    auto it = m_seCache.find(filepath);
    if (it != m_seCache.end())
    {
        return &it->second;
    }

    WavData wav{};
    if (!LoadWavPcm(filepath, wav))
    {
        return nullptr;
    }

    auto res = m_seCache.emplace(filepath, std::move(wav));
    return &res.first->second;
}


bool Sound::LoadWavPcm(const std::wstring& filepath, WavData& outData)
{
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open())
    {
        return false;
    }

    char riff[4]{};
    ifs.read(riff, 4);
    if (std::memcmp(riff, "RIFF", 4) != 0)
    {
        return false;
    }

    (void)ReadU32(ifs); // file size

    char wave[4]{};
    ifs.read(wave, 4);
    if (std::memcmp(wave, "WAVE", 4) != 0)
    {
        return false;
    }

    bool foundFmt = false;
    bool foundData = false;

    WAVEFORMATEX fmt{};
    std::vector<uint8_t> data;

    while (ifs && (!foundFmt || !foundData))
    {
        char chunkId[4]{};
        ifs.read(chunkId, 4);
        if (!ifs)
        {
            break;
        }

        uint32_t chunkSize = ReadU32(ifs);

        if (std::memcmp(chunkId, "fmt ", 4) == 0)
        {
            uint16_t audioFormat = ReadU16(ifs);
            uint16_t numChannels = ReadU16(ifs);
            uint32_t sampleRate = ReadU32(ifs);
            uint32_t byteRate = ReadU32(ifs);
            uint16_t blockAlign = ReadU16(ifs);
            uint16_t bitsPerSample = ReadU16(ifs);

            if (chunkSize > 16)
            {
                ifs.seekg(chunkSize - 16, std::ios::cur);
            }

            if (audioFormat != 1)
            {
                // PCM以外はこの最小実装では非対応
                return false;
            }

            fmt.wFormatTag = WAVE_FORMAT_PCM;
            fmt.nChannels = numChannels;
            fmt.nSamplesPerSec = sampleRate;
            fmt.nAvgBytesPerSec = byteRate;
            fmt.nBlockAlign = blockAlign;
            fmt.wBitsPerSample = bitsPerSample;
            fmt.cbSize = 0;

            foundFmt = true;
        }
        else if (std::memcmp(chunkId, "data", 4) == 0)
        {
            data.resize(chunkSize);
            ifs.read(reinterpret_cast<char*>(data.data()), chunkSize);
            foundData = true;
        }
        else
        {
            ifs.seekg(chunkSize, std::ios::cur);
        }
    }

    if (!foundFmt || !foundData)
    {
        return false;
    }

    outData.format = fmt;
    outData.buffer = std::move(data);
    return true;
}

bool Sound::PlayBgmWav(const std::wstring& filepath, float volume)
{
    if (!m_xAudio2)
    {
        return false;
    }

    StopBgm();

    WavData wav{};
    if (!LoadWavPcm(filepath, wav))
    {
        return false;
    }

    m_bgmData = std::move(wav);

    HRESULT hr = m_xAudio2->CreateSourceVoice(&m_bgmVoice, &m_bgmData.format);
    if (FAILED(hr))
    {
        m_bgmVoice = nullptr;
        return false;
    }

    XAUDIO2_BUFFER buf{};
    buf.AudioBytes = static_cast<UINT32>(m_bgmData.buffer.size());
    buf.pAudioData = m_bgmData.buffer.data();
    buf.Flags = XAUDIO2_END_OF_STREAM;
    buf.LoopCount = XAUDIO2_LOOP_INFINITE;

    hr = m_bgmVoice->SubmitSourceBuffer(&buf);
    if (FAILED(hr))
    {
        StopBgm();
        return false;
    }

    m_bgmVolume = std::clamp(volume, 0.0f, 1.0f);
    m_bgmVoice->SetVolume(m_bgmVolume);

    hr = m_bgmVoice->Start();
    if (FAILED(hr))
    {
        StopBgm();
        return false;
    }

    return true;
}

bool Sound::PlaySeWav(const std::wstring& filepath, float volume)
{
    if (!m_xAudio2)
    {
        return false;
    }

    const WavData* wav = GetOrLoadSeWav(filepath);
    if (!wav)
    {
        return false;
    }

    IXAudio2SourceVoice* seVoice = nullptr;
    HRESULT hr = m_xAudio2->CreateSourceVoice(&seVoice, &wav->format);
    if (FAILED(hr) || !seVoice)
    {
        return false;
    }

    XAUDIO2_BUFFER buf{};
    buf.AudioBytes = static_cast<UINT32>(wav->buffer.size());
    buf.pAudioData = wav->buffer.data();
    buf.Flags = XAUDIO2_END_OF_STREAM;

    hr = seVoice->SubmitSourceBuffer(&buf);
    if (FAILED(hr))
    {
        seVoice->DestroyVoice();
        return false;
    }

    float finalVol = std::clamp(volume, 0.0f, 1.0f) * std::clamp(m_seVolume, 0.0f, 1.0f);
    seVoice->SetVolume(finalVol);

    hr = seVoice->Start();
    if (FAILED(hr))
    {
        seVoice->DestroyVoice();
        return false;
    }

    SeVoiceEntry entry{};
    entry.voice = seVoice;
    entry.wav = wav;
    m_seVoices.push_back(entry);

    return true;
}


void Sound::StopBgm()
{
    if (m_bgmVoice)
    {
        m_bgmVoice->Stop();
        m_bgmVoice->FlushSourceBuffers();
        m_bgmVoice->DestroyVoice();
        m_bgmVoice = nullptr;
    }

    m_bgmData.buffer.clear();
}

void Sound::SetBgmVolume(float volume)
{
    m_bgmVolume = std::clamp(volume, 0.0f, 1.0f);
    if (m_bgmVoice)
    {
        m_bgmVoice->SetVolume(m_bgmVolume);
    }
}

float Sound::GetBgmVolume()
{
    return m_bgmVolume;
}

void Sound::SetSeVolume(float volume)
{
    m_seVolume = std::clamp(volume, 0.0f, 1.0f);
}

float Sound::GetSeVolume()
{
    return m_seVolume;
}

void Sound::StopAllSe()
{
    for (auto& e : m_seVoices)
    {
        if (e.voice)
        {
            e.voice->Stop();
            e.voice->FlushSourceBuffers();
            e.voice->DestroyVoice();
            e.voice = nullptr;
        }
    }
    m_seVoices.clear();
}

void Sound::ClearSeCache()
{
    // 再生中のSEがあると参照が残るので止めてから消す
    StopAllSe();
    m_seCache.clear();
}