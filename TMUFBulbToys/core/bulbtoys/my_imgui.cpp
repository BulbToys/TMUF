#include <unordered_map>
#include <Windows.h>

#include "my_imgui.h"
#include "io.h"

bool ImGui::BulbToys_ListBox(const char* text, const char* id, int* current_item, const char* const* items, int items_count, int height_in_items)
{
	ImGui::Text(text);
	return ImGui::ListBox(id, current_item, items, items_count, items_count);
}

bool ImGui::BulbToys_SliderFloat(const char* text, const char* id, float* v, float v_min, float v_max, const char* format)
{
	ImGui::Text(text);
	return ImGui::SliderFloat(id, v, v_min, v_max, format);
}

bool ImGui::BulbToys_SliderInt(const char* text, const char* id, int* v, int v_min, int v_max, const char* format)
{
	ImGui::Text(text);
	return ImGui::SliderInt(id, v, v_min, v_max, format);
}

bool ImGui::BulbToys_Menu(const char* menu_name, const char* menu_label)
{
	// Get the ID through the menu name. Some people may want to rename their menus dynamically, so we have a separate label for that
	ImGuiID id = ImGui::GetID(menu_name);
	static std::unordered_map<ImGuiID, bool> menu_map;
	bool& show = menu_map[id];

	// If we have a menu label, show that instead
	if (menu_label)
	{
		ImGui::SeparatorText(menu_label);
	}
	else
	{
		ImGui::SeparatorText(menu_name);
	}
	ImGui::SameLine();

	// Hide and show button, using the menu name ID as their ID as well
	char button[32];
	if (show)
	{
		sprintf_s(button, 32, "hide" "##%u", id);
	}
	else
	{
		sprintf_s(button, 32, "show" "##%u", id);
	}

	// Toggle show/hide
	if (ImGui::Button(button))
	{
		show = !show;
	}

	// Should this menu be shown?
	return show;
}

bool ImGui::BulbToys_InputInt(const char* text, const char* id, int* i, int min, int max)
{
	ImGui::Text(text);
	if (ImGui::InputInt(id, i))
	{
		if (*i < min)
		{
			*i = min;
		}
		else if (*i > max)
		{
			*i = max;
		}

		return true;
	}

	return false;
}

void ImGui::BulbToys_AddyLabel(uintptr_t addy, const char* fmt, ...)
{
	// Format label text
	char name[64];
	va_list va;
	va_start(va, fmt);
	vsprintf_s(name, 64, fmt, va);

	// Append address
	ImGui::Text("%s: %p", name, addy);

	char button[16];
	sprintf_s(button, 16, "copy" "##%08X", addy);

	ImGui::SameLine();
	if (ImGui::Button(button))
	{
		auto mem = GlobalAlloc(GMEM_MOVEABLE, 9);
		if (!mem)
		{
			return;
		}

		auto ptr = GlobalLock(mem);
		if (!ptr)
		{
			GlobalFree(mem);
			return;
		}

		char addy_str[9];
		sprintf_s(addy_str, 9, "%08X", addy);
		memcpy(ptr, addy_str, 9);
		GlobalUnlock(mem);

		OpenClipboard(IO::Get()->Window());
		EmptyClipboard();
		SetClipboardData(CF_TEXT, mem);
		CloseClipboard();

		// The clipboard calls GlobalFree for us, no need to do it ourselves
	}
}

void IWindow::Construct(bool push_width, ImGuiWindowFlags flags, const char* fmt, va_list args)
{
	char buffer[128];
	vsprintf_s(buffer, 128, fmt, args);

	size_t title_len = strlen(buffer) + 1 + 4 + 10;
	char* title = new char[title_len];
	sprintf_s(title, title_len, "%s" "##IWindow_%u", buffer, IWindow::id++);

	this->push_width = push_width;
	this->flags = flags;
	this->title = title;

	IWindow::queue.push_back(this);
}

IWindow::IWindow(bool push_width, ImGuiWindowFlags flags, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	this->Construct(push_width, flags, fmt, va);
	va_end(va);
}

IWindow::~IWindow()
{
	// The window is already erased from the queue and list
	delete this->title;
}

IWindow::IWindow(bool push_width, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	this->Construct(true, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar, fmt, va);
	va_end(va);
}

IWindow::IWindow(ImGuiWindowFlags flags, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	this->Construct(push_width, flags, fmt, va);
	va_end(va);
}

IWindow::IWindow(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	this->Construct(true, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar, fmt, va);
	va_end(va);
}

void IWindow::CloseAll()
{
	auto iter = list.begin();
	while (iter != list.end())
	{
		(*iter)->OpenedRef() = false;
		++iter;
	}
}

void IWindow::DestroyAll()
{
	auto iter = queue.begin();
	while (iter != queue.end())
	{
		auto window = *iter;

		queue.erase(iter);
		delete window;
	}

	auto iter_2 = list.begin();
	while (iter_2 != list.end())
	{
		auto window = *iter_2;

		list.erase(iter_2);
		delete window;
	}
}
