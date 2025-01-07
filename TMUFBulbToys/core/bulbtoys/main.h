#pragma once

#include <Windows.h>

#define BULBTOYS_SLEEP 200

namespace BulbToys
{
	using GetD3D9DeviceFn = LPVOID();
	using GetDI8DeviceFn = LPVOID();

	enum InputMethod : unsigned char
	{
		// WinAPI Window Procedure - processing WM_MOUSE/WM_KEY and similar messages, the preferred method of getting input
		WindowProcedure = 1 << 0,

		// DirectInput8::GetDeviceState - immediate data from DI8, typically (but not necessarily) used for mouse input
		DeviceState = 1 << 1,

		// DirectInput8::GetDeviceData - buffered data from DI8, typically (but not necessarily) used for keyboard input
		DeviceData = 1 << 2,

		// Sanity check - do not use!
		_MAX = 1 << 3,
	};

	struct SetupParams
	{
		// Handle to the module instance, given by DllMain
		HMODULE instance = 0;

		// Pointer to the function that gets called indefinitely by the BulbToys setup until a D3D9 device is returned
		// If this is nullptr, IO and GUI functionality will not be available (only Modules, Hooks and Settings)
		// If IO functionality is not available in threaded setup mode, BulbToys immediately terminates
		GetD3D9DeviceFn* GetD3D9Device = nullptr;

		// Pointer to the function that gets called indefinitely by the BulbToys setup until a DI8 keyboard device is returned
		// If this is nullptr, BulbToys might not properly handle ImGui keyboard input if your game uses DirectInput
		GetDI8DeviceFn* GetDI8KeyboardDevice = nullptr;

		// Pointer to the function that gets called indefinitely by the BulbToys setup until a DI8 mouse device is returned
		// If this is nullptr, BulbToys might not properly handle ImGui mouse input if your game uses DirectInput
		GetDI8DeviceFn* GetDI8MouseDevice = nullptr;

		// Preferred default/initial method of reading keyboard input, used by GUI/ImGui and Settings::Hotkeys
		// InputMethod is bitwise, thus it is allowed to use multiple methods at once if necessary (beware of multiplicated input)
		// DeviceState and DeviceData will not work without a DI8 keyboard/mouse device
		// Can be freely changed at any point by calling IO::Get()->SetKeyboardInputMethod(new_keyboard_im);
		unsigned char keyboard_input_method = InputMethod::WindowProcedure;

		// Preferred default/initial method of reading mouse input, used by GUI/ImGui
		// InputMethod is bitwise, thus it is allowed to use multiple methods at once if necessary (beware of multiplicated input)
		// DeviceState and DeviceData will not work without a DI8 keyboard/mouse device
		// Can be freely changed at any point by calling IO::Get()->SetMouseInputMethod(new_mouse_im);
		unsigned char mouse_input_method = InputMethod::WindowProcedure;

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