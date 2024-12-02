#include "../../core/bulbtoys.h"
#include "../tmuf.h"

namespace test
{
	struct TestPanel : IPanel
	{
		bool deref = false;
		char classname_addy[9] { 0 };

		virtual bool Draw() override final
		{
			auto trackmania = TMUF::BulbToys_GetTrackMania();

			if (ImGui::BulbToys_Menu("Test Features"))
			{
				auto network = Read<uintptr_t>(trackmania + 0x12C);
				auto master_server = Read<uintptr_t>(network + 0x1B0);
				auto features = reinterpret_cast<TMUF::CFastBuffer<TMUF::CGameMasterServer_SFeature>*>(master_server + 0x2B0);
				auto count = features->size;

				if (count > 0)
				{
					if (ImGui::BeginTable("FeatureTable", 3, ImGuiTableFlags_SizingFixedFit))
					{
						// todo implement two-way sorting (asc and desc)
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("#");
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("Feature");
						ImGui::TableSetColumnIndex(2);
						ImGui::Text("Value");

						for (int row = 0; row < count; row++)
						{
							auto feature = features->pElems[row];

							TMUF::CFastString name;
							TMUF::CMwId_GetName(&feature._ID, &name);

							ImGui::TableNextRow();
							for (int column = 0; column < 3; column++)
							{
								ImGui::TableSetColumnIndex(column);

								switch (column)
								{
								case 0: ImGui::Text("%d", row + 1); break;
								case 1: ImGui::Text("%s", name.psz); break;
								case 2: ImGui::Text("%d", feature._Value); break;
								default: break;
								}
							}
						}
						ImGui::EndTable();
					}
				}
				else
				{
					ImGui::Text("No features found");
				}
			}

			if (ImGui::BulbToys_Menu("Test Format"))
			{
				char string[128] { 0 };

				ImGui::InputText("##FTest", string, IM_ARRAYSIZE(string));
				ImGui::TMUF_Text(string);
			}

			if (ImGui::BulbToys_Menu("Test GetClassName"))
			{
				ImGui::Checkbox("Dereference", &(this->deref));
				ImGui::InputText("##GetClassName_InputAddr", this->classname_addy, IM_ARRAYSIZE(this->classname_addy), ImGuiInputTextFlags_CharsHexadecimal);
				
				uintptr_t addr;
				if (sscanf_s(this->classname_addy, "%IX", &addr) == 1)
				{
					if (!deref)
					{
						// do not dereference, this is a pointer to a vtable
						ImGui::Text("Class: %s", TMUF::BulbToys_GetClassName(addr));
					}
					else
					{
						// do dereference, this is a class pointer which has a vtable
						Unprotect _(addr, 4);
						auto vtbl = Read<uintptr_t>(addr);
						ImGui::BulbToys_AddyLabel(vtbl, "VTable");
						ImGui::Text("Class: %s", TMUF::BulbToys_GetClassName(vtbl));
					}
				}
				else
				{
					ImGui::Text("Address is not valid!");
				}
			}

			if (ImGui::BulbToys_Menu("Test Time"))
			{
				SYSTEMTIME st;
				GetLocalTime(&st);

				/*
				typedef struct _SYSTEMTIME {
					WORD wYear;
					WORD wMonth;
					WORD wDayOfWeek;
					WORD wDay;
					WORD wHour;
					WORD wMinute;
					WORD wSecond;
					WORD wMilliseconds;
				} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
				
				*/

				auto get_month_name = +[](char month)
				{
					switch (month)
					{
					case 1: return "January";
					case 2: return "February";
					case 3: return "March";
					case 4: return "April";
					case 5: return "May";
					case 6: return "June";
					case 7: return "July";
					case 8: return "August";
					case 9: return "September";
					case 10: return "October";
					case 11: return "November";
					case 12: return "December";
					default: return "(invalid month)";
					}
				};

				auto get_dow_name = +[](char dow)
				{
					switch (dow)
					{
					case 1: return "Monday";
					case 2: return "Tuesday";
					case 3: return "Wednesday";
					case 4: return "Thursday";
					case 5: return "Friday";
					case 6: return "Saturday";
					case 0: return "Sunday";
					default: return "(invalid dow)";
					}
				};

				ImGui::Text("Local Time: %s, %u %s %u at %u:%02u:%02u.%03u",
					get_dow_name(st.wDayOfWeek), st.wDay, get_month_name(st.wMonth), st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

				static TMUF::SSystemTime sst;
				TMUF::Win32SystemTimeToSystemTime(&sst, &st);

				ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(&sst), "SSystemTime");

				TMUF::SystemTimeToWin32SystemTime(&st, &sst);

				ImGui::Text("Converted: %s, %u %s %u at %u:%02u:%02u.%03u",
					get_dow_name(st.wDayOfWeek), st.wDay, get_month_name(st.wMonth), st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			}

			return true;
		}
	};
	

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new TestPanel();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(test);