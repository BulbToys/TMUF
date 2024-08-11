#include <windows.h>

#include "../core/bulbtoys.h"
#include "tmuf.h"

BOOL APIENTRY DllMain(HMODULE instance, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		// Version check :3
		//if (!strncmp(reinterpret_cast<char*>(0x9F03D5), ":33", 3))
		if (true)
		{
			BulbToys::SetupParams params;
			params.instance = instance;
			params.GetDevice = +[]() 
			{
				IDirect3DDevice9* device = nullptr;

				auto vision_viewport = TMUF::BulbToys_GetVisionViewportDX9();
				if (vision_viewport)
				{
					device = Read<IDirect3DDevice9*>(vision_viewport + 0x9F8);
				}

				return device;
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
