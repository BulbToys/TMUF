#include <windows.h>

#include "../core/bulbtoys.h"
#include "tmuf.h"

BOOL APIENTRY DllMain(HMODULE instance, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		// Version check :3
		if (!strncmp(reinterpret_cast<char*>(0xCCD45B), ":3", 2))
		{
			BulbToys::SetupParams params;
			params.instance = instance;
			params.GetD3D9Device = +[]() 
			{
				LPVOID device = nullptr;

				auto vision_viewport = TMUF::BulbToys_GetVisionViewportDX9();
				if (vision_viewport)
				{
					device = Read<LPVOID>(vision_viewport + 0x9F8);
				}

				return device;
			};
			params.GetDI8KeyboardDevice = +[]()
			{
				return TMUF::BulbToys_GetDI8Device(1);
			};
			params.GetDI8MouseDevice = +[]()
			{
				return TMUF::BulbToys_GetDI8Device(0);
			};
			params.settings_file = "TMUF_BulbToys.ini";
			BulbToys::Setup(params);
		}
		else
		{
			Error("This version of TMUF is not supported.");
			return FALSE;
		}
	}

	return TRUE;
}
