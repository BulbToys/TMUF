#include "settings.h"

Settings::Settings(const char* filename)
{
	mINI::INIFile file(filename);
	mINI::INIStructure file_ini;
	file.read(file_ini);

	for (auto const& iter : Settings::ini)
	{
		auto const& section = iter.first;
		if (file_ini.has(section))
		{
			auto const& collection = iter.second;
			for (auto const& iter_2 : collection)
			{
				auto const& key = iter_2.first;
				if (file_ini[section].has(key))
				{
					Settings::ini[section][key] = file_ini[section][key];
				}
			}
		}
	}

	file.write(Settings::ini, true);

	for (int i = 0; i < 256; i++)
	{
		auto vk = (uint8_t)i;
;		char* str = new char[32] { 0 };

		auto sc = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
		if (sc)
		{
			GetKeyNameTextA(sc << 16, str, 32);
		}
		
		if (strlen(str) < 1)
		{
			MYPRINTF(str, 32, "%s", "(none)");
		}

		keys.insert({ vk, str });
	}
}

Settings::~Settings()
{
	auto iter = keys.begin();
	while (iter != keys.end())
	{
		delete iter->second;
		keys.erase(iter);
	}
}

Settings* Settings::Get(const char* filename)
{
	if (!Settings::instance && filename)
	{
		Settings::instance = new Settings(filename);
	}
	return Settings::instance;
}

void Settings::End()
{
	Settings::instance = nullptr;
	delete this;
}

uint8_t Settings::StrToVK(const char* str)
{
	auto iter = keys.begin();
	while (iter != keys.end())
	{
		if (!strcmp(iter->second, str))
		{
			return iter->first;
		}
		++iter;
	}

	return 0;
}
