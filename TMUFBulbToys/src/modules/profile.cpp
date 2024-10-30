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
				uintptr_t profile = Read<uintptr_t>(TMUF::BulbToys_GetTrackMania() + 0x168);
				if (profile)
				{
					uintptr_t trail_color_ptr = profile + 0x23C;
					TMUF::GmVec3 trail_color = Read<TMUF::GmVec3>(trail_color_ptr);

					ImGui::Text("Trail Color:");
					if (ImGui::ColorEdit3("##TrailColor", &trail_color.x))
					{
						Write<TMUF::GmVec3>(trail_color_ptr, trail_color);
					}

					constexpr size_t NICK_MAX = 76;
					auto ign = reinterpret_cast<TMUF::CFastStringInt*>(profile + 0xF8);
					char nickname[NICK_MAX] { 0 };
					WideStringToString(ign->pwsz, NICK_MAX, nickname, NICK_MAX);
					int len = strlen(nickname);

					ImGui::Text("Nickname [%2d/75]:", len);
					if (len)
					{
						ImGui::SameLine();
					}
					ImGui::TMUF_Text(nickname);
					if (ImGui::InputText("##NickName", nickname, IM_ARRAYSIZE(nickname)))
					{
						wchar_t new_nickname[NICK_MAX];
						StringToWideString(nickname, NICK_MAX, new_nickname, NICK_MAX);
						ign->SetString(lstrlenW(new_nickname), new_nickname);
					}
				}
				else
				{
					ImGui::Text("No profile loaded");
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