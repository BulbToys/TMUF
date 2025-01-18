#include <intrin.h>

#include "gui.h"
#include "utils.h"
#include "io.h"
#include "hook.h"
#include "version.h"

#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx9.h"

void GUI::CreateMainWindow()
{
	new MainWindow();
}

void GUI::CloseAllWindows()
{
	IWindow::CloseAll();
}

GUI::Overlay::~Overlay()
{
	auto iter = panels.begin();
	while (iter != panels.end())
	{
		auto panel = *iter;

		panels.erase(iter);
		delete panel;
	}
}

void GUI::Overlay::Render()
{
	RECT rect;
	GetClientRect(IO::Get()->Window(), &rect);
	float w = rect.right - rect.left;
	float h = rect.bottom - rect.top;

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(w, h));
	if (ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground))
	{
		if (ImGui::BulbToys_Overlay_BeginTable("Watermark"))
		{
			auto frame_calc = GUI::Get()->FrameCalc();
			if (frame_calc->Type() == GUI::FrameCalc::Type::None)
			{
				ImGui::Text("Powered by BulbToys %d | Built on %s", GIT_REV_COUNT + 1, BulbToys::GetBuildDateTime());
			}
			else
			{
				ImGui::Text("%d FPS | Powered by BulbToys %d | Built on %s", frame_calc->FPS(), GIT_REV_COUNT + 1, BulbToys::GetBuildDateTime());
			}

			ImGui::BulbToys_Overlay_EndTable();
		}

		// Overlay module panels
		auto iter = panels.begin();
		while (iter != panels.end())
		{
			auto panel = *iter;

			if (!panel->Draw())
			{
				// A panel has ended the overlay and requested to end any further rendering immediately
				return;
			}

			++iter;
		}

		ImGui::End();
	}
}

GUI::FrameCalc::FrameCalc()
{
	Settings::String<"GUI", "FrameCalcType", "none", 7> setting;
	auto value = setting.Get();

	// Only psychopaths use mixed case here
	if (!strcmp(value, "early") || !strcmp(value, "EARLY"))
	{ 
		this->type = Type::Early;
	}
	if (!strcmp(value, "late") || !strcmp(value, "LATE"))
	{
		this->type = Type::Late;
	}
}

void GUI::FrameCalc::Perform()
{
	static Stopwatch render_time;

	if (render_time.Running())
	{
		render_time.Stop();

		auto elapsed = render_time.Elapsed();

		static Stopwatch fps_update_time;
		if (!fps_update_time.Running())
		{
			fps_update_time.Start();
		}

		// Update visual FPS counter every second
		if (fps_update_time.Elapsed() > 1000000)
		{
			this->fps = 1000000.0f / (float)elapsed;

			fps_update_time.Reset();
			fps_update_time.Start();
		}

		render_time.Reset();
		render_time.Start();

		// Limit our FPS if the option is enabled
		if (this->fps_limit > 0)
		{
			int delay = ((1000.0f / this->fps_limit) - ((float)elapsed / 1000.0f));
			if (delay > 0)
			{
				Sleep(delay);
			}
		}
	}
	else
	{
		render_time.Start();
	}
}

GUI::GUI(LPVOID device, HWND window)
{
	GUI::instance = this;

	this->device = device;

	ImGui::CreateContext();
	
	// Setup ImGui style
	{
		// Enemymouse style from ImThemes, orange-ified
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.6000000238418579f;
		style.WindowPadding = ImVec2(8.0f, 8.0f);
		style.WindowRounding = 3.0f;
		style.WindowBorderSize = 1.0f;
		style.WindowMinSize = ImVec2(32.0f, 32.0f);
		style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Left;
		style.ChildRounding = 3.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupRounding = 0.0f;
		style.PopupBorderSize = 1.0f;
		style.FramePadding = ImVec2(4.0f, 3.0f);
		style.FrameRounding = 3.0f;
		style.FrameBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(8.0f, 4.0f);
		style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
		style.CellPadding = ImVec2(4.0f, 2.0f);
		style.IndentSpacing = 0.0f;//21.0f;
		style.ColumnsMinSpacing = 6.0f;
		style.ScrollbarSize = 14.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabMinSize = 20.0f;
		style.GrabRounding = 1.0f;
		style.TabRounding = 4.0f;
		style.TabBorderSize = 0.0f;
		style.TabMinWidthForCloseButton = 0.0f;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 0.4635193347930908f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.407843142747879f, 0.1890431791543961f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8299999833106995f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.239215686917305f, 0.1738281697034836f, 0.1568627655506134f, 0.6000000238418579f);
		style.Colors[ImGuiCol_Border] = ImVec4(1.0f, 0.4635193347930908f, 0.0f, 0.6499999761581421f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.800000011920929f, 0.6064462065696716f, 0.4392156600952148f, 0.1802574992179871f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.800000011920929f, 0.6078431606292725f, 0.4392156898975372f, 0.274678111076355f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.8588235378265381f, 0.6337120532989502f, 0.4392156600952148f, 0.4721029996871948f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2078431397676468f, 0.1703019291162491f, 0.1369319409132004f, 0.729411780834198f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.4635193347930908f, 0.0f, 0.2700000107288361f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5400000214576721f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.2000000029802322f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2980392277240753f, 0.2565818727016449f, 0.219730868935585f, 0.7098039388656616f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.4392156898975372f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.7411764860153198f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.6784313917160034f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.3607843220233917f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.7607843279838562f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.6470588445663452f, 0.3044982552528381f, 0.0f, 0.4588235318660736f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.4747404456138611f, 0.007843136787414551f, 0.4313725531101227f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.6196078658103943f);
		style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.3294117748737335f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.4196078479290009f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.5411764979362488f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.4980392158031464f, 0.2343713790178299f, 0.0f, 0.3294117748737335f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.4980392158031464f, 0.2343713790178299f, 0.0f, 0.4705882370471954f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.6980392336845398f, 0.3284890353679657f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.5411764979362488f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.7411764860153198f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.250980406999588f, 0.1587516069412231f, 0.07677046954631805f, 0.8627451062202454f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.9764705896377563f, 0.5960784554481506f, 0.2588235437870026f, 0.501960813999176f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.3921568691730499f, 0.2447927296161652f, 0.1138023808598518f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1441742479801178f, 0.1450980454683304f, 0.06657439470291138f, 0.9725490212440491f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.4235294163227081f, 0.2696519196033478f, 0.1328719705343246f, 1.0f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.6470588445663452f, 0.3044982552528381f, 0.0f, 0.4588235318660736f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
		style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
		style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.0f, 0.470588207244873f, 0.0f, 0.2196078449487686f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.9764705896377563f, 0.5973702073097229f, 0.2603921294212341f, 1.0f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.09803921729326248f, 0.06689734756946564f, 0.03921568393707275f, 0.5098039507865906f);
	}

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init((IDirect3DDevice9*)device);

	GUI::PatchVTables(PatchMode::Patch);
}

GUI::~GUI()
{
	GUI::PatchVTables(PatchMode::Unpatch);

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();
	
	IWindow::DestroyAll();

	GUI::instance = nullptr;
}

void GUI::Render()
{
	auto& list = IWindow::List();
	auto iter = list.begin();
	while (iter != list.end())
	{
		auto window = *iter;

		if (ImGui::Begin(window->Title(), &window->OpenedRef(), window->Flags()))
		{
			bool push_width = window->PushWidth();
			if (push_width)
			{
				ImGui::PushItemWidth(ImGui::GetWindowWidth() - 16.0f /* GetStyle().WindowPadding */);
			}

			if (!window->Draw())
			{
				// A window has ended its own rendering and requested to end any further rendering immediately
				return;
			}

			if (push_width)
			{
				ImGui::PopItemWidth();
			}
			ImGui::End();
		}

		// If we've closed the window with X, deallocate
		if (!window->OpenedRef())
		{
			list.erase(iter);
			delete window;
		}
		else
		{
			++iter;
		}
	}

	if (overlay.Enabled())
	{
		overlay.Render();
	}
}

void GUI::PatchVTables(PatchMode pm)
{
	auto device = reinterpret_cast<uintptr_t>(this->device);

	auto Present = PtrVirtual<17>(device);
	auto Reset = PtrVirtual<16>(device);
	auto BeginStateBlock = PtrVirtual<60>(device);

	// Usually not necessary, but some things (like the Steam overlay) mess with the protection
	Unprotect u1(Present, 4);
	Unprotect u2(Reset, 4);
	Unprotect u3(BeginStateBlock, 4);

	if (pm == PatchMode::Unpatch || pm == PatchMode::Repatch)
	{
		Unpatch(BeginStateBlock);

		Unpatch(Reset);
		Unpatch(Present);
	}

	if (pm == PatchMode::Patch || pm == PatchMode::Repatch)
	{
		CREATE_VTABLE_PATCH(Present, ID3DDevice9_Present);
		CREATE_VTABLE_PATCH(Reset, ID3DDevice9_Reset);

		CREATE_VTABLE_PATCH(BeginStateBlock, ID3DDevice9_BeginStateBlock);
	}
}

HRESULT APIENTRY GUI::ID3DDevice9_Present_(LPVOID device, LPVOID pSourceRect, LPVOID pDestRect, HWND hDestWindowOverride, LPVOID pDirtyRegion)
{
	auto this_ = GUI::instance;
	if (!this_)
	{
		Error("GUI::ID3DDevice9_Present_ called but no GUI instance.");
		DIE();
		return GUI::ID3DDevice9_Present(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	}

	if (device != this_->device)
	{
		return GUI::ID3DDevice9_Present(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	}

	auto ret_addr = _ReturnAddress();
	static auto my_ret_addr = ret_addr;
	if (ret_addr != my_ret_addr)
	{
		return GUI::ID3DDevice9_Present(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	}

	auto frame_calc_type = this_->frame_calc.Type();
	if (frame_calc_type == GUI::FrameCalc::Type::Early)
	{
		this_->frame_calc.Perform();
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Push all windows from the queue into the list
	auto& queue = IWindow::Queue();
	auto iter = queue.begin();
	while (iter != queue.end())
	{
		auto window = *iter;

		queue.erase(iter);
		IWindow::List().push_back(window);
	}

	// TODO: If called multiple times per frame, FPS counter will be incorrect. FIXME FIXME FIXME
	this_->Render();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	auto result = GUI::ID3DDevice9_Present(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

	if (frame_calc_type == GUI::FrameCalc::Type::Late)
	{
		this_->frame_calc.Perform();
	}

	return result;
}

HRESULT APIENTRY GUI::ID3DDevice9_Reset_(LPVOID device, LPVOID params)
{
	auto this_ = GUI::instance;
	if (!this_)
	{
		Error("GUI::ID3DDevice9_Reset_ called but no GUI instance.");
		DIE();
		return GUI::ID3DDevice9_Reset(device, params);
	}

	if (device != this_->device)
	{
		return GUI::ID3DDevice9_Reset(device, params);
	}

	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = GUI::ID3DDevice9_Reset(device, params);
	ImGui_ImplDX9_CreateDeviceObjects();

	return result;
}

HRESULT APIENTRY GUI::ID3DDevice9_BeginStateBlock_(LPVOID device)
{
	auto result = GUI::ID3DDevice9_BeginStateBlock(device);

	auto this_ = GUI::instance;
	if (!this_)
	{
		Error("GUI::ID3DDevice9_BeginStateBlock_ called but no GUI instance.");
		DIE();
		return result;
	}

	if (device != this_->device)
	{
		return result;
	}

	// ID3DDevice9::BeginStateBlock resets the vtable ?????????? (thanks ficool2 :3)
	this_->PatchVTables(PatchMode::Repatch);

	return result;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK GUI::WndProc(WNDPROC original_wndproc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto io = IO::Get();
	auto& imgui_io = ImGui::GetIO();

	// Keyboard input messages
	if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
	{
		// Are we allowed to read keyboard input from WndProc?
		if (io->KeyboardInputAllowed(IO::InputMethod::WindowProcedure))
		{
			// Pass keyboard input to ImGui
			auto result = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
			if (result)
			{
				return result;
			}
		}

		// If ImGui is requesting to use our keyboard, block the input further down the window procedure chain
		if (imgui_io.WantCaptureKeyboard)
		{
			return 0;
		}
	}

	// Mouse input messages
	else if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
	{
		// Are we allowed to read mouse input from WndProc?
		if (io->MouseInputAllowed(IO::InputMethod::WindowProcedure))
		{
			// Pass mouse input to ImGui
			auto result = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
			if (result)
			{
				return result;
			}
		}

		// If ImGui is requesting to use our mouse, block the input further down the window procedure chain
		if (imgui_io.WantCaptureMouse)
		{
			return 0;
		}
	}

	// All other messages
	else
	{
		// Pass miscellaneous messages to ImGui
		auto result = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		if (result)
		{
			return result;
		}
	}

	// In all the above cases, keep calling our own window procedure until ImGui's window procedure returns TRUE
	return CallWindowProcA(original_wndproc, hWnd, uMsg, wParam, lParam);
}

void GUI::Init(LPVOID device, HWND window)
{
	ASSERT(!GUI::instance);
	new GUI(device, window);
}

GUI* GUI::Get()
{
	return GUI::instance;
}

void GUI::End()
{
	delete this;
}

MemoryWindow::MemoryWindow(uintptr_t address, size_t size, bool use_vprot) :
	IWindow(false, ImGuiWindowFlags_NoSavedSettings, "%s%s 0x%08X", ((use_vprot) ? "[VP] " : ""), ((address == -1) ? "Playground" : "Memory Editor"), address)
{
	bool malloc = address == -1;

	this->malloc = malloc;
	if (malloc)
	{
		this->memory = new char[size] {0};
	}
	else
	{
		this->memory = reinterpret_cast<char*>(address);
	}
	this->size = size;

	if (use_vprot)
	{
		this->editor.ReadFn = MemoryWindow::VProtRead;
		this->editor.WriteFn = MemoryWindow::VProtWrite;
	}
}

MemoryWindow::~MemoryWindow()
{
	if (malloc)
	{
		delete[] this->memory;
	}
}

uint8_t MemoryWindow::VProtRead(const uint8_t* address, size_t offset)
{
	uint8_t data = 0;

	uint8_t* addy = const_cast<uint8_t*>(address);
	addy += offset;

	Unprotect _(addy, 1);
	data = *addy;

	return data;
}

void MemoryWindow::VProtWrite(uint8_t* address, size_t offset, uint8_t data)
{
	address += offset;

	Unprotect _(address, 1);
	*address = data;
}

MainWindow::~MainWindow()
{
	auto iter = panels.begin();
	while (iter != panels.end())
	{
		auto panel = *iter;

		panels.erase(iter);
		delete panel;
	}
}

bool MainWindow::Draw()
{
	if (ImGui::BulbToys_Menu("[Main]"))
	{
		auto gui = GUI::Get();
		auto frame_calc = gui->FrameCalc();
		auto overlay = gui->Overlay();

		auto io = IO::Get();

		ImGui::Text("Dear ImGui version: " IMGUI_VERSION);

		static bool enable_overlay = overlay->Enabled();
		if (ImGui::Checkbox("Enable overlay", &enable_overlay))
		{
			overlay->SetEnabled(enable_overlay);
		}

		// Check GUI::FrameCalc::Type for more information about these
		static int frame_calc_type = frame_calc->Type();
		const char* frame_calc_methods[] = { "None", "Early", "Late" };
		ImGui::Text("Frame calc method:");
		if (ImGui::Combo("##FrameCalcMethod", &frame_calc_type, frame_calc_methods, IM_ARRAYSIZE(frame_calc_methods)))
		{
			frame_calc->SetType(frame_calc_type);
		}

		ImGui::BeginDisabled(!frame_calc->Type());

		static int fps_limit = 0;
		static bool fps_limit_enabled = false;
		if (ImGui::Checkbox("FPS Limit:", &fps_limit_enabled))
		{
			if (fps_limit_enabled)
			{
				frame_calc->SetFPSLimit(fps_limit);
			}
			else
			{
				frame_calc->SetFPSLimit(0);
			}
		}

		if (ImGui::InputInt("##FPSLimit", &fps_limit))
		{
			if (fps_limit_enabled)
			{
				frame_calc->SetFPSLimit(fps_limit);
			}
		}

		ImGui::EndDisabled();

		ImGui::Separator();
		ImGui::BeginDisabled(!io->DetachAllowed());

		if (ImGui::Button("Detach"))
		{
			if (this->confirm_close)
			{
				io->Detach();
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Confirm", &this->confirm_close);

		ImGui::EndDisabled();
		ImGui::Separator();

		ImGui::Text("Memory address:");
		ImGui::InputText("##MemEdit_InputAddr", this->input_addr, IM_ARRAYSIZE(this->input_addr), ImGuiInputTextFlags_CharsHexadecimal);

		// !!! THIS TANKS PERFORMANCE - USE SPARINGLY !!!
		// With this option, VirtualProtect is called every time a byte is read or written to
		// Keep in mind bytes are being read every frame as they're rendered, thus VirtualProtect gets called twice per frame for every byte you see on screen
		// If you have, say, 4 large memory windows, showing 1024 bytes each, running at 100fps - this equates to (((819200 VP calls per second))) !!!
		// However, for practical intents and purposes, the performance impact is *just* negligible enough to leave this option on by default
		// TODO: Rewrite imgui_memory_editor.h to better support VirtualProtect, calling for the entire range of visible bytes when changed (scrolling/resizing)
		//       This would lower VirtualProtect usage to anywhere from 0 to FPS*2 times per second (maybe even lower?)

		ImGui::Checkbox("Use VirtualProtect for new window", &this->use_vprot);

		if (ImGui::Button("New Memory Editor"))
		{
			uintptr_t addr;
			if (sscanf_s(this->input_addr, "%IX", &addr) == 1)
			{
				new MemoryWindow(addr, 0x10000, this->use_vprot);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("+ 0000"))
		{
			uintptr_t addr;
			if (sscanf_s(this->input_addr, "%IX", &addr) == 1 && addr < 0xFFFF)
			{
				new MemoryWindow(addr * 0x10000, 0x10000, this->use_vprot);
			}
		}

		if (ImGui::Button("New Playground"))
		{
			new MemoryWindow(-1, 0x10000, false);
		}

		ImGui::Separator();

		if (ImGui::Button("New Stopwatch"))
		{
			new StopwatchWindow();
		}
	}

	if (ImGui::BulbToys_Menu("[Modules]"))
	{
		if (!Module::First())
		{
			ImGui::Text("No modules loaded.");
		}

		for (auto iter = Module::First(); iter; iter = iter->Next())
		{
			ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(iter), "- %s (%s%s%s)", iter->Name(),
				(iter->InitFunc()) ? "I" : "", (iter->PanelFunc()) ? "P" : "", (iter->EndFunc()) ? "E" : "");
		}
	}

	if (ImGui::BulbToys_Menu("[VKMap]"))
	{
		auto settings = Settings::Get();
		if (!settings)
		{
			ImGui::Text("Settings are uninitialized.");
		}
		else
		{
			for (int i = 0; i < 256; i++)
			{
				auto str = settings->VKToStr(i);

				if (strcmp(str, INVALID_KEY))
				{
					ImGui::Text("%-3d = %s", i, str);
				}
			}
		}
	}

	if (ImGui::BulbToys_Menu("[Windows]"))
	{
		if (ImGui::Button("Close all"))
		{
			IWindow::CloseAll();
		}

		auto& list = IWindow::List();
		int i = 0;

		for (auto iter = list.begin(); iter != list.end(); ++iter)
		{
			auto window = *iter;

			char close[32]{ 0 };
			sprintf_s(close, 32, "X" "##Windows_X_%u", i++);

			if (ImGui::Button(close))
			{
				window->OpenedRef() = false;
			}
			ImGui::SameLine();
			ImGui::Text("%s", window->Title());
		}
	}

	// Main window module panels
	auto iter = panels.begin();
	while (iter != panels.end())
	{
		auto panel = *iter;

		if (!panel->Draw())
		{
			// A module has ended the main window and requested to end any further rendering immediately
			return false;
		}

		++iter;
	}

	return true;
}

bool StopwatchWindow::Draw()
{
	long long elapsed = stopwatch.Elapsed();

	int microseconds = elapsed;

	int seconds = microseconds / 1000000;
	microseconds %= 1000000;

	int minutes = seconds / 60;
	seconds %= 60;

	int hours = minutes / 60;
	minutes %= 60;

	ImGui::Text("%02d:%02d:%02d.%06d", hours, minutes, seconds, microseconds);

	ImGui::Separator();

	bool running = stopwatch.Running();

	//ImGui::BeginDisabled(running);
	if (ImGui::Button("Start"))
	{
		stopwatch.Start();
	}
	//ImGui::EndDisabled();
	ImGui::SameLine();
	//ImGui::BeginDisabled(!running);
	if (ImGui::Button("Stop"))
	{
		stopwatch.Stop();
	}
	ImGui::SameLine();
	//ImGui::EndDisabled();
	if (ImGui::Button("Reset"))
	{
		stopwatch.Reset();
	}

	ImGui::Separator();

	ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(&this->stopwatch), "Address");

	return true;
}
