#pragma once
#include <Windows.h>
#include <string>
#include <cstdlib>

class IniFile
{
public:
	explicit IniFile(const char* filePath)
		: m_fileName(filePath ? filePath : ""){ }

	/// <summary>
	/// .iniファイルからfloatの値を読み込むための関数
	/// </summary>
	/// <param name="section">セクション名</param>
	/// <param name="kay">キー名</param>
	/// <param name="defaultValue">読み込めなかった際に入れる値</param>
	/// <returns></returns>
	float ReadFloat(const char* section, const char* kay, float defaultValue) const
	{
		if (!section || !kay)
		{
			return defaultValue;
		}

		char buf[256]{};
		DWORD len = GetPrivateProfileStringA(section, kay, "", buf, static_cast<DWORD>(sizeof(buf)), m_fileName.c_str());

		if(len == 0)
		{
			return defaultValue;
		}

		return static_cast<float>(std::atof(buf));
	}

	/// <summary>
	/// .iniファイルからintの値を読み込むための関数
	/// </summary>
	/// <param name="section">セクション名</param>
	/// <param name="kay">キー名</param>
	/// <param name="defaultValue">読み込めなかった際に入れる値</param>
	/// <returns></returns>
	int ReadInt(const char* section, const char* key, int defaultValue)
	{
		if (!section || !key)
		{
			return defaultValue;
		}

		return static_cast<int>(GetPrivateProfileIntA(section, key, defaultValue, m_fileName.c_str()));

	}
	
	/// <summary>
	/// 
	/// </summary>
	/// <param name="section"></param>
	/// <param name="key"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	bool WriteFloatResult(const char* section, const char* key, float value) const
	{
		if (!section || !key)
		{
			return false;
		}

		char buf[64]{};
		sprintf_s(buf, "%.6f", value);

		bool ok = WritePrivateProfileStringA(section, key, buf, m_fileName.c_str());
		return ok;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="section"></param>
	/// <param name="key"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	bool WriteIntResult(const char* section, const char* key, int value) const
	{
		if (!section || !key)
		{
			return false;
		}

		char buf[64]{};
		sprintf_s(buf, "%d", value);

		bool ok = WritePrivateProfileStringA(section, key, buf, m_fileName.c_str());
		return ok;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="section"></param>
	/// <param name="key"></param>
	/// <param name="value"></param>
	void WriteFloat(const char* section, const char* key, float value) const
	{
		if (!section || !key){ return; }

		char buf[64]{};
		sprintf_s(buf, "%.6f", value);
		WritePrivateProfileStringA(section, key, buf, m_fileName.c_str());
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="section"></param>
	/// <param name="key"></param>
	/// <param name="value"></param>
	void WriteInt(const char* section, const char* key, int value) const
	{
		if (!section || !key){ return; }

		char buf[64]{};
		sprintf_s(buf, "%d", value);
		WritePrivateProfileStringA(section, key, buf, m_fileName.c_str());
	}

	
private:
	std::string m_fileName;	
};

