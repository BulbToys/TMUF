#pragma once
#include <d3d9.h>

namespace BulbToys
{
	// IDirect3DDevice9* GetDevice() { return the_device; }
	using GetDeviceFn = IDirect3DDevice9*();

	struct SetupParams
	{
		// Handle to the module instance, given by DllMain
		HMODULE instance = 0;

		// Pointer to the function that gets called indefinitely by the created thread until a device is returned
		// In case the device is NOT nullptr, this function is unused
		// Can be nullptr, but if the device is also nullptr, IO and GUI functionality will not be available
		// If IO functionality is not available in threaded setup mode, BulbToys immediately terminates
		BulbToys::GetDeviceFn* GetDevice = nullptr;

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