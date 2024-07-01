#include "io.h"
#include "gui.h"

IO::IO(HWND window)
{
	this->window = window;
	this->original_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(IO::WndProc)));
}

IO::~IO()
{
	SetWindowLongPtr(this->window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(this->original_wndproc));
}

LRESULT CALLBACK IO::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto this_ = IO::instance;
	if (!this_)
	{
		Error("IO::WndProc called but no IO instance.");
		return 0L;// DIE();
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

IO* IO::Get(HWND window)
{
	if (!IO::instance && window)
	{
		IO::instance = new IO(window);
	}
	return IO::instance;
}

void IO::End()
{
	IO::instance = nullptr;
	delete this;
}