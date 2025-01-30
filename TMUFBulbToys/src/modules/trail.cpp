#include "../../core/bulbtoys.h"
#include "../tmuf.h"

// massive shoutout to jailman for making this possible :3 meow
namespace trail
{
	const char* trail_methods[] = { "Vanilla", "Forced", "Hue Wheel" };

	enum TrailMethod : int
	{
		Vanilla = 0,
		Forced,
		HueWheel
	};
	int trail_method = TrailMethod::Vanilla;
	TMUF::GmVec3 forced_color;

	float increment = 0.001f;

	struct TrailPanel : IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Trail"))
			{
				ImGui::Text("Medal ghost skin method:");
				ImGui::Combo("##TrailPanelTrailMethod", &trail_method, trail_methods, IM_ARRAYSIZE(trail_methods));

				ImGui::BeginDisabled(trail_method != TrailMethod::Forced);

				ImGui::Text("Forced trail color:");
				ImGui::ColorEdit3("##ForcedTrailColor", &forced_color.x);

				ImGui::EndDisabled();

				ImGui::BeginDisabled(trail_method != TrailMethod::HueWheel);

				ImGui::Text("Hue Wheel increment:");
				ImGui::SliderFloat("##HueWheelIncrement", &increment, 0.0001f, 0.01f);

				ImGui::EndDisabled();
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new TrailPanel();
		}

		return nullptr;
	}

	// todo: is it possible to do this without a hook?
	HOOK(0x7CB380, void, __fastcall, CSceneVehicle_VisualUpdateAsync, uintptr_t c_scene_vehicle);

	void Init()
	{
		CREATE_HOOK(CSceneVehicle_VisualUpdateAsync);
	}

	void End()
	{
		Hooks::Destroy(0x7CB380);
	}

	void __fastcall CSceneVehicle_VisualUpdateAsync_(uintptr_t c_scene_vehicle)
	{
		if (trail_method == TrailMethod::Forced)
		{
			auto old_color = Read<TMUF::GmVec3>(c_scene_vehicle + 0xC8);

			Write<TMUF::GmVec3>(c_scene_vehicle + 0xC8, forced_color);

			CSceneVehicle_VisualUpdateAsync(c_scene_vehicle);

			Write<TMUF::GmVec3>(c_scene_vehicle + 0xC8, old_color);

		}
		else if (trail_method == TrailMethod::HueWheel)
		{
			static TMUF::GmVec3 color = { 255.0f, .0f, .0f };
			static float t = 0.0f;

			constexpr float PI2 = 3.14159265358979323846 * 2;
			constexpr float PI2_3 = 2 * 3.14159265358979323846 / 3;
			constexpr float PI4_3 = 4 * 3.14159265358979323846 / 3;

			color.x = 0.5 + 0.5 * cos(t);
			color.y = 0.5 + 0.5 * cos(t + PI2_3);
			color.z = 0.5 + 0.5 * cos(t + PI4_3);

			auto old_color = Read<TMUF::GmVec3>(c_scene_vehicle + 0xC8);

			Write<TMUF::GmVec3>(c_scene_vehicle + 0xC8, color);

			CSceneVehicle_VisualUpdateAsync(c_scene_vehicle);

			Write<TMUF::GmVec3>(c_scene_vehicle + 0xC8, old_color);

			t += increment;
			if (t >= PI2)
			{
				t -= PI2;
			}
		}
		else
		{
			CSceneVehicle_VisualUpdateAsync(c_scene_vehicle);
		}
	}
}

MODULE(trail);