#include <unordered_map>
#include <Windows.h>

#include "my_imgui.h"
#include "io.h"

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
		CopyToClipboard<9>("%08X", addy);
	}
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

bool ImGui::BulbToys_Overlay_BeginTable(const char* str_id)
{
	if (!ImGui::BeginTable(str_id, 1, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX))
	{
		return false;
	}

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32({ .0f, .0f, .0f, .6f }));

	ImGui::Dummy({ 0, 0 });
	ImGui::SameLine(11, -1);

	ImGui::BeginGroup();

	return true;
}

void ImGui::BulbToys_Overlay_EndTable()
{
	ImGui::EndGroup();

	ImGui::EndTable();
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
