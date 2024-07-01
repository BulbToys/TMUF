#include <Windows.h>
#include <d3d9.h>

#include "main.h"
#include "io.h"
#include "gui.h"
#include "modules.h"
#include "hooks.h"
#include "utils.h"
#include "settings.h"

DWORD WINAPI BulbToys_Main(LPVOID lpThreadParameter)
{
	BulbToys::SetupParams params = *(BulbToys::SetupParams*)lpThreadParameter;
	HeapFree(GetProcessHeap(), 0, lpThreadParameter);

	if (!BulbToys::Init(params, true))
	{
		FreeLibraryAndExitThread(params.instance, 0);
		return 0;
	}

	auto io = IO::Get();
	if (io)
	{
		while (!io->Done())
		{
			Sleep(200);
		}
	}

	BulbToys::End();

	FreeLibraryAndExitThread(params.instance, 0);
	return 0;
}

void BulbToys::Setup(BulbToys::SetupParams& params)
{
	if (!params.instance)
	{
		return;
	}

	DisableThreadLibraryCalls(params.instance);

	auto heap_params = (BulbToys::SetupParams*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BulbToys::SetupParams));
	if (!heap_params)
	{
		return;
	}
	*heap_params = params;

	auto thread = CreateThread(nullptr, 0, BulbToys_Main, heap_params, 0, nullptr);
	if (thread)
	{
		CloseHandle(thread);
	}
}

bool BulbToys::Init(BulbToys::SetupParams& params, bool thread)
{
	// Hooks
	if (Hooks::Init() != MH_OK)
	{
		return false;
	}

	// Settings
	Settings::Get(params.settings_file);

	// Modules
	Modules::Init();

	IDirect3DDevice9* device = nullptr;

	if (!params.GetDevice)
	{
		// Partial success - no IO or GUI
		return true;
	}

	if (thread)
	{
		while (!(device = params.GetDevice()))
		{
			Sleep(200);
		}
	}
	else
	{
		if (!(device = params.GetDevice()))
		{
			// Partial success - no IO or GUI
			return true;
		}
	}

	// IO
	D3DDEVICE_CREATION_PARAMETERS d3d_params;
	device->GetCreationParameters(&d3d_params);
	auto window = d3d_params.hFocusWindow;
	IO::Get(window);

	// GUI
	Settings::Bool<"BulbToys", "UseGUI", true> use_gui;
	if (use_gui.Get())
	{
		GUI::Get(device, window);
	}

	return true;
}

void BulbToys::End()
{
	// GUI
	auto gui = GUI::Get();
	if (gui)
	{
		gui->End();
	}

	// IO
	auto io = IO::Get();
	if (io)
	{
		io->End();
	}

	// Modules
	Modules::End();

	// Settings
	auto settings = Settings::Get();
	if (settings)
	{
		settings->End();
	}

	// Hooks
	if (Hooks::End() != MH_OK)
	{
		DIE();
	}

	// Final sanity check to make sure all our patches and hooks are gone
	if (!PatchInfo::SanityCheck())
	{
		DIE();
	}
}