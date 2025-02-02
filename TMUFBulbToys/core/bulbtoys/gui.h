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

	class FrameCalc
	{
	public:
		enum Type : int
		{
			// Do not perform framerate calculations
			None,

			// Perform framerate calculations before presenting a frame (ID3DD9::Present) - default for old versions of MangoHud
			// Ideal for stable framerates
			Early,

			// Perform framerate calculations after presenting a frame (ID3DD9::Present) - default for MangoHud, DXVK and libstrangle
			// Ideal for low latency
			Late,

			// Sanity check - do not use!
			_MAX
		};
	private:
		int type = Type::None;
		int fps = 0;
		int fps_limit = 0;
	public:
		FrameCalc();

		inline auto Type() { return type; }
		inline void SetType(auto type) { this->type = type; }

		inline auto FPS() { return fps; }

		inline auto FPSLimit() { return type; }
		inline void SetFPSLimit(auto fps_limit) { this->fps_limit = fps_limit; }

		void Perform();
	};
	FrameCalc frame_calc;

	class Overlay
	{
		bool enabled = true;
		std::vector<IPanel*> panels;
	public:
		Overlay() { this->panels = Modules::Panels(Module::DrawType::Overlay); }
		~Overlay();

		inline bool Enabled() { return enabled; }
		inline void SetEnabled(bool enabled) { this->enabled = enabled; }

		void Render();
	};
	Overlay overlay;

	class TextureLoader
	{
		static constexpr size_t max_size = 1024 * 1024 * 32;

		struct ImageBuffer : public IFile<ImageBuffer>
		{
			char data[max_size] { 0 };

			bool Validate() override final { return true; }
		};
		ImageBuffer image_buffer;

		HMODULE library = nullptr;
		DWORD lasterr_library = (1 << 29);

		using D3DXCreateTextureFromFileInMemoryFn = HRESULT(WINAPI)(void* pDevice, void* pSrcData, UINT srcDataSize, ImTextureID* ppTexture);
		D3DXCreateTextureFromFileInMemoryFn* CreateTexture = nullptr;
		DWORD lasterr_CreateTexture = (1 << 29);
	public:
		TextureLoader();
		~TextureLoader();

		bool Online();

		ImTextureID Load(const char* filename);
		ImTextureID LoadDialog(const char* title = "Load Image");

		void Unload(ImTextureID texture);
	};
	TextureLoader texture_loader;

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

	inline auto Device() { return device; }

	inline auto FrameCalc() { return &frame_calc; }
	inline auto Overlay() { return &overlay; }
	inline auto TextureLoader() { return &texture_loader; }
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