#include "../../core/bulbtoys.h"
#include "../tmuf.h"

namespace server
{
	struct ServerPanel : IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Server"))
			{
				ImGui::Text("todo :3");
			}

			if (ImGui::BulbToys_Menu("Server [SECRET]"))
			{
				auto auth_users = reinterpret_cast<TMUF::CFastBuffer<TMUF::CIPCRemoteControl_SAuthParams*>*>(0xD7B3E0);
				for (int i = 0; i < auth_users->size; i++)
				{
					auto auth_user = auth_users->pElems[i];

					ImGui::Text("%s:", auth_user->_Name.psz);

					char id[16];
					MYPRINTF(id, 16, "##AuthUser%d", i);

					auto& password = auth_user->_Password;

					char buffer[16];
					ImGui::TMUF_InputFastString(password, id, buffer, IM_ARRAYSIZE(buffer));
				}
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new ServerPanel();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(server);