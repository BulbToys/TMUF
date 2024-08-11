#pragma once
#include <d3d9.h>

#include "settings.h"

class IO
{
public:
	using OnKeyDownFn = void();
	using OnKeyUpFn = void();
private:
	std::vector<OnKeyDownFn*> on_key_down[256];
	std::vector<OnKeyUpFn*> on_key_up[256];
public:
	template <StringLiteral name, uint8_t default_key>
	class Hotkey
	{
		Settings::Key<"Hotkeys", name, default_key> key;
		OnKeyDownFn* OnKeyDown;
		OnKeyUpFn* OnKeyUp;
	public:
		Hotkey(OnKeyDownFn* OnKeyDown = nullptr, OnKeyUpFn* OnKeyUp = nullptr) : OnKeyDown(OnKeyDown), OnKeyUp(OnKeyUp)
		{
			auto io = IO::Get();
			if (io)
			{
				if (OnKeyDown)
				{
					io->on_key_down[key.Get()].push_back(OnKeyDown);
				}
				if (OnKeyUp)
				{
					io->on_key_up[key.Get()].push_back(OnKeyUp);
				}
			}
		}
		~Hotkey()
		{
			if (OnKeyDown)
			{
				auto& list = IO::Get()->on_key_down[key.Get()];
				list.erase(std::remove(list.begin(), list.end(), OnKeyDown), list.end());
			}
			if (OnKeyUp)
			{
				auto& list = IO::Get()->on_key_up[key.Get()];
				list.erase(std::remove(list.begin(), list.end(), OnKeyUp), list.end());
			}
		}
	};
private:
	static inline IO* instance = nullptr;
	bool done = false;
	HWND window = 0;
	bool key_held[256] { false };

	IO(HWND window);
	~IO();

	WNDPROC original_wndproc = nullptr;
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	static IO* Get(HWND window = 0);
	void End();

	inline void Detach() { done = true; }
	inline bool Done() { return done; }

	inline HWND Window() { return window; }
	inline bool KeyHeld(char vk) { return key_held[vk]; }
};