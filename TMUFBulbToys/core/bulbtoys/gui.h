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
	IO::Hotkey<"CreateMainWindow", VK_F9> create_main_window { GUI::CreateMainWindow };

	static void CloseAllWindows();
	IO::Hotkey<"CloseAllWindows", VK_F8> close_all_windows { GUI::CloseAllWindows };

	class Overlay
	{
		std::vector<IPanel*> panels;
	public:
		Overlay() { this->panels = Modules::Panels(Module::DrawType::Overlay); }
		~Overlay();

		void Render();
	};
	Overlay overlay;

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

	using DxEndSceneFn = long(__stdcall)(LPVOID);
	static DxEndSceneFn ID3DDevice9_EndScene_;
	static inline DxEndSceneFn* ID3DDevice9_EndScene = nullptr;

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