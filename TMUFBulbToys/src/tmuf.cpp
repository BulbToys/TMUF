#include "tmuf.h"

#include "../core/bulbtoys.h"

#include <string>

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

	// account for the last row
	if (last_string_empty)
	{
		slices.back().str.append("$");
	}

	return slices;
}

// i'd rather fucking kill myself than learn to regex this shit
// NO UNICODE SUPPORT - if you get a bunch of question marks after this, good luck and godspeed o7
void ImGui::TMUF_Text(const char* text)
{
	auto slices = ImGui::TMUF_Parse(text);

	ImGui::TMUF_TextEx(slices, text);
}

void ImGui::TMUF_TextEx(std::vector<ImGui::TMUF_TextSlice>& slices, const char* tooltip)
{
	// god that was exhausting - build this motherfucker already
	bool after_first = false;
	for (auto& slice : slices)
	{
		// join text elements together
		if (after_first)
		{
			ImGui::SameLine(0, 0);
		}

		// we set alpha to 0, which means this text has no specific color
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

		after_first = true;
	}
}
