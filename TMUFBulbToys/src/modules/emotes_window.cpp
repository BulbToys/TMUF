#include "../../core/bulbtoys.h"
#include "../../core/bulbtoys/gui.h"

#include <filesystem>

namespace emotes_window
{
	// change these two as needed
	constexpr float emote_max_size_px = 48;
	constexpr int emotes_per_line = 5;

	struct Emote
	{
		ImTextureID texture = nullptr;

		// TMUF chat limit
		wchar_t data[151] { 0 };
	};
	std::vector<Emote*> emotes;

	class EmotesWindow : public IWindow
	{
		virtual bool Draw() override final
		{
			uint32_t i = 0;

			auto iter = emotes.begin();
			while (iter != emotes.end())
			{
				auto emote = *iter;

				char str[32];
				MYPRINTF(str, 32, "##Emote_%u", i);

				if (i % emotes_per_line != 0)
				{
					ImGui::SameLine();
				}

				if (ImGui::ImageButton(str, emote->texture, { emote_max_size_px, emote_max_size_px }))
				{
					CopyToClipboardUnicode(emote->data);
				}

				++iter;
				++i;
			}

			return true;
		}
	public:
		EmotesWindow() : IWindow(true, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar, "Emotes [%u]", emotes.size()) {}
	};

	void CreateEmotesWindow()
	{
		new EmotesWindow();
	}

	// F7 is default, user-modifiable in TMUF_BulbToys.ini
	IO::Hotkey<"CreateEmotesWindow", VK_F7>* create_emotes_window = nullptr;

	void Init()
	{
		auto gui = GUI::Get();
		if (gui)
		{
			create_emotes_window = new IO::Hotkey<"CreateEmotesWindow", VK_F7> { CreateEmotesWindow };

			auto texture_loader = gui->TextureLoader();

			namespace fs = std::filesystem;

			auto emotes_folder = fs::current_path() / "Emotes";
			if (fs::exists(emotes_folder) && fs::is_directory(emotes_folder))
			{
				for (const auto& file : fs::directory_iterator(emotes_folder))
				{
					if (!fs::is_regular_file(file))
					{
						continue;
					}

					if (file.path().extension() != ".bmp" &&
						file.path().extension() != ".dds" &&
						file.path().extension() != ".dib" &&
						file.path().extension() != ".hdr" &&
						file.path().extension() != ".jpg" &&
						file.path().extension() != ".jpeg" &&
						file.path().extension() != ".pfm" &&
						file.path().extension() != ".png" &&
						file.path().extension() != ".ppm" &&
						file.path().extension() != ".tga")
					{
						continue;
					}

					auto txt_path = file.path();
					txt_path.replace_extension("txt");

					if (!fs::exists(txt_path))
					{
						continue;
					}

					struct Buffer : public IFile<Buffer>
					{
						wchar_t data[153]{ 0 };

						// txt file must be saved in UTF-16LE
						bool Validate() override final { return /*data[0] == 0xFEFF*/ true; }
					};
					Buffer buffer;

					auto size = buffer.Load(txt_path.string().c_str(), true);
					if (size == 0)
					{
						continue;
					}

					auto texture = texture_loader->Load(file.path().string().c_str());
					if (!texture)
					{
						continue;
					}

					auto emote = new Emote();
					emote->texture = texture;
					memcpy(emote->data, buffer.data + 1, size - 2);
					emotes.push_back(emote);
				}
			}
		}
	}

	void End()
	{
		auto gui = GUI::Get();
		if (gui)
		{
			auto texture_loader = gui->TextureLoader();

			auto iter = emotes.begin();
			while (iter != emotes.end())
			{
				auto emote = *iter;

				texture_loader->Unload(emote->texture);

				emotes.erase(iter);
				delete emote;
			}

			delete create_emotes_window;
		}
	}
}

MODULE_NO_PANEL(emotes_window);