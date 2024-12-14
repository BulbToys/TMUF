#include <Windows.h>

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
			Sleep(BULBTOYS_SLEEP);
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

	LPVOID d3d9_device = nullptr;
	LPVOID keyboard_device = nullptr;
	LPVOID mouse_device = nullptr;

	if (!params.GetD3D9Device)
	{
		// Modules (No UI)
		Modules::Init();

		// Partial success - no IO or GUI
		return true;
	}

	if (thread)
	{
		while (!(d3d9_device = params.GetD3D9Device()))
		{
			Sleep(BULBTOYS_SLEEP);
		}

		if (params.GetDI8KeyboardDevice)
		{
			while (!(keyboard_device = params.GetDI8KeyboardDevice()))
			{
				Sleep(BULBTOYS_SLEEP);
			}
		}

		if (params.GetDI8MouseDevice)
		{
			while (!(mouse_device = params.GetDI8MouseDevice()))
			{
				Sleep(BULBTOYS_SLEEP);
			}
		}
	}
	else
	{
		if (!(d3d9_device = params.GetD3D9Device()))
		{
			// Modules (No UI)
			Modules::Init();

			// Partial success - no IO or GUI
			return true;
		}

		if (params.GetDI8KeyboardDevice)
		{
			keyboard_device = params.GetDI8KeyboardDevice();
		}

		if (params.GetDI8MouseDevice)
		{
			mouse_device = params.GetDI8MouseDevice();
		}
	}

	// IO
	uint8_t d3d_params[16] {0};

	using DxGetCreationParametersFn = long(__stdcall)(LPVOID, LPVOID);
	auto ID3DDevice9_GetCreationParameters = reinterpret_cast<DxGetCreationParametersFn*>(Virtual<9>(reinterpret_cast<uintptr_t>(d3d9_device)));
	ID3DDevice9_GetCreationParameters(d3d9_device, &d3d_params);

	auto window = Read<HWND>(reinterpret_cast<uintptr_t>(&d3d_params) + 0x8);
	IO::Get(window, keyboard_device, mouse_device);

	// GUI
	Settings::Bool<"BulbToys", "UseGUI", true> use_gui;
	if (use_gui.Get())
	{
		GUI::Get(d3d9_device, window);
	}

	// Modules
	Modules::Init();

	return true;
}

void BulbToys::End()
{
	auto io = IO::Get();
	if (io)
	{
		// Modules
		Modules::End();
	}

	// GUI
	auto gui = GUI::Get();
	if (gui)
	{
		gui->End();
	}

	// IO
	if (io)
	{
		io->End();
	}
	else
	{
		// Modules (No UI)
		Modules::End();
	}
	
	// Settings
	auto settings = Settings::Get();
	if (settings)
	{
		settings->End();
	}

	// Hooks
	if (Hooks::End() != MH_OK)
	{
		// No need for an assert, the function already prints out an error
		DIE();
	}

	// Final sanity check to make sure all our patches and hooks are gone
	if (!PatchInfo::SanityCheck())
	{
		// No need for an assert, the function already prints out an error
		DIE();
	}
}