#include "../../core/bulbtoys.h"
#include "../tmuf.h"

namespace analyser
{
	HOOK(0x70ED20, void, __fastcall, CGameAnalyzer_Init, uintptr_t cga);

	void Init()
	{
		CREATE_HOOK(CGameAnalyzer_Init);
	}

	void End()
	{
		Hooks::Destroy(0x70ED20);
	}

	void __fastcall CGameAnalyzer_Init_(uintptr_t cga)
	{
		// *(*(this + 40) + 44) - some CTrackManiaRaceAnalyzerContext shit idfk
		auto style = Read<uintptr_t>(Read<uintptr_t>(cga + 0x28) + 0x2C);

		auto grayed_char_attribs = reinterpret_cast<TMUF::CFastStringInt*>(style + 0xA8);
		grayed_char_attribs->SetString(2, L"$s");

		CGameAnalyzer_Init(cga);
	}
}

MODULE_NO_PANEL(analyser);