#pragma once
#include "utils.h"

#define MINI_CASE_SENSITIVE
#include "../mini/ini.h"

#include <format>

#define INVALID_KEY "(invalid)"

class Settings
{
	const char* filename;

	static inline std::vector<std::pair<const char*, const char*>> key_settings;
	static inline mINI::INIStructure ini;
	static inline Settings* instance = nullptr;

	using KeyMap = std::unordered_map<uint8_t, char*>;
	KeyMap keys;

	Settings(const char* filename);
	~Settings();

	void InitKeyMap();
	void DeleteKeyMap();
public:
	static Settings* Get(const char* filename = nullptr);
	void End();

	inline char* VKToStr(uint8_t vk) { return keys.at(vk); }
	uint8_t StrToVK(const char* str);

	template <typename T>
	using ValidateFn = bool(T);

	// NOTE: Settings are initialized in BulbToys::Init(), way after all static and global variables
	// Thus, the INI structure will be empty, as it hasn't been read from a file yet, and guarantees your settings will ALWAYS use their default values
	// To avoid this behavior in modules for example, you can put settings in Init() (and copy the values to global variables for later use if necessary)
	/* ===== I N T 3 2 ===== */
	template <StringLiteral section, StringLiteral key, int default_value>
	class Int32
	{
		inline static bool init = [](){
			Settings::ini[section.str][key.str] = std::to_string(default_value);
			return true;
		}();
		int value = default_value;
	public:
		Int32(ValidateFn<int>* Validate = nullptr)
		{
			if (Settings::Get())
			{
				if (sscanf_s(Settings::ini[section.str][key.str].c_str(), "%d", &value) == 1)
				{
					if (Validate)
					{
						if (!Validate(value))
						{
							value = default_value;
						}
					}
				}
			}
		}

		int& Get() { return value; }
	};

	/* ===== U I N T 3 2 ===== */
	template <StringLiteral section, StringLiteral key, uint32_t default_value>
	class UInt32
	{
		inline static bool init = []() {
			Settings::ini[section.str][key.str] = std::to_string(default_value);
			return true;
		}();
		uint32_t value = default_value;
	public:
		UInt32(ValidateFn<uint32_t>* Validate = nullptr)
		{
			if (Settings::Get())
			{
				if (sscanf_s(Settings::ini[section.str][key.str].c_str(), "%u", &value) == 1)
				{
					if (Validate)
					{
						if (!Validate(value))
						{
							value = default_value;
						}
					}
				}
			}
		}

		uint32_t& Get() { return value; }
	};

	/* ===== U I N T 8 ===== */
	template <StringLiteral section, StringLiteral key, uint8_t default_value>
	class UInt8
	{
		inline static bool init = []() {
			Settings::ini[section.str][key.str] = std::to_string(default_value);
			return true;
		}();
		uint8_t value = default_value;
	public:
		UInt8(ValidateFn<uint8_t>* Validate = nullptr)
		{
			if (Settings::Get())
			{
				if (sscanf_s(Settings::ini[section.str][key.str].c_str(), "%hhu", &value) == 1)
				{
					if (Validate)
					{
						if (!Validate(value))
						{
							value = default_value;
						}
					}
				}
			}
		}

		uint8_t& Get() { return value; }
	};

	/* ===== B O O L ===== */
	template <StringLiteral section, StringLiteral key, bool default_value>
	class Bool
	{
		inline static bool init = []() {
			Settings::ini[section.str][key.str] = default_value ? "true" : "false";
			return true;
		}();
		bool value = default_value;
	public:
		Bool()
		{
			if (Settings::Get())
			{
				if (Settings::ini[section.str][key.str] == "true")
				{
					value = true;
				}
				else if (Settings::ini[section.str][key.str] == "false")
				{
					value = false;
				}
			}
		}

		bool& Get() { return value; }
	};

	/* ===== F L O A T ===== */
	template <StringLiteral section, StringLiteral key, float default_value>
	class Float
	{
		inline static bool init = []() {
			Settings::ini[section.str][key.str] = std::to_string(default_value);
			return true;
		}();
		float value = default_value;
	public:
		Float(ValidateFn<float>* Validate = nullptr)
		{
			if (Settings::Get())
			{
				if (sscanf_s(Settings::ini[section.str][key.str].c_str(), "%f", &value) == 1)
				{
					if (Validate)
					{
						if (!Validate(value))
						{
							value = default_value;
						}
					}
				}
			}
		}

		float& Get() { return value; }
	};

	/* ===== S T R I N G ===== */
	template <StringLiteral section, StringLiteral key, StringLiteral default_value, size_t size>
	class String
	{
		static inline bool init = [](){
			Settings::ini[section.str][key.str] = default_value.str;
			return true;
		}();
		char value[size] { 0 };
	public:
		String()
		{
			if (Settings::Get())
			{
				MYPRINTF(value, size, "%s", Settings::ini[section.str][key.str].c_str());
			}
		}

		char(&Get())[size] { return value; }
	};

	/* ===== K E Y ===== */
	template <StringLiteral section, StringLiteral key, uint8_t default_value>
	class Key
	{
		inline static bool init = []() {
			// Key settings need special attention, check the Settings constructor for more info
			Settings::key_settings.push_back({ section.str, key.str });

			// 1. Can't call VKToStr() here, because our Settings have not been initialized yet
			// 2. Key map is localized based on whichever language keyboard input you had selected on Windows when BulbToys Settings were initialized
			//    To maintain compatibility between systems which may have different language keyboard inputs it's better to save the VK code instead
			//    For ease of use, we will allow conversion from the key name to the VK code upon reading the setting (construction of Settings::Key)
			Settings::ini[section.str][key.str] = std::format("{:#02x}", default_value);
			return true;
		}();
		uint8_t value = default_value;
	public:
		Key()
		{
			auto settings = Settings::Get();
			if (settings)
			{
				// We save and parse VK codes in hexadecimal to avoid collisions with actual keys
				// This conversion was done in the constructor of Settings, for the reasons listed there
				// For example, VK_BACK (for backspace) is 8, will be misinterpreted by the "8" key, so we save 0x08 instead
				// We can't check for the VK code before the actual key because we get the inverse problem ("8" key will turn into backspace)
				// This silly mistake cost me 4 hours of debugging and trying to figure out why my key binds weren't working and why key vectors were empty :3
				if (sscanf_s(Settings::ini[section.str][key.str].c_str(), "0x%02hhx", &value) != 1)
				{
					value = default_value;
				}
			}
		}

		uint8_t& Get() { return value; }
	};
};
