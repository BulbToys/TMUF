#include "io.h"
#include "hook.h"
#include "gui.h"

IO::IO(HWND window, LPVOID keyboard, LPVOID mouse)
{
	this->window = window;
	this->keyboard = keyboard;
	this->mouse = mouse;
	this->original_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(IO::WndProc)));

	if (mouse)
	{
		CREATE_VTABLE_PATCH(PtrVirtual<9>(reinterpret_cast<uintptr_t>(this->mouse)), IDIDevice8_GetDeviceState);
		if (!keyboard)
		{
			// No need to patch twice if we're using a KB&M, they'll always use the same vtable
			CREATE_VTABLE_PATCH(PtrVirtual<10>(reinterpret_cast<uintptr_t>(this->mouse)), IDIDevice8_GetDeviceData);
		}
	}
	if (keyboard)
	{
		CREATE_VTABLE_PATCH(PtrVirtual<10>(reinterpret_cast<uintptr_t>(this->keyboard)), IDIDevice8_GetDeviceData);
		if (!mouse)
		{
			// No need to patch twice if we're using a KB&M, they'll always use the same vtable
			CREATE_VTABLE_PATCH(PtrVirtual<9>(reinterpret_cast<uintptr_t>(this->keyboard)), IDIDevice8_GetDeviceState);
		}
	}
}

IO::~IO()
{
	if (this->keyboard)
	{
		if (!this->mouse)
		{
			Unpatch(PtrVirtual<9>(reinterpret_cast<uintptr_t>(this->keyboard)));
		}
		Unpatch(PtrVirtual<10>(reinterpret_cast<uintptr_t>(this->keyboard)));
	}
	if (this->mouse)
	{
		if (!this->keyboard)
		{
			Unpatch(PtrVirtual<10>(reinterpret_cast<uintptr_t>(this->mouse)));
		}
		Unpatch(PtrVirtual<9>(reinterpret_cast<uintptr_t>(this->mouse)));
	}

	SetWindowLongPtr(this->window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(this->original_wndproc));
}

LRESULT CALLBACK IO::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto this_ = IO::instance;
	if (!this_)
	{
		Error("IO::WndProc called but no IO instance.");
		return 0L;
	}

	if (uMsg == WM_KEYDOWN)
	{
		this_->key_held[wParam] = true;

		auto& list = this_->on_key_down[wParam];
		auto iter = list.begin();
		while (iter != list.end())
		{
			(*iter)();
			++iter;
		}
	}

	else if (uMsg == WM_KEYUP)
	{
		this_->key_held[wParam] = false;

		auto& list = this_->on_key_up[wParam];
		auto iter = list.begin();
		while (iter != list.end())
		{
			(*iter)();
			++iter;
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

		return CallWindowProc(this_->original_wndproc, hWnd, uMsg, wParam, lParam);
	}
}

HRESULT __stdcall IO::IDIDevice8_GetDeviceState_(LPVOID device, DWORD cbData, LPVOID lpvData)
{
	const auto result = IO::IDIDevice8_GetDeviceState(device, cbData, lpvData);

	auto this_ = IO::instance;
	if (!this_)
	{
		Error("IO::IDIDevice8_GetDeviceState_ called but no IO instance.");
		return result;
	}

	if (result == S_OK)
	{
		auto& io = ImGui::GetIO();
		if (device == this_->mouse && io.WantCaptureMouse && (cbData == 16 || cbData == 20))
		{
			auto di_mouse_state = reinterpret_cast<uintptr_t>(lpvData);
			Write<uint8_t>(di_mouse_state + 0xC, 0);
			Write<uint8_t>(di_mouse_state + 0xD, 0);
		}
		if (device == this_->keyboard && io.WantCaptureKeyboard && cbData == 256)
		{
			memset(lpvData, 0, 256);
		}
	}

	return result;
}

HRESULT __stdcall IO::IDIDevice8_GetDeviceData_(LPVOID device, DWORD cbObjectData, LPVOID rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	const auto result = IO::IDIDevice8_GetDeviceData(device, cbObjectData, rgdod, pdwInOut, dwFlags);

	auto this_ = IO::instance;
	if (!this_)
	{
		Error("IO::IDIDevice8_GetDeviceState_ called but no IO instance.");
		return result;
	}

	if (result == S_OK)
	{
		auto& io = ImGui::GetIO();
		if (device == this_->keyboard && io.WantCaptureKeyboard)
		{
			*pdwInOut = 0;
		}
		if (device == this_->mouse && io.WantCaptureMouse)
		{
			*pdwInOut = 0;
		}
	}

	return result;
}

IO* IO::Get(HWND window, LPVOID keyboard, LPVOID mouse)
{
	if (!IO::instance && window)
	{
		IO::instance = new IO(window, keyboard, mouse);
	}
	return IO::instance;
}

void IO::End()
{
	IO::instance = nullptr;
	delete this;
}