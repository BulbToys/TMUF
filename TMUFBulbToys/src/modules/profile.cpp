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

				// TODO 1: all of this for nickname changing is horrible, nuke me as soon as you figure out where the control gets its max length
				// TODO 2: also, implement CopyToClipboardUnicode please for the love of god for the old nickname or osmething
				uintptr_t menu = Read<uintptr_t>(trackmania + 0x194);
				if (menu)
				{
					constexpr size_t NICK_MAX = 76;

					auto old_nick = reinterpret_cast<TMUF::CFastStringInt*>(menu + 0x3B8);
					auto new_nick = reinterpret_cast<TMUF::CFastStringInt*>(menu + 0x3B0);

					char im_old_nick[NICK_MAX]{ 0 };
					WideStringToString(old_nick->pwsz, NICK_MAX, im_old_nick, NICK_MAX);
					auto im_old_len = strlen(im_old_nick);

					char im_new_nick[NICK_MAX] { 0 };
					WideStringToString(new_nick->pwsz, NICK_MAX, im_new_nick, NICK_MAX);
					auto im_new_len = strlen(im_new_nick);

					ImGui::Text("Old Nickname [%2d/75]:", im_old_len);
					if (im_old_len)
					{
						ImGui::SameLine();
					}
					ImGui::TMUF_Text(im_old_nick);

					ImGui::Text("New Nickname [%2d/75]:", im_new_len);
					if (im_new_len)
					{
						ImGui::SameLine();
					}
					ImGui::TMUF_Text(im_new_nick);
					if (ImGui::InputText("##NickName", im_new_nick, IM_ARRAYSIZE(im_new_nick)))
					{
						wchar_t im_new_nick_w[NICK_MAX];
						StringToWideString(im_new_nick, NICK_MAX, im_new_nick_w, NICK_MAX);
						new_nick->SetString(lstrlenW(im_new_nick_w), im_new_nick_w);
					}
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

	void Init()
	{
		Unprotect _(0x643682, 2);
		Patch<uint16_t>(0x643682, 0xC033);
	}

	void End()
	{
		Unprotect _(0x643682, 2);
		Unpatch(0x643682);
	}
}

MODULE(profile);