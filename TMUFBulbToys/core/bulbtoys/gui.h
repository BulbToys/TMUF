#pragma once

#include "my_imgui.h"
#include "io.h"
#include "modules.h"

#include "../imgui/imgui_memory_editor.h"

class GUI
{
	static inline GUI* instance = nullptr;

	WNDPROC original_wndproc = nullptr;
	LPVOID device = nullptr;

	static void CreateMainWindow();
	IO::Hotkey<"CreateMainWindow", VK_F9> create_main_window{ GUI::CreateMainWindow };

	static void CloseAllWindows();
	IO::Hotkey<"CloseAllWindows", VK_F8> close_all_windows{ GUI::CloseAllWindows };

	class Overlay
	{
		bool enabled = true;
		std::vector<IPanel*> panels;
	public:
		Overlay() { this->panels = Modules::Panels(Module::DrawType::Overlay); }
		~Overlay();

		inline bool& EnabledRef() { return enabled; }

		void Render();
	};
	Overlay overlay;

	class FrameCalc
	{
	public:
		enum Type : int
		{
			None,
			Early,
			Late,
			_MAX
		};
	private:
		int type = Type::None;
		int fps = 0;
		int fps_limit = 0;
	public:
		inline int& TypeRef() { return type; }
		inline int FPS() { return fps; }
		inline int& FPSLimitRef() { return fps_limit; }

		void Perform();
	};
	FrameCalc frame_calc;

	GUI(LPVOID device, HWND window);
	~GUI();

	void Render();

	enum struct PatchMode
	{
		Patch,
		Unpatch,
		Repatch,
	};
	void PatchVTables(PatchMode pm);

	using DxPresentFn = long(__stdcall)(LPVOID, LPVOID, LPVOID, HWND, LPVOID);
	static DxPresentFn ID3DDevice9_Present_;
	static inline DxPresentFn* ID3DDevice9_Present = nullptr;

	using DxResetFn = long(__stdcall)(LPVOID, LPVOID);
	static DxResetFn ID3DDevice9_Reset_;
	static inline DxResetFn* ID3DDevice9_Reset = nullptr;

	using DxBeginStateBlockFn = long(__stdcall)(LPVOID);
	static DxBeginStateBlockFn ID3DDevice9_BeginStateBlock_;
	static inline DxBeginStateBlockFn* ID3DDevice9_BeginStateBlock = nullptr;

	static LRESULT CALLBACK WndProc(WNDPROC original_wndproc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend class IO; // Sadly necessary so the IO can call our WndProc (friend function will not work)
public:
	static void Init(LPVOID device, HWND window);
	static GUI* Get();
	void End();

	inline bool& Overlay_EnabledRef() { return overlay.EnabledRef(); }

	inline int& FrameCalc_TypeRef() { return frame_calc.TypeRef(); }
	inline int FrameCalc_FPS() { return frame_calc.FPS(); }
	inline int& FrameCalc_FPSLimitRef() { return frame_calc.FPSLimitRef(); }
};

class MainWindow : public IWindow
{
	std::vector<IPanel*> panels;

	bool confirm_close = false;
	char input_addr[9]{ 0 };
	bool use_vprot = true;
public:
	MainWindow() : IWindow("BulbToys Main Window") { this->panels = Modules::Panels(Module::DrawType::MainWindow); }
	~MainWindow() override final;

	virtual bool Draw() override final;
};

class MemoryWindow : public IWindow
{
	MemoryEditor editor;

	char* memory;
	size_t size;
	bool malloc;
public:
	MemoryWindow(uintptr_t address, size_t size, bool use_vprot);
	~MemoryWindow() override final;

	virtual bool Draw() override final { editor.DrawContents(memory, size); return true; }

	static uint8_t VProtRead(const uint8_t* address, size_t offset);
	static void VProtWrite(uint8_t* address, size_t offset, uint8_t data);
};

class StopwatchWindow : public IWindow
{
	Stopwatch stopwatch;
public:
	StopwatchWindow() : IWindow("BulbToys Stopwatch") {}
	~StopwatchWindow() override final {}

	virtual bool Draw() override final;
};