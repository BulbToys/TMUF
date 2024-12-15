#include "settings.h"

Settings::Settings(const char* filename)
{
	Settings::instance = this;

	this->filename = filename;
	mINI::INIFile file(filename);
	mINI::INIStructure file_ini;
	file.read(file_ini);

	// Read the ini file, making sure it respects our own statically created ini structure
	// Our ini file should only contain settings that are actively used in BulbToys (ie. ones that are compiled into the binary), nothing more, nothing less
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

	// First, generate the key map, as it will be necessary later on
	this->InitKeyMap();

	// GetKeyNameText translates keys according to the currently active keyboard layout, therefore the function might return different results for different layouts
	// For config file portability sake, we save all "Key" settings as VK codes instead of the actual keys (even if we support parsing them on startup)
	// Each setting queries data during its construction, but we don't know at what point in the application's runtime that will happen
	// Because of this, by the time the "Key" setting is read, the user might have already changed their keyboard layout, and invalidated the keybind
	// For this reason, we make an exception for "Key" settings, and at least try to parse the actual keys and set them to their respective VK hex codes
	// Before we eventually pass on these settings to the "Key" settings once they're constructed and request them from the Settings::ini structure
	for (auto& it : Settings::key_settings)
	{
		auto& setting = Settings::ini[it.first][it.second];

		uint8_t _;
		if (sscanf_s(setting.c_str(), "0x%02hhx", &_) != 1)
		{
			auto vk = this->StrToVK(setting.c_str());
			if (vk)
			{
				setting = std::format("{:#02x}", vk);
			}
			else
			{
				setting = INVALID_KEY;
			}
		}
	}

	// Finally, save with our newly converted keys
	file.write(Settings::ini, true);
}

Settings::~Settings()
{
	// Save one more time to account for things like new default values
	mINI::INIFile file(this->filename);
	file.write(Settings::ini, true);

	this->DeleteKeyMap();

	Settings::instance = nullptr;
}

void Settings::Init(const char* filename)
{
	ASSERT(!Settings::instance);
	new Settings(filename);
}

Settings* Settings::Get()
{
	return Settings::instance;
}

void Settings::End()
{
	delete this;
}

void Settings::InitKeyMap()
{
	// Let's see what keys this layout has to offer...
	for (int i = 0; i < 256; i++)
	{
		auto vk = (uint8_t)i;
		char* str = new char[32] { 0 };

		auto sc = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
		if (sc)
		{
			GetKeyNameTextA(sc << 16, str, 32);
		}

		if (strlen(str) < 1)
		{
			MYPRINTF(str, 32, "%s", INVALID_KEY);
		}

		keys.insert({ vk, str });
	}
}

void Settings::DeleteKeyMap()
{
	auto iter = keys.begin();
	while (iter != keys.end())
	{
		auto key = *iter;

		keys.erase(iter);
		delete key.second;
	}
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
