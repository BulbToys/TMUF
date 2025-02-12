#include "tmuf.h"

#include "../core/bulbtoys.h"

#include <string>

uintptr_t TMUF::BulbToys_GetEngine(_Engine e)
{
	uintptr_t mw_engine_main = Read<uintptr_t>(0xD74960);
	if (!mw_engine_main)
	{
		return 0;
	}

	int index = (int)e;
	auto engines = reinterpret_cast<TMUF::CFastBuffer<uintptr_t>*>(mw_engine_main + 0x20);
	if (index >= engines->size)
	{
		return 0;
	}

	return engines->pElems[index];
}

const char* TMUF::BulbToys_GetClassName(uintptr_t vtable)
{
	static mINI::INIStructure ini;
	static bool loaded = false;
	if (!loaded)
	{
		// vtables.ini must be next to TMUF_BulbToys.ini (ie. next to TmForever.exe)
		mINI::INIFile file("vtables.ini");
		file.read(ini);

		loaded = true;
	}

	char address[9];
	MYPRINTF(address, 9, "%08X", vtable);

	const char* name = ini["VTables"][address].c_str();
	if (strlen(name) == 0)
	{
		return "(unknown class)";
	}

	return name;
}

LPVOID TMUF::BulbToys_GetDI8Device(int index)
{
	LPVOID device = nullptr;

	auto input_port = Read<uintptr_t>(0xD72DE8);
	if (input_port)
	{
		auto input_port_vtbl = Read<uintptr_t>(input_port);
		if (input_port_vtbl == 0xBBEE64)
		{
			auto input_device_array = reinterpret_cast<TMUF::CFastArray<uintptr_t>*>(input_port + 0x2C);

			device = Read<LPVOID>(input_device_array->pElems[index] + 0x3C);
		}
	}

	return device;
}

uintptr_t TMUF::BulbToys_GetControlFromFrame(const char* frame, const char* control)
{
	auto trackmania = TMUF::BulbToys_GetTrackMania();
	if (!trackmania)
	{
		return 0;
	}

	auto menu = Read<uintptr_t>(trackmania + 0x194);
	if (!menu)
	{
		return 0;
	}

	auto game_menu = Read<uintptr_t>(menu + 0x788);
	if (!game_menu)
	{
		return 0;
	}
	auto frames = reinterpret_cast<TMUF::CFastBuffer<uintptr_t>*>(game_menu + 0x68);

	CMwId frame_mwid;
	TMUF::CMwId_CreateFromLocalName(&frame_mwid, frame);

	// CControlContainer* ccc = CFastBuffer<4>::GetNodFromId(frames, &frame_mwid);
	uintptr_t container = reinterpret_cast<uintptr_t(__thiscall*)(TMUF::CFastBuffer<uintptr_t>*, CMwId*)>(0x57AF90)(frames, &frame_mwid);
	if (!container)
	{
		return 0;
	}

	CMwId control_mwid;
	TMUF::CMwId_CreateFromLocalName(&control_mwid, control);

	// CControlContainer::GetChildFromId(ccc, control_mwid, 1);
	return reinterpret_cast<uintptr_t(__thiscall*)(uintptr_t, CMwId*, int)>(0x767620)(container, &control_mwid, 1);
}

bool TMUF::BulbToys_MwIsKindOf(uintptr_t mw, TMUF::_MwClassId cid)
{
	using MwIsKindOfFn = bool(__thiscall*)(uintptr_t, TMUF::_MwClassId);
	auto MwIsKindOf = reinterpret_cast<MwIsKindOfFn>(Virtual<4>(mw));
	return MwIsKindOf(mw, cid);
}

std::vector<ImGui::TMUF_TextSlice> ImGui::TMUF_Parse(const char* text)
{
	std::stringstream stream(text);
	std::string buffer;

	// we will split our string into slices, based on what color they should be in
	std::vector<ImGui::TMUF_TextSlice> slices;

	// things we need to remember when reading the stream
	bool link = false;
	bool bracket = false;
	bool capitalize = false;
	bool last_string_empty = false;

	// now we can remove any and all leftover instances of the non-supported tags, since we're lazy
	bool after_first = false;
	while (std::getline(stream, buffer, '$'))
	{
		ImVec4 color = { .0f, .0f, .0f, .0f };

		// if it's our first time in this loop
		if (!after_first)
		{
			// accounting for an empty vector of slices is just gonna result in uglier and more complicated code everywhere
			// fuck it, we're fine with empty slices in this one case, it's not worth the micro-optimization
			// also, we don't set 'last_string_empty' here, because it wasn't caused by a '$'
			slices.push_back({ buffer, color });
			after_first = true;
		}

		// if it's not our first time in this loop, we found a '$' - what was next to it?
		else
		{
			char c = buffer[0];

			// nothing (ie, another '$')
			if (!c)
			{
				// mark last string as empty
				if (!last_string_empty)
				{
					last_string_empty = true;
				}
				else
				{
					// the first one escapes the other, so the other '$' is actually shown
					slices.back().str.append("$");

					// stop adding additional '$'s
					last_string_empty = false;
				}

				continue;
			}

			// if last string was empty already, we ran into a '$$'
			// we don't care what was next to it, because we don't want to format anything with it anyways
			if (last_string_empty)
			{
				// the first one escapes the other, so the other '$' is actually shown
				slices.back().str.append("$");

				// stop adding additional '$'s
				last_string_empty = false;
			}
			else
			{
				// default the color to be the same as the last slice
				color = slices.back().clr;

				// hex digit - beginning of a color
				if ((c >= '0' && c <= '9') ||
					(c >= 'A' && c <= 'F') ||
					(c >= 'a' && c <= 'f'))
				{
					// trackmania's color formatting is fucking stupid
					// it only checks if the first character is hexadecimal before initiating color mode
					auto get_color = +[](char c)
					{
						if (c >= '0' && c <= '9')
						{
							c -= '0';
						}
						else if ((c >= 'A' && c <= 'F'))
						{
							c -= 'A';
							c += 10;
						}
						else if ((c >= 'a' && c <= 'f'))
						{
							c -= 'a';
							c += 10;
						}
						else // everything else gets treated as black
						{
							c = 0;
						}

						return float(c / 15.f);
					};

					// set R color value
					color.x = get_color(c);

					// if we find a '$' anywhere (in this case, a null character), bail custom coloring
					c = buffer[1];
					if (!c)
					{
						continue;
					}

					// set G color value
					color.y = get_color(c);

					c = buffer[2];
					if (!c)
					{
						continue;
					}

					// set B color value
					color.z = get_color(c);

					// we set alpha to 1 to indicate a custom color is being passed (0 indicates default)
					color.w = 1.f;

					// erase the color characters
					buffer.erase(0, 3);
				}

				// toggle link formatting
				else if (c == 'l' || c == 'L' || c == 'h' || c == 'H' || c == 'p' || c == 'P')
				{
					link = !link;

					if (link)
					{
						// we just started a link, check for brackets
						c = buffer[1];
						if (!c)
						{
							continue;
						}

						// even though we're not adding any text, get this - other tags STILL affect the output
						bracket = (c == '[');
					}
					else
					{
						// we just ended a link, don't check for brackets, just remove the tag
						buffer.erase(0, 1);

						// also, end bracket checking
						bracket = false;
					}
				}

				// toggle capitalization
				else if (c == 't' || c == 'T')
				{
					capitalize = !capitalize;
					buffer.erase(0, 1);
				}

				// reset ALL formatting
				else if (c == 'z' || c == 'Z')
				{
					capitalize = false;
					link = false;
					bracket = false;
					color = { .0f, .0f, .0f, .0f };
					buffer.erase(0, 1);
				}

				// reset COLOR formatting
				else if (c == 'g' || c == 'G')
				{
					color = { .0f, .0f, .0f, .0f };
					buffer.erase(0, 1);
				}

				// anything else, just discard the first character
				else
				{
					buffer.erase(0, 1);
				}
			}

			// we have an open bracket - don't add any text until the bracket is closed
			if (bracket)
			{
				auto i = buffer.find(']');
				if (i == std::string::npos)
				{
					// empty the entire string
					buffer = "";
				}
				else
				{
					// remove everything inbetween brackets
					buffer.erase(0, i + 1);
				}
			}

			if (capitalize)
			{
				std::transform(buffer.begin(), buffer.end(), buffer.begin(), toupper);
			}

			// why does ImVec4 not have operator== overloaded ;w;
			if (color.x == slices.back().clr.x &&
				color.y == slices.back().clr.y &&
				color.z == slices.back().clr.z &&
				color.w == slices.back().clr.w)
			{
				// if the last color was the same, simply append the text we got now to the last slice
				slices.back().str.append(buffer);
			}

			// if not, add a new slice
			else //if (buffer.length() > 0)
			{
				// we're fine with empty strings, since the color was different this time
				slices.push_back({ buffer, color });
			}
		}
	}

	// account for the last hanging '$', if any
	if (last_string_empty)
	{
		slices.back().str.append("$");
	}

	// god that was exhausting
	return slices;
}

void ImGui::TMUF_Text(const char* text)
{
	auto slices = ImGui::TMUF_Parse(text);

	ImGui::TMUF_TextEx(slices, text);
}

void ImGui::TMUF_TextEx(std::vector<ImGui::TMUF_TextSlice>& slices, const char* tooltip)
{
	bool after_first = false;
	for (auto& slice : slices)
	{
		// join text elements together
		if (after_first)
		{
			ImGui::SameLine(0, 0);
		}
		after_first = true;

		// we've set alpha to 0, which means this text has no specific color
		// there's no good answer here - in-game, sometimes it's white, blue, yellow, dark gray etc... so we just use the ImGui default
		if (slice.clr.w == .0f)
		{
			ImGui::Text("%s", slice.str.c_str());
		}
		else
		{
			ImGui::TextColored(slice.clr, "%s", slice.str.c_str());
		}

		if (tooltip)
		{
			// you should be able to see the original unformatted text if you hover over
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip("%s", tooltip);
			}
		}
	}
}

void ImGui::TMUF_InputFastString(TMUF::CFastString& fast_string, const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags,
	ImGuiInputTextCallback callback, void* user_data)
{
	// copy fast string to our text box
	MYPRINTF(buf, buf_size, "%s", fast_string.psz);

	// then invoke the text box
	ImGui::InputText(label, buf, buf_size, flags, callback, user_data);

	// finally, copy the text from our text box if changed
	if (strcmp(buf, fast_string.psz))
	{
		fast_string.SetString(strlen(buf), buf);
	}
}