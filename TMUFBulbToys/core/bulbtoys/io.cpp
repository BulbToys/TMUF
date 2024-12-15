#include "io.h"
#include "hook.h"
#include "gui.h"

IO::IO(HWND window, LPVOID keyboard, LPVOID mouse, uint8_t keyboard_im, uint8_t mouse_im)
{
	IO::instance = this;

	this->window = window;
	this->keyboard = keyboard;
	this->mouse = mouse;
	this->keyboard_im = keyboard_im;
	this->mouse_im = mouse_im;
	this->original_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongA(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(IO::WndProc)));

	if (mouse)
	{
		auto Mouse_GetDeviceState = PtrVirtual<9>(reinterpret_cast<uintptr_t>(this->mouse));
		Unprotect _(Mouse_GetDeviceState, 4);

		CREATE_VTABLE_PATCH(Mouse_GetDeviceState, IDIDevice8_GetDeviceState);
		if (!keyboard)
		{
			auto Mouse_GetDeviceData = PtrVirtual<10>(reinterpret_cast<uintptr_t>(this->mouse));
			Unprotect _(Mouse_GetDeviceData, 4);

			// No need to patch twice if we're using a KB&M, they'll always use the same vtable
			CREATE_VTABLE_PATCH(Mouse_GetDeviceData, IDIDevice8_GetDeviceData);
		}
	}
	if (keyboard)
	{
		auto Keyboard_GetDeviceData = PtrVirtual<10>(reinterpret_cast<uintptr_t>(this->keyboard));
		Unprotect _(Keyboard_GetDeviceData, 4);

		CREATE_VTABLE_PATCH(Keyboard_GetDeviceData, IDIDevice8_GetDeviceData);
		if (!mouse)
		{
			auto Keyboard_GetDeviceState = PtrVirtual<9>(reinterpret_cast<uintptr_t>(this->keyboard));
			Unprotect _(Keyboard_GetDeviceState, 4);

			// No need to patch twice if we're using a KB&M, they'll always use the same vtable
			CREATE_VTABLE_PATCH(Keyboard_GetDeviceState, IDIDevice8_GetDeviceState);
		}
	}
}

IO::~IO()
{
	if (this->keyboard)
	{
		if (!this->mouse)
		{
			auto Keyboard_GetDeviceState = PtrVirtual<9>(reinterpret_cast<uintptr_t>(this->keyboard));
			Unprotect _(Keyboard_GetDeviceState, 4);

			Unpatch(Keyboard_GetDeviceState);
		}

		auto Keyboard_GetDeviceData = PtrVirtual<10>(reinterpret_cast<uintptr_t>(this->keyboard));
		Unprotect _(Keyboard_GetDeviceData, 4);

		Unpatch(Keyboard_GetDeviceData);
	}
	if (this->mouse)
	{
		if (!this->keyboard)
		{
			auto Mouse_GetDeviceData = PtrVirtual<10>(reinterpret_cast<uintptr_t>(this->mouse));
			Unprotect _(Mouse_GetDeviceData, 4);

			Unpatch(Mouse_GetDeviceData);
		}

		auto Mouse_GetDeviceState = PtrVirtual<9>(reinterpret_cast<uintptr_t>(this->mouse));
		Unprotect _(Mouse_GetDeviceState, 4);

		Unpatch(Mouse_GetDeviceState);
	}

	SetWindowLongA(this->window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(this->original_wndproc));

	IO::instance = nullptr;
}

void IO::HotkeyDown(int key)
{
	this->key_held[key] = true;

	auto& list = this->on_key_down[key];
	auto iter = list.begin();
	while (iter != list.end())
	{
		(*iter)();
		++iter;
	}
}

void IO::HotkeyUp(int key)
{
	this->key_held[key] = false;

	auto& list = this->on_key_up[key];
	auto iter = list.begin();
	while (iter != list.end())
	{
		(*iter)();
		++iter;
	}
}

void IO::DI8_DeviceState_KeyboardInput(char* keys)
{
	static char old_keys[256] {0};

	for (int i = 0; i < 256; i++)
	{
		char old_key = old_keys[i];
		char key = keys[i];

		if (old_key != key)
		{
			auto vk = IO::DIKToVK(i);

			bool down = (key & 0x80) != 0;
			if (down)
			{
				this->HotkeyDown(vk);
			}
			else
			{
				this->HotkeyUp(vk);
			}

			if (ImGui::GetCurrentContext())
			{
				auto imgui_io = ImGui::GetIO();
				auto im = (ImGuiKey)IO::DIKToIM(i);
				imgui_io.AddKeyEvent(im, down);

				imgui_io.AddKeyEvent(ImGuiMod_Ctrl, ((keys[0x1D] & 0x80) != 0) || ((keys[0x9D] & 0x80) != 0));
				imgui_io.AddKeyEvent(ImGuiMod_Shift, ((keys[0x2A] & 0x80) != 0) || ((keys[0x36] & 0x80) != 0));
				imgui_io.AddKeyEvent(ImGuiMod_Alt, ((keys[0x38] & 0x80) != 0) || ((keys[0xB8] & 0x80) != 0));
				imgui_io.AddKeyEvent(ImGuiMod_Super, ((keys[0xDB] & 0x80) != 0) || ((keys[0xDC] & 0x80) != 0));

				if (down)
				{
					unsigned char kb[256];
					for (int j = 0; j < 256; j++)
					{
						kb[IO::DIKToVK(j)] = keys[j];
					}

					// if either of the two are held, set the "main" one
					kb[VK_SHIFT] = kb[VK_LSHIFT] | kb[VK_RSHIFT];
					kb[VK_CONTROL] = kb[VK_LCONTROL] | kb[VK_RCONTROL];
					kb[VK_MENU] = kb[VK_LMENU] | kb[VK_RMENU];

					// HACK: no other way to find out if capslock is engaged or not
					if ((GetKeyState(VK_CAPITAL) & 1) != 0)
					{
						kb[VK_CAPITAL]++;
					}

					wchar_t buf[256];
					auto len = ToUnicode(vk, MapVirtualKeyA(vk, MAPVK_VK_TO_VSC), kb, buf, 255, (1 << 2));
					if (len > 0)
					{
						for (int j = 0; j < len; j++)
						{
							imgui_io.AddInputCharacterUTF16(buf[j]);
						}
					}
				}
			}
		}
	}

	memcpy(old_keys, keys, 256);
}

HRESULT APIENTRY IO::IDIDevice8_GetDeviceState_(LPVOID device, DWORD cbData, LPVOID lpvData)
{
	const auto result = IO::IDIDevice8_GetDeviceState(device, cbData, lpvData);

	auto this_ = IO::instance;
	if (!this_)
	{
		Error("IO::IDIDevice8_GetDeviceState_ called but no IO instance.");
		DIE();
		return result;
	}

	if (result == S_OK)
	{
		if (device == this_->keyboard && cbData == 256)
		{
			if (this_->KeyboardInputAllowed(IO::InputMethod::DeviceState))
			{
				this_->DI8_DeviceState_KeyboardInput((char*)lpvData);
			}

			if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard)
			{
				memset(lpvData, 0, 256);
			}
		}
		else if (device == this_->mouse && (cbData == 16 || cbData == 20))
		{
			auto di_mouse_state = reinterpret_cast<uintptr_t>(lpvData);

			if (this_->MouseInputAllowed(IO::InputMethod::DeviceState))
			{
				ASSERT(!"Not implemented");
			}

			if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse)
			{
				Write<uint8_t>(di_mouse_state + 0xC, 0);
				Write<uint8_t>(di_mouse_state + 0xD, 0);
			}
		}
	}

	return result;
}

HRESULT APIENTRY IO::IDIDevice8_GetDeviceData_(LPVOID device, DWORD cbObjectData, LPVOID rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	const auto result = IO::IDIDevice8_GetDeviceData(device, cbObjectData, rgdod, pdwInOut, dwFlags);

	auto this_ = IO::instance;
	if (!this_)
	{
		Error("IO::IDIDevice8_GetDeviceState_ called but no IO instance.");
		DIE();
		return result;
	}

	if (result == S_OK)
	{
		if (device == this_->keyboard)
		{
			if (this_->KeyboardInputAllowed(IO::InputMethod::DeviceData))
			{
				ASSERT(!"Not implemented");
			}

			if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard)
			{
				*pdwInOut = 0;
			}
		}
		if (device == this_->mouse)
		{
			if (this_->MouseInputAllowed(IO::InputMethod::DeviceData))
			{
				ASSERT(!"Not implemented");
			}

			if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse)
			{
				*pdwInOut = 0;
			}
		}
	}

	return result;
}

LRESULT CALLBACK IO::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto this_ = IO::instance;
	if (!this_)
	{
		Error("IO::WndProc called but no IO instance.");
		DIE();
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}

	if (this_->KeyboardInputAllowed(IO::InputMethod::WindowProcedure))
	{
		if (uMsg == WM_KEYDOWN)
		{
			this_->HotkeyDown(wParam);
		}

		else if (uMsg == WM_KEYUP)
		{
			this_->HotkeyUp(wParam);
		}
	}

	if (GUI::Get())
	{
		return GUI::WndProc(this_->original_wndproc, hWnd, uMsg, wParam, lParam);
	}
	else
	{
		// If GUI functionality is disabled, we will leak memory every time we create a window
		// Since we can't return nullptr from the new operator (unless it's std::nothrow_t, which has ugly syntax), we can't cancel a window's construction
		// We could also call GUI::Get() before calling the new operator for our window, but that's also pretty ugly and involves code repetition
		// As the queue only gets serviced in the GUI, which in this case doesn't exist, we, unfortunately, MUST service the queue here instead
		// We normally don't service the IWindow queue here as it's unnecessary - windows won't be rendered until the next frame anyways
		// Hence why this is done in the ID3DDevice9::EndScene hook, right before we tell our GUI to render all our ImGui widgets
		IWindow::DestroyAll();

		return CallWindowProcA(this_->original_wndproc, hWnd, uMsg, wParam, lParam);
	}
}

void IO::Init(HWND window, LPVOID keyboard, LPVOID mouse, uint8_t keyboard_im, uint8_t mouse_im)
{
	ASSERT(!IO::instance);
	ASSERT(keyboard_im < IO::InputMethod::IM_MAX);
	ASSERT(mouse_im < IO::InputMethod::IM_MAX);
	new IO(window, keyboard, mouse, keyboard_im, mouse_im);
}

IO* IO::Get()
{
	return IO::instance;
}

void IO::End()
{
	delete this;
}

struct DIKTo
{
	uint8_t vk;
	int im;
};

#define DIKTO_KEY(k) { (""#k)[0], ImGuiKey_##k }

constexpr DIKTo dik_to[256] = {
	{ 0, ImGuiKey_None },
	{ VK_ESCAPE, ImGuiKey_Escape },
	DIKTO_KEY(1),
	DIKTO_KEY(2),
	DIKTO_KEY(3),
	DIKTO_KEY(4),
	DIKTO_KEY(5),
	DIKTO_KEY(6),
	DIKTO_KEY(7),
	DIKTO_KEY(8),
	DIKTO_KEY(9),
	DIKTO_KEY(0),
	{ VK_OEM_MINUS, ImGuiKey_Minus },
	{ VK_OEM_PLUS, ImGuiKey_Equal },
	{ VK_BACK, ImGuiKey_Backspace },
	{ VK_TAB, ImGuiKey_Tab },
	DIKTO_KEY(Q),
	DIKTO_KEY(W),
	DIKTO_KEY(E),
	DIKTO_KEY(R),
	DIKTO_KEY(T),
	DIKTO_KEY(Y),
	DIKTO_KEY(U),
	DIKTO_KEY(I),
	DIKTO_KEY(O),
	DIKTO_KEY(P),
	{ VK_OEM_4, ImGuiKey_LeftBracket },
	{ VK_OEM_6, ImGuiKey_RightBracket },
	{ VK_RETURN, ImGuiKey_Enter },
	{ VK_LCONTROL, ImGuiKey_LeftCtrl },
	DIKTO_KEY(A),
	DIKTO_KEY(S),
	DIKTO_KEY(D),
	DIKTO_KEY(F),
	DIKTO_KEY(G),
	DIKTO_KEY(H),
	DIKTO_KEY(J),
	DIKTO_KEY(K),
	DIKTO_KEY(L),
	{ VK_OEM_1, ImGuiKey_Semicolon },
	{ VK_OEM_7, ImGuiKey_Apostrophe },
	{ VK_OEM_3, ImGuiKey_GraveAccent },
	{ VK_LSHIFT, ImGuiKey_LeftShift },
	{ VK_OEM_5, ImGuiKey_Backslash },
	DIKTO_KEY(Z),
	DIKTO_KEY(X),
	DIKTO_KEY(C),
	DIKTO_KEY(V),
	DIKTO_KEY(B),
	DIKTO_KEY(N),
	DIKTO_KEY(M),
	{ VK_OEM_COMMA, ImGuiKey_Comma },
	{ VK_OEM_PERIOD, ImGuiKey_Period },
	{ VK_OEM_2, ImGuiKey_Slash },
	{ VK_RSHIFT, ImGuiKey_RightShift },
	{ VK_MULTIPLY, ImGuiKey_KeypadMultiply },
	{ VK_LMENU, ImGuiKey_LeftAlt },
	{ VK_SPACE, ImGuiKey_Space },
	{ VK_CAPITAL, ImGuiKey_CapsLock },
	{ VK_F1, ImGuiKey_F1 },
	{ VK_F2, ImGuiKey_F2 },
	{ VK_F3, ImGuiKey_F3 },
	{ VK_F4, ImGuiKey_F4 },
	{ VK_F5, ImGuiKey_F5 },
	{ VK_F6, ImGuiKey_F6 },
	{ VK_F7, ImGuiKey_F7 },
	{ VK_F8, ImGuiKey_F8 },
	{ VK_F9, ImGuiKey_F9 },
	{ VK_F10, ImGuiKey_F10 },
	{ VK_NUMLOCK, ImGuiKey_NumLock },
	{ VK_SCROLL, ImGuiKey_ScrollLock },
	{ VK_NUMPAD7, ImGuiKey_Keypad7 },
	{ VK_NUMPAD8, ImGuiKey_Keypad8 },
	{ VK_NUMPAD9, ImGuiKey_Keypad9 },
	{ VK_SUBTRACT, ImGuiKey_KeypadSubtract },
	{ VK_NUMPAD4, ImGuiKey_Keypad4 },
	{ VK_NUMPAD5, ImGuiKey_Keypad5 },
	{ VK_NUMPAD6, ImGuiKey_Keypad6 },
	{ VK_ADD, ImGuiKey_KeypadAdd },
	{ VK_NUMPAD1, ImGuiKey_Keypad1 },
	{ VK_NUMPAD2, ImGuiKey_Keypad2 },
	{ VK_NUMPAD3, ImGuiKey_Keypad3 },
	{ VK_NUMPAD0, ImGuiKey_Keypad0 },
	{ VK_DECIMAL, ImGuiKey_KeypadDecimal },
	{ 0, 0 },
	{ 0, 0 },
	{ VK_OEM_5, ImGuiKey_Backslash }, // DIK_OEM_102
	{ VK_F11, ImGuiKey_F11 },
	{ VK_F12, ImGuiKey_F12 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ VK_F13, 0 },
	{ VK_F14, 0 },
	{ VK_F15, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ VK_KANA, 0 }, // DIK_KANA
	{ 0, 0 },
	{ 0, 0 },
	{ VK_OEM_2, ImGuiKey_Slash }, // DIK_ABNT_C1
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ VK_CONVERT, 0 },
	{ 0, 0 },
	{ VK_NONCONVERT, 0 },
	{ 0, 0 },
	{ 0, 0 }, // DIK_YEN
	{ VK_DECIMAL, ImGuiKey_KeypadDecimal }, // DIK_ABNT_C2
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 }, // DIK_NUMPADEQUALS
	{ 0, 0 },
	{ 0, 0 },
	{ VK_MEDIA_PREV_TRACK, 0 },
	{ 0, 0 }, // DIK_AT
	{ 0, 0 }, // DIK_COLON
	{ 0, 0 }, // DIK_UNDERLINE
	{ VK_KANJI, 0 },
	{ 0, 0 }, // DIK_STOP
	{ 0, 0 }, // DIK_AX
	{ 0, 0 }, // DIK_UNLABELED
	{ 0, 0 },
	{ VK_MEDIA_NEXT_TRACK, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ VK_RETURN, ImGuiKey_KeypadEnter },
	{ VK_RCONTROL, ImGuiKey_RightCtrl },
	{ 0, 0 },
	{ 0, 0 },
	{ VK_VOLUME_MUTE, 0 },
	{ 0, 0 }, // DIK_CALCULATOR
	{ VK_MEDIA_PLAY_PAUSE, 0 },
	{ 0, 0 },
	{ VK_MEDIA_STOP, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ VK_VOLUME_DOWN, 0 },
	{ 0, 0 },
	{ VK_VOLUME_UP, 0 },
	{ 0, 0 },
	{ VK_BROWSER_HOME, 0 },
	{ 0, 0 }, // DIK_NUMPADCOMMA
	{ 0, 0 },
	{ VK_DIVIDE, ImGuiKey_KeypadDivide },
	{ 0, 0 },
	{ 0, 0 }, // DIK_SYSRQ
	{ VK_RMENU, ImGuiKey_RightAlt },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ VK_PAUSE, ImGuiKey_Pause },
	{ 0, 0 },
	{ VK_HOME, ImGuiKey_Home },
	{ VK_UP, ImGuiKey_UpArrow },
	{ VK_PRIOR, ImGuiKey_PageUp },
	{ 0, 0 },
	{ VK_LEFT, ImGuiKey_LeftArrow },
	{ 0, 0 },
	{ VK_RIGHT, ImGuiKey_RightArrow },
	{ 0, 0 },
	{ VK_END, ImGuiKey_End },
	{ VK_DOWN, ImGuiKey_DownArrow },
	{ VK_NEXT, ImGuiKey_PageDown },
	{ VK_INSERT, ImGuiKey_Insert },
	{ VK_DELETE, ImGuiKey_Delete },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ VK_LWIN, ImGuiKey_LeftSuper },
	{ VK_RWIN, ImGuiKey_RightSuper },
	{ VK_APPS, ImGuiKey_Menu },
	{ 0, 0 }, // DIK_POWER
	{ VK_SLEEP, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 }, // DIK_WAKE
	{ 0, 0 },
	{ VK_BROWSER_SEARCH, 0 },
	{ VK_BROWSER_FAVORITES, 0 },
	{ VK_BROWSER_REFRESH, 0 },
	{ VK_BROWSER_STOP, 0 },
	{ VK_BROWSER_FORWARD, 0 },
	{ VK_BROWSER_BACK, 0 },
	{ 0, 0 }, // DIK_MYCOMPUTER
	{ VK_LAUNCH_MAIL, 0 },
	{ VK_LAUNCH_MEDIA_SELECT, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
};

uint8_t IO::DIKToVK(uint8_t dik)
{
	return dik_to[dik].vk;
}

int IO::DIKToIM(uint8_t dik)
{
	return dik_to[dik].im;
}
