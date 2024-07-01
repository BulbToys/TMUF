#include "../../core/bulbtoys.h"
#include "../tmuf.h"

namespace unbind
{
	struct UnbindPanel : IPanel
	{
		virtual bool Draw() override final
		{
			auto trackmania = TMUF::GetTrackMania();
			if (trackmania)
			{
				if (ImGui::BulbToys_Menu("Unbind Control"))
				{
					auto input_bindings = TMUF::CGameCtnApp_GetCurrentInputBindings(trackmania, 0);
					if (input_bindings)
					{
						for (int i = 0; i < 47; i++)
						{
							if (i == 11 || i == 12 || i == 45 || i == 46)
							{
								continue;
							}

							auto input_action_desc = Read<TMUF::SInputActionDesc*>(0xCD5F58 + (i * 4));

							char button[64];
							MYPRINTF(button, 64, "%s" "##%p", input_action_desc->name.psz, input_action_desc);
							if (ImGui::Button(button))
							{
								auto action = TMUF::CInputBindingsConfig_FindAction(input_bindings, input_action_desc);

								TMUF::CInputBindingsConfig_ClearBindings(input_bindings, action, Read<int>(0xD75044));
							}
						}
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
			return new UnbindPanel();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(unbind);