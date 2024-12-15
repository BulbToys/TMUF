#pragma once

#include "my_imgui.h"

class Module
{
public:
	// The drawing order is always according to the enum, from top to bottom
	enum struct DrawType
	{
		// Inside the main window
		MainWindow,

		// Inside the overlay
		Overlay,
	};

	using InitFn = void();
	using PanelFn = IPanel*(DrawType);
	using EndFn = void();
private:
	static inline Module* first = nullptr;
	static inline Module* last = nullptr;

	Module* next;
	Module* prev;

	const char* name;
	InitFn* Init;
	PanelFn* Panel;
	EndFn* End;
public:
	// DO NOT USE - Create modules through the macro below instead
	Module(const char* name, InitFn* Init, PanelFn* Panel, EndFn* End);
	Module(const Module&) = delete;
	Module(Module&&) = delete;

	inline static Module* First() { return first; }
	inline static Module* Last() { return last; }

	inline Module* Next() { return next; }
	inline Module* Prev() { return prev; }

	inline const char* Name() { return name; }
	inline InitFn* InitFunc() { return Init; }
	inline PanelFn* PanelFunc() { return Panel; }
	inline EndFn* EndFunc() { return End; }
};

#define MODULE(name)			static Module name##_##(#name, &name::Init, &name::Panel, &name::End)
#define MODULE_NO_PANEL(name)	static Module name##_##(#name, &name::Init, nullptr, &name::End)
#define MODULE_PANEL_ONLY(name)	static Module name##_##(#name, nullptr, &name::Panel, nullptr)