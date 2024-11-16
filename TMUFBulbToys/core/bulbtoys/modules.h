#pragma once

#include "module.h"

// Modules are End()ed in reverse of the order they're Init()ed in to ensure proper cleanup. For example:
// Init: A, B, C, D, E
// End:  E, D, C, B, A
namespace Modules
{
	void Init();
	std::vector<IPanel*> Panels(Module::DrawType dt);
	void End();
}