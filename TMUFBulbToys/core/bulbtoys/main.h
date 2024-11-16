#pragma once

#include <Windows.h>

#define BULBTOYS_SLEEP 200

namespace BulbToys
{
	using GetD3D9DeviceFn = LPVOID();
	using GetDI8DeviceFn = LPVOID();

	struct SetupParams
	{
		// Handle to the module instance, given by DllMain
		HMODULE instance = 0;

		// Pointer to the function that gets called indefinitely by the BulbToys setup until a D3D9 device is returned
		// If this is nullptr, IO and GUI functionality will not be available (only Modules, Hooks and Settings)
		// If IO functionality is not available in threaded setup mode, BulbToys immediately terminates
		GetD3D9DeviceFn* GetD3D9Device = nullptr;

		// Pointer to the function that gets called indefinitely by the BulbToys setup until a DI8 keyboard device is returned
		// If this is nullptr, BulbToys might not properly handle ImGui keyboard input blocking if your game uses DirectInput
		GetDI8DeviceFn* GetDI8KeyboardDevice = nullptr;

		// Pointer to the function that gets called indefinitely by the BulbToys setup until a DI8 mouse device is returned
		// If this is nullptr, BulbToys might not properly handle ImGui mouse input blocking if your game uses DirectInput
		GetDI8DeviceFn* GetDI8MouseDevice = nullptr;

		// Path to the settings file, absolute or relative to the attached-to executable (NOT the DLL)
		// If nullptr, the settings functionality will not be used
		const char* settings_file = nullptr;
	};

	// Threaded setup mode - call this in DllMain once the game loads your DLL
	void Setup(SetupParams& params);

	// Non-threaded setup mode - call this on the main thread of your game
	bool Init(SetupParams& params, bool thread = false);
	void End();
}