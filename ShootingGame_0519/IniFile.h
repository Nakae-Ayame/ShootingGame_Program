#pragma once
#include <Windows.h>
#include <string>
#include <cstdlib>

class IniFile
{
public:
	explicit IniFile(const char* filePath)
		: m_fileName(filePath ? filePath : ""){ }

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

	int ReadInt(const char* section, const char* key, int defaultValue)
	{
		if (!section || !key)
		{
			return defaultValue;
		}

		return static_cast<int>(GetPrivateProfileIntA(section, key, defaultValue, m_fileName.c_str()));

	}

	void WriteFloat(const char* section, const char* key, float value) const
	{
		if (!section || !key)
		{
			return;
		}

		char buf[64]{};
		sprintf_s(buf, "%.6f", value);
		WritePrivateProfileStringA(section, key, buf, m_fileName.c_str());
	}

	void WriteInt(const char* section, const char* key, int value) const
	{
		if (!section || !key)
		{
			return;
		}

		char buf[64]{};
		sprintf_s(buf, "%d", value);
		WritePrivateProfileStringA(section, key, buf, m_fileName.c_str());
	}

private:
	std::string m_fileName;	
};

