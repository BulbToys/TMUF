#include "../../core/bulbtoys.h"
#include "../tmuf.h"

namespace ghosts
{
	const char* skin_methods[] = { "Vanilla (CRC32)", "Random", "Min/Max (Inclusive)" };

	namespace SkinMethod
	{
		enum
		{
			VANILLA = 0,
			RANDOM,
			MIN_MAX
		};
	}
	int skin_method = SkinMethod::VANILLA;

	uint32_t min_index = 0;
	uint32_t max_index = 0;

	struct GhostsPanel : IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Ghosts"))
			{
				ImGui::Text("Medal ghost skin method:");
				ImGui::Combo("##GhostsPanelSkinMethod", &skin_method, skin_methods, IM_ARRAYSIZE(skin_methods));

				ImGui::BeginDisabled(skin_method != SkinMethod::MIN_MAX);

				ImGui::Text("Minimum/Maximum index:");
				if (ImGui::InputScalar("##GhostsPanelMin", ImGuiDataType_U32, &min_index) && min_index > max_index)
				{
					max_index = min_index;
				}
				if (ImGui::InputScalar("##GhostsPanelMax", ImGuiDataType_U32, &max_index) && max_index < min_index)
				{
					min_index = max_index;
				}

				ImGui::EndDisabled();
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new GhostsPanel();
		}

		return nullptr;
	}

	HOOK(0x4FC640, void, __fastcall, CTrackManiaRace_Ghosts_SetRandomGhostSkin, uintptr_t c_tm_race, uintptr_t edx, void* c_game_ctn_ghost, uint32_t index);

	void Init()
	{
		CREATE_HOOK(CTrackManiaRace_Ghosts_SetRandomGhostSkin);
	}

	void End()
	{
		Hooks::Destroy(0x4FC640);
	}

	void __fastcall CTrackManiaRace_Ghosts_SetRandomGhostSkin_(uintptr_t c_tm_race, uintptr_t edx, void* c_game_ctn_ghost, uint32_t index)
	{
		if (skin_method == SkinMethod::RANDOM)
		{
			index = -1;
		}
		else if (skin_method == SkinMethod::MIN_MAX)
		{
			index = index % (max_index - min_index + 1) + min_index;
		}

		CTrackManiaRace_Ghosts_SetRandomGhostSkin(c_tm_race, edx, c_game_ctn_ghost, index);
	}
}

MODULE(ghosts);