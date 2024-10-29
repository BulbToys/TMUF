#include "../../core/bulbtoys.h"
#include "../tmuf.h"

#include <shlwapi.h>

namespace stats
{
	class StatsPanel : public IPanel
	{
		char search_name[64] {0};
		char search_author[64] {0};
		char search_envi[16] {0};

		struct Stat
		{
			// this struct is a disaster
			char name_raw[64] {0};
			std::vector<ImGui::TMUF_TextSlice> slices;
			std::string name_clean = "";

			TMUF::CFastString author;
			TMUF::CFastString envi;

			unsigned int total_time;
			unsigned int edit_time;
			unsigned int race_time;
			unsigned int net_time;
			unsigned __int16 reset_count;
			unsigned __int16 finish_count;

			bool FitsCriteria(const char* search_name = nullptr, const char* search_author = nullptr, const char* search_envi = nullptr)
			{
				if (search_name && strlen(search_name) > 0 && !StrStrIA(this->name_raw, search_name) && !StrStrIA(this->name_clean.c_str(), search_name))
				{
					return false;
				}

				if (search_author && strlen(search_author) > 0 && !StrStrIA(this->author.psz, search_author))
				{
					return false;
				}

				if (search_envi && strlen(search_envi) > 0 && !StrStrIA(this->envi.psz, search_envi))
				{
					return false;
				}

				return true;
			}
		}
		*stats = nullptr;
		int count = 0;

		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Stats"))
			{
				ImGui::Text("Search by Track Name:");
				ImGui::InputText("##StatsSearchName", search_name, IM_ARRAYSIZE(search_name));

				ImGui::Text("Search by Author Name:");
				ImGui::InputText("##StatsSearchAuthor", search_author, IM_ARRAYSIZE(search_author));

				ImGui::Text("Search by Environment:");
				ImGui::InputText("##StatsSearchEnvi", search_envi, IM_ARRAYSIZE(search_envi));

				if (ImGui::Button("Reload Data"))
				{
					if (stats)
					{
						delete[] stats;
						stats = nullptr;
						count = 0;
					}
				}
				ImGui::SameLine();
				ImGui::Text("Total race count: %d", count);

				if (!stats)
				{
					auto gps = Read<TMUF::CGamePlayerScore*>(TMUF::BulbToys_GetTrackMania() + 0x16C);
					if (gps)
					{
						count = gps->_Challenges.size;

						if (count > 0)
						{
							stats = new Stat[count];

							for (int i = 0; i < count; i++)
							{
								auto score = gps->_Challenges.pElems[i];

								WideStringToString(score->_TrackName.pwsz, score->_TrackName.size, stats[i].name_raw, 64);

								stats[i].slices = ImGui::TMUF_Parse(stats[i].name_raw);
								for (auto& slice : stats[i].slices)
								{
									stats[i].name_clean.append(slice.str);
								}

								TMUF::CMwId_GetName(&score->_AuthorNameID, &stats[i].author);
								TMUF::CMwId_GetName(&score->_EnvironmentNameID, &stats[i].envi);

								stats[i].total_time = score->_EditPlayTime + score->_RacePlayTime + score->_NetPlayTime;
								stats[i].edit_time = score->_EditPlayTime;
								stats[i].race_time = score->_RacePlayTime;
								stats[i].net_time = score->_NetPlayTime;
								stats[i].reset_count = score->_TotalResetCount;
								stats[i].finish_count = score->_TotalFinishCount;
							}
						}
					}
				}

				if (stats)
				{
					if (ImGui::BeginTable("StatsTable", 8, ImGuiTableFlags_SizingFixedFit))
					{
						// todo implement two-way sorting (asc and desc)
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("#");

						ImGui::TableSetColumnIndex(1);
						if (ImGui::Button("Track Name" "##StatsTable"))
						{
							qsort(stats, count, sizeof(Stat), +[](const void* a, const void* b)
							{
								Stat* stat_a = (Stat*)a;
								Stat* stat_b = (Stat*)b;

								return strcmp(stat_a->name_clean.c_str(), stat_b->name_clean.c_str());
							});
						}

						ImGui::TableSetColumnIndex(2);
						if (ImGui::Button("Author Name" "##StatsTable"))
						{
							qsort(stats, count, sizeof(Stat), +[](const void* a, const void* b)
							{
								Stat* stat_a = (Stat*)a;
								Stat* stat_b = (Stat*)b;

								return strcmp(stat_a->author.psz, stat_b->author.psz);
							});
						}

						ImGui::TableSetColumnIndex(3);
						if (ImGui::Button("Environment" "##StatsTable"))
						{
							qsort(stats, count, sizeof(Stat), +[](const void* a, const void* b)
							{
								Stat* stat_a = (Stat*)a;
								Stat* stat_b = (Stat*)b;

								return strcmp(stat_a->envi.psz, stat_b->envi.psz);
							});
						}

						ImGui::TableSetColumnIndex(4);
						if (ImGui::Button("Total Time" "##StatsTable"))
						{
							qsort(stats, count, sizeof(Stat), +[](const void* a, const void* b)
							{
								Stat* stat_a = (Stat*)a;
								Stat* stat_b = (Stat*)b;

								int diff = stat_a->total_time - stat_b->total_time;

								if (diff < 0)
								{
									return 1;
								}
								else if (diff > 0)
								{
									return -1;
								}

								return 0;
							});
						}

						ImGui::TableSetColumnIndex(5);
						if (ImGui::Button("Edit Time" "##StatsTable"))
						{
							qsort(stats, count, sizeof(Stat), +[](const void* a, const void* b)
							{
								Stat* stat_a = (Stat*)a;
								Stat* stat_b = (Stat*)b;

								int diff = stat_a->edit_time - stat_b->edit_time;

								if (diff < 0)
								{
									return 1;
								}
								else if (diff > 0)
								{
									return -1;
								}

								return 0;
							});
						}

						ImGui::TableSetColumnIndex(6);
						if (ImGui::Button("Race Time" "##StatsTable"))
						{
							qsort(stats, count, sizeof(Stat), +[](const void* a, const void* b)
							{
								Stat* stat_a = (Stat*)a;
								Stat* stat_b = (Stat*)b;

								int diff = stat_a->race_time - stat_b->race_time;

								if (diff < 0)
								{
									return 1;
								}
								else if (diff > 0)
								{
									return -1;
								}

								return 0;
							});
						}

						ImGui::TableSetColumnIndex(7);
						if (ImGui::Button("Netplay Time" "##StatsTable"))
						{
							qsort(stats, count, sizeof(Stat), +[](const void* a, const void* b)
							{
								Stat* stat_a = (Stat*)a;
								Stat* stat_b = (Stat*)b;

								int diff = stat_a->net_time - stat_b->net_time;

								if (diff < 0)
								{
									return 1;
								}
								else if (diff > 0)
								{
									return -1;
								}

								return 0;
							});
						}

						int displayed_row = 1;
						for (int row = 0; row < count; row++)
						{
							if (!stats[row].FitsCriteria(search_name, search_author, search_envi))
							{
								continue;
							}

							ImGui::TableNextRow();
							for (int column = 0; column < 8; column++)
							{
								ImGui::TableSetColumnIndex(column);

								switch (column)
								{
									case 0: ImGui::Text("%d", displayed_row++); break;
									case 1: ImGui::TMUF_TextEx(stats[row].slices, stats[row].name_raw); break;
									case 2: ImGui::Text("%s", stats[row].author.psz); break; // can use TMUF_Text but it's wasteful and unnecessary 99% of the time
									case 3: ImGui::Text("%s", stats[row].envi.psz); break;
									case 4: ImGui::Text("%u:%02u:%02u", stats[row].total_time / 3600, (stats[row].total_time / 60) % 60, stats[row].total_time % 60); break;
									case 5: ImGui::Text("%u:%02u:%02u", stats[row].edit_time / 3600, (stats[row].edit_time / 60) % 60, stats[row].edit_time % 60); break;
									case 6: ImGui::Text("%u:%02u:%02u", stats[row].race_time / 3600, (stats[row].race_time / 60) % 60, stats[row].race_time % 60); break;
									case 7: ImGui::Text("%u:%02u:%02u", stats[row].net_time / 3600, (stats[row].net_time / 60) % 60, stats[row].net_time % 60); break;
									default: break;
								}
							}
						}
						ImGui::EndTable();
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
			return new StatsPanel();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(stats);