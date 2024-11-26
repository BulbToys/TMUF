#include "../../core/bulbtoys.h"
#include "../tmuf.h"

namespace profile
{
	struct ProfilePanel : IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Profile"))
			{
				uintptr_t trackmania = TMUF::BulbToys_GetTrackMania();

				uintptr_t profile = Read<uintptr_t>(trackmania + 0x168);
				if (profile)
				{
					uintptr_t trail_color_ptr = profile + 0x23C;
					TMUF::GmVec3 trail_color = Read<TMUF::GmVec3>(trail_color_ptr);

					ImGui::Text("Trail Color:");
					if (ImGui::ColorEdit3("##TrailColor", &trail_color.x))
					{
						Write<TMUF::GmVec3>(trail_color_ptr, trail_color);
					}
				}
				else
				{
					ImGui::Text("No profile loaded");
				}

				ImGui::Separator();

				static int nick_max_len = 45;
				if (ImGui::BulbToys_SliderInt("EntryNickname MaxLength (default: 45)", "##NickMaxLen", &nick_max_len, 0, 75))
				{
					uintptr_t entry_nick = TMUF::BulbToys_GetControlFromFrame("FrameProfile2", "EntryNickName");
					Write<int>(entry_nick + 0x150, nick_max_len);
				}
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new ProfilePanel();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(profile);