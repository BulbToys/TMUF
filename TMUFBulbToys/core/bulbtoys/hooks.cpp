#include "hooks.h"
#include "utils.h"

MH_STATUS Hooks::Init()
{
	auto status = MH_Initialize();
	if (status != MH_OK)
	{
		Error("Setup unsuccessful: Failed to initialize MinHook: %s", MH_StatusToString(status));
	}
	return status;
}

MH_STATUS Hooks::End()
{
	auto status = MH_Uninitialize();
	if (status != MH_OK)
	{
		Error("Failed to uninitialize MinHook: %s", MH_StatusToString(status));
	}
	return status;
}

bool Hooks::Create(void* address, void* hook, void* call)
{
	ASSERT(address != nullptr);

	auto status = MH_CreateHook(reinterpret_cast<void*>(address), hook, reinterpret_cast<void**>(call));
	if (status != MH_OK)
	{
		Error("Failed to create hook 0x%p: %s", address, MH_StatusToString(status));
		return false;
	}

	status = MH_EnableHook(reinterpret_cast<void*>(address));
	if (status != MH_OK)
	{
		Error("Failed to enable hook 0x%p: %s", address, MH_StatusToString(status));
		return false;
	}

	new PatchInfo(reinterpret_cast<uintptr_t>(address), 1);
	return true;
}

bool Hooks::Create(uintptr_t address, void* hook, void* call = nullptr)
{
	return Hooks::Create(reinterpret_cast<void*>(address), hook, call);
}

bool Hooks::Destroy(void* address)
{
	Unpatch(reinterpret_cast<uintptr_t>(address));

	auto status = MH_DisableHook(address);
	if (status != MH_OK)
	{
		Error("Failed to disable hook 0x%p: %s", address, MH_StatusToString(status));
		return false;
	}

	status = MH_RemoveHook(address);
	if (status != MH_OK)
	{
		Error("Failed to remove hook 0x%p: %s", address, MH_StatusToString(status));
		return false;
	}

	return true;
}

bool Hooks::Destroy(uintptr_t address)
{
	return Hooks::Destroy(reinterpret_cast<void*>(address));
}

void Hooks::VTablePatch(uintptr_t vtbl_func_addr, void* hook, void* call = nullptr)
{
	if (call)
	{
		*reinterpret_cast<void**>(call) = Read<void*>(vtbl_func_addr);
	}
	Patch<void*>(vtbl_func_addr, hook);
}
