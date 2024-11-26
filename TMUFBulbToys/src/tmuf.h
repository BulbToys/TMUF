#pragma once

#include <cstdint>
#include <string>
#include <dinput.h>

#include "../core/bulbtoys/utils.h"
#include "../core/imgui/imgui.h"

namespace TMUF
{
	/* ===== N A M E S P A C E S ===== */



	/* ===== E N U M S ===== */

	enum _Engine
	{
		MwFoundations = 0x1,
		Data = 0x2,
		Game = 0x3,
		Graphic = 0x4,
		Function = 0x5,
		Hms = 0x6,
		Control = 0x7,
		Motion = 0x8,
		Plug = 0x9,
		Scene = 0xA,
		System = 0xB,
		Vision = 0xC,
		Psy = 0xD,
		Edit = 0xE,
		NodEdit = 0xD,
		Audio = 0x10,
		Script = 0x11,
		Net = 0x12,
		Input = 0x13,
		Xml = 0x14,
		Movie = 0x15,
		Ptp = 0x16,
		CyberDrive = 0x20,
		VirtualSkipper = 0x21,
		Adventure = 0x22,
		Lanfeust = 0x23,
		TrackMania = 0x24,
		Sorcieres = 0x25,
		Mg = 0x26,
		GbxViewer = 0x27,
		Gbe = 0x28,
		MediaTrackerApp = 0x29,
		RenderBox = 0x2A,
		Fbx = 0x2B,
		QuestMania = 0x2C,
		ShootMania = 0x2D,
		ClassicSb = 0x70,
		Anim = 0x71,
		SCEngine = 0x72,
		GLManager = 0x73,
		TLEngine = 0x74,
		UIEngine = 0x75,
		Undo = 0x76,
		PropertyEditor = 0x77,
		Storyboarder = 0x78,
	};


	/* ===== S T R U C T S ===== */

	template <typename T>
	struct CFastBuffer
	{
		int size = 0;
		T* pElems = nullptr;
		int capacity = 0;
	};

	template <typename T>
	struct CFastArray
	{
		int size = 0;
		T* pElems = nullptr;
	};

	struct CFastString
	{
		int size;
		char* psz;

		CFastString(const char* str = nullptr)
		{
			reinterpret_cast<uintptr_t(__thiscall*)(CFastString*, const char*)>(0x401660)(this, str);
		}

		~CFastString()
		{
			reinterpret_cast<void(__thiscall*)(CFastString*)>(0x4683E0)(this);
		}

		void SetString(int len, const char* str)
		{
			reinterpret_cast<void(__thiscall*)(CFastString*, int, const char*)>(0x440DC0)(this, len, str);
		}
	};

	struct SStringParamInt
	{
		const wchar_t* pwsz = nullptr;
		int size = 0;
		int _unk = 0;
	};

	struct CFastStringInt
	{
		int size;
		wchar_t* pwsz;

		CFastStringInt(const wchar_t* wstr = nullptr)
		{
			reinterpret_cast<uintptr_t(__thiscall*)(CFastStringInt*, const wchar_t*)>(0x401750)(this, wstr);
		}

		~CFastStringInt()
		{
			reinterpret_cast<void(__thiscall*)(CFastStringInt*)>(0x4F1030)(this);
		}

		void SetString(int len, const wchar_t* str)
		{
			SStringParamInt param;
			param.pwsz = str;
			param.size = len;
			
			reinterpret_cast<void(__thiscall*)(CFastStringInt*, SStringParamInt*)>(0x903A90)(this, &param);
		}
	};

	struct CMwId
	{
		int value;
	};

	struct GmVec3
	{
		float x;
		float y;
		float z;
	};

	struct SSystemTime
	{
		unsigned __int16 y;
		unsigned __int16 m_d;
		unsigned __int32 h_m_s_ms;
	};

	struct CGameMasterServer_SFeature
	{
		CMwId _ID;
		int _Value;
	};

	struct SGameCtnIdentifier
	{
		CMwId _ChallengeInfoID;
		CMwId _EnvironmentNameID;
		CMwId _AuthorNameID;
	};

	struct SScore : SGameCtnIdentifier
	{
		unsigned int unkNat32_a;
		unsigned int _RespawnsMaybe;
		unsigned int unkNat32_b;
		unsigned int unkNat32_c;
		unsigned int _EditPlayTime;
		unsigned int _RacePlayTime;
		unsigned int _NetPlayTime;
		unsigned __int16 _TotalResetCount;
		unsigned __int16 _TotalFinishCount;
		unsigned int unkNat32_d;
		unsigned int unkNat32_e;
		unsigned int unkNat32_f;
		unsigned __int16 unkNat16_a;
		unsigned __int16 unkNat16_b;
		char a;
		unsigned __int8 unkNat8_a;
		char b[2];
		CFastStringInt _TrackName;
		SSystemTime unkSysTime_a;
		SSystemTime unkSysTime_b;
		unsigned int unkNat32_g;
		unsigned int unkNat32_h;
		unsigned int unkNat32_i;
		unsigned int unkNat32_j;
		unsigned int unkNat32_k;
		unsigned int unkNat32_l;
		unsigned int unkNat32_m;
		unsigned int unkNat32_n;
	};

	struct CGamePlayerScore
	{
		void* __vftable;
		char a[16];
		CFastString PlayerName;
		CFastStringInt NickName;
		char b[4];
		CFastBuffer<SScore*> _Challenges;
		char c[80];
		unsigned int ChallengesCount;
		unsigned int TotalPlayTime;
		unsigned int EditPlayTime;
		unsigned int RacePlayTime;
		unsigned int NetPlayTime;
		unsigned int TotalResetCount;
		unsigned int TotalFinishCount;
		unsigned int MaxPlayTime;
		unsigned int MaxEditPlayTime;
		unsigned int MaxRacePlayTime;
		unsigned int MaxNetPlayTime;
		unsigned int MaxResetCount;
		unsigned int MaxFinishCount;
		unsigned int AvgPlayTime;
		unsigned int AvgEditPlayTime;
		unsigned int AvgRacePlayTime;
		unsigned int AvgNetPlayTime;
		unsigned int AvgResetCount;
		unsigned int AvgFinishCount;
		CFastStringInt MostPlayed;
		CFastStringInt MostEdited;
		CFastStringInt MostRaced;
		CFastStringInt MostNetted;
		char d[176];
	};

	struct SInputActionDesc
	{
		int field_0;
		CFastString name;
		int field_C;
		int field_10;
	};

	struct CIPCRemoteControl_SAuthParams
	{
		CFastString _Name;
		CFastString _Password;
		unsigned int _Level;
		__int16 _unk;
	};

	/* ===== C O N S T A N T S ===== */

	// Globals
	inline uintptr_t BulbToys_GetTrackMania() { return Read<uintptr_t>(0xD6A2A4); }

	inline uintptr_t BulbToys_GetVisionViewportDX9() { return Read<uintptr_t>(0xD70C00); }

	/* ===== F U N C T I O N S ===== */

	FUNC(0x935660, CFastString*, __thiscall, CMwId_GetName, CMwId* mw_id, CFastString* string);
	FUNC(0x42BD90, CMwId*, , CMwId_CreateFromLocalName, CMwId* mw_id, const char* local_name);

	FUNC(0x5F8420, uintptr_t, __thiscall, CGameCtnApp_GetCurrentInputBindings, uintptr_t game_ctn_app, int zero);

	FUNC(0x8F7D10, void, __thiscall, CInputBindingsConfig_ClearBindings, uintptr_t input_bindings_config, unsigned int action, int mw_id);
	FUNC(0x8F7CC0, unsigned int, __thiscall, CInputBindingsConfig_FindAction, uintptr_t input_bindings_config, SInputActionDesc* input_action_desc);

	/* ===== H E L P E R   F U N C S ===== */

	uintptr_t BulbToys_GetEngine(_Engine e);

	// 0 = Mouse, 1 = Keyboard, 2+ = Joystick
	LPVOID BulbToys_GetDI8Device(int index);

	uintptr_t BulbToys_GetControlFromFrame(const char* frame, const char* control);

	/* ===== C O N S T E X P R   F U N C S ===== */

	constexpr void SystemTimeToWin32SystemTime(SYSTEMTIME* win32_st, SSystemTime* st)
	{
		win32_st->wYear = st->y;
		win32_st->wMonth = st->m_d & 0xF;
		win32_st->wDayOfWeek = (*(unsigned int*)&st->y >> 20) & 7;
		win32_st->wDay = (*(unsigned int*)&st->y >> 23) & 0x1F;
		win32_st->wHour = st->h_m_s_ms & 0x1F;
		win32_st->wMinute = (st->h_m_s_ms >> 5) & 0x3F;
		win32_st->wSecond = (st->h_m_s_ms >> 11) & 0x3F;
		win32_st->wMilliseconds = (st->h_m_s_ms >> 17) & 0x3FF;
	}

	constexpr void Win32SystemTimeToSystemTime(SSystemTime* st, SYSTEMTIME* win32_st)
	{
		int v2; // ecx
		unsigned __int32 v3; // ecx
		int v4; // edx

		st->y = win32_st->wYear;
		*(unsigned int*)&st->y ^= (*(DWORD*)&st->y ^ (win32_st->wMonth << 16)) & 0xF0000;
		v2 = *(unsigned int*)&st->y ^ (*(unsigned int*)&st->y ^ (win32_st->wDayOfWeek << 20)) & 0x700000;
		*(unsigned int*)&st->y = v2;
		*(unsigned int*)&st->y = v2 ^ (v2 ^ (win32_st->wDay << 23)) & 0xF800000;
		st->h_m_s_ms ^= (st->h_m_s_ms ^ win32_st->wHour) & 0x1F;
		v3 = st->h_m_s_ms ^ ((unsigned __int16)st->h_m_s_ms ^ (unsigned __int16)(32 * win32_st->wMinute)) & 0x7E0;
		st->h_m_s_ms = v3;
		v4 = v3 ^ (v3 ^ (win32_st->wSecond << 11)) & 0x1F800;
		st->h_m_s_ms = v4;
		st->h_m_s_ms = v4 ^ (v4 ^ (win32_st->wMilliseconds << 17)) & 0x7FE0000;
	}
}

/* ===== I M G U I ===== */
namespace ImGui
{
	struct TMUF_TextSlice
	{
		std::string str = "";
		ImVec4 clr = ImVec4(.0f, .0f, .0f, .0f);

		TMUF_TextSlice(std::string string, ImVec4 color) : str(string), clr(color) {}
	};
	std::vector<TMUF_TextSlice> TMUF_Parse(const char* text);

	// Displays text according to TrackMania formatting rules. Unicode and certain TM formats are unsupported!
	void TMUF_Text(const char* text);
	void TMUF_TextEx(std::vector<TMUF_TextSlice>& slices, const char* tooltip = nullptr);

	// CFastString equivalent of ImGui::InputText
	void TMUF_InputFastString(TMUF::CFastString& fast_string, const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0,
		ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
}