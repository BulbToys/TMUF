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
				uintptr_t unk = Read<uintptr_t>(TMUF::BulbToys_GetTrackMania() + 0x168);
				if (unk)
				{
					ImGui::Text("Trail Color:");
					uintptr_t trail_color_ptr = unk + 0x23C;
					TMUF::GmVec3 trail_color = Read<TMUF::GmVec3>(trail_color_ptr);
					if (ImGui::SliderFloat3("##TrailColor", reinterpret_cast<float*>(&trail_color), 0.0f, 1.0f))
					{
						Write<TMUF::GmVec3>(trail_color_ptr, trail_color);
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