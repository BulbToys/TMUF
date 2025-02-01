#include "../../core/bulbtoys.h"
#include "../../core/bulbtoys/gui.h"

namespace image_test
{
	ImTextureID texture = nullptr;

	struct ImageTestPanel : IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Image Test"))
			{
				ImGui::BeginDisabled(texture);
				if (ImGui::Button("Load"))
				{
					texture = GUI::Get()->TextureLoader()->LoadDialog();
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::BeginDisabled(!texture);
				if (ImGui::Button("Unload"))
				{
					GUI::Get()->TextureLoader()->Unload(texture);
					texture = nullptr;
				}
				ImGui::EndDisabled();

				ImGui::Separator();

				if (texture)
				{
					ImGui::Image(texture, { 256, 256 });
				}
				else
				{
					ImGui::Text("No image loaded!");
				}
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new ImageTestPanel();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(image_test);