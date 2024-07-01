#pragma once
#include <vector>

#include "../imgui/imgui.h"

namespace ImGui
{
	// ListBox with text at the top
	bool BulbToys_ListBox(const char* text, const char* id, int* current_item, const char* const* items, int items_count, int height_in_items = -1);

	// SliderFloat with text at the top
	bool BulbToys_SliderFloat(const char* text, const char* id, float* v, float v_min, float v_max, const char* format = "%.3f");

	// SliderInt with text at the top
	bool BulbToys_SliderInt(const char* text, const char* id, int* v, int v_min, int v_max, const char* format = "%d");

	// Dead simple drop-down menu implementation. !! DO NOT USE RECURSIVELY !!
	bool BulbToys_Menu(const char* menu_name, const char* menu_label = nullptr);

	// InputInt with text at the top. !! min AND max ARE INCLUSIVE !!
	bool BulbToys_InputInt(const char* text, const char* id, int* i, int min = (-2147483647 - 1), int max = 2147483647);

	// A custom text label with an address right after it, as well as a "Copy to Clipboard" button
	void BulbToys_AddyLabel(uintptr_t addy, const char* fmt, ...);
}

// Blank slate for drawing ImGui widgets - primarily used by MainWindow and the overlay
// If you want your panels (and windows) to share data, use static variables, otherwise use member variables
struct IPanel
{
	IPanel() {};
	virtual ~IPanel() {};
	virtual bool Draw() = 0;
};

// Window wrapper for the ImGui "panel"
class IWindow : public IPanel
{
	static inline uint32_t id = 0;
	static inline std::vector<IWindow*> list;
	static inline std::vector<IWindow*> queue;

	char* title;
	bool opened = true; // Windows always start opened and are deallocated when they're closed
	ImGuiWindowFlags flags;
	bool push_width;

	void Construct(bool push_width, ImGuiWindowFlags flags, const char* fmt, va_list args);
public:
	IWindow(bool push_width, ImGuiWindowFlags flags, const char* fmt = "", ...);
	virtual ~IWindow();

	// Implies default: flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar
	IWindow(bool push_width, const char* fmt = "", ...);

	// Implies default: push_width = true
	IWindow(ImGuiWindowFlags flags, const char* fmt = "", ...);

	// Implies defaults: flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar, push_width = true
	IWindow(const char* fmt = "", ...);

	// The GUI will call this function, wrapping it around an ImGui::Begin() beforehand.
	// This is where you make calls to ImGui, adding various options/widgets, etc...
	// 
	// Sometimes, you might want a widget to do something which might invalidate the graphics context.
	// This, of course, will likely result in a crash, as we're in the middle of rendering our window.
	// To alleviate this, ImGui::End() your window AS SOON AS your widget is activated, make it do what it needs to do, and return false, IN THAT ORDER.
	//
	// Return "true" to continue drawing other windows, "false" otherwise.
	// If you are returning "true", the GUI automatically ImGui::End()s the window for you.
	// If you are returning "false", YOU MUST ImGui::End() the window yourself
	virtual bool Draw() = 0;

	virtual char* Title() final { return title; }
	virtual bool& OpenedRef() final { return opened; }
	virtual ImGuiWindowFlags Flags() final { return flags; }
	virtual bool PushWidth() final { return push_width; }

	// Don't modify these lists, the GUI already does it for you
	static auto& List() { return list; }
	static auto& Queue() { return queue; }

	static void CloseAll();
	static void DestroyAll();
};