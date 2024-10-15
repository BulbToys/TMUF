#include <Windows.h>
#include <cstdarg>
#include <stdio.h>

#include "utils.h"
#include "io.h"

void IFileBase::Save(const char* filename)
{
	size_t size = Size();

	if (!Validate())
	{
		char msg[512];
		sprintf_s(msg, 512, "Error saving file %s.\n\n"
			"The object you are trying to save has failed to validate, indicating it contains invalid (corrupt or otherwise unsafe) values.\n\nProceed?", filename);

		if (MessageBoxA(NULL, msg, PROJECT_NAME, MB_ICONWARNING | MB_YESNO | MB_SYSTEMMODAL) != IDYES)
		{
			return;
		}
	}

	// Avoid saving the vtable pointer
	size -= 4;

	FILE* file = nullptr;
	fopen_s(&file, filename, "wb");
	if (!file)
	{
		char error[64];
		strerror_s(error, errno);
		Error("Error saving file %s.\n\nError code %d: %s", filename, errno, error);
	}
	else
	{
		// Avoid saving the vtable pointer
		fwrite((char*)this + 4, 1, size, file);
		fclose(file);
	}
}

bool IFileBase::Load(const char* filename, bool allow_undersize)
{
	size_t size = Size();

	// Offset from vtable pointer
	size -= 4;

	FILE* file = nullptr;
	fopen_s(&file, filename, "rb");
	if (!file)
	{
		char error[64];
		strerror_s(error, errno);
		Error("Error opening file %s.\n\nError code %d: %s", filename, errno, error);
		return false;
	}
	else
	{
		char* buffer = new char[size];

		fseek(file, 0, SEEK_END);
		size_t len = ftell(file);
		fseek(file, 0, SEEK_SET);

		if (len > size || (!allow_undersize && len < size))
		{
			fclose(file);
			Error("Error opening file %s.\n\nInvalid file length - expected %d, got %d.", filename, size, len);
			return false;
		}

		fread_s(buffer, len, 1, len, file);
		fclose(file);

		if (!Validate())
		{
			char msg[512];
			sprintf_s(msg, 512, "Error opening file %s.\n\n"
				"The object you are trying to load has failed to validate, indicating it contains invalid (corrupt or otherwise unsafe) values.\n\nProceed?", filename);

			if (MessageBoxA(NULL, msg, PROJECT_NAME, MB_ICONWARNING | MB_YESNO | MB_SYSTEMMODAL) != IDYES)
			{
				return false;
			}
		}

		// Offset from vtable pointer
		memcpy((char*)this + 4, buffer, len);
	}

	return true;
}

void IFileBase::SaveDialog(const char* title, const char* filter, const char* default_extension)
{
	char filename[MAX_PATH] { 0 };

	OPENFILENAMEA ofn {};
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = IO::Get()->Window();
	ofn.lpstrFilter = filter;
	ofn.lpstrTitle = title;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = default_extension;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;

	if (GetSaveFileNameA(&ofn))
	{
		this->Save(ofn.lpstrFile);
	}
}

bool IFileBase::LoadDialog(const char* title, const char* filter, const char* default_extension, bool allow_undersize)
{
	char filename[MAX_PATH] { 0 };

	OPENFILENAMEA ofn {};
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = IO::Get()->Window();
	ofn.lpstrFilter = filter;
	ofn.lpstrTitle = title;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = default_extension;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;

	if (GetOpenFileNameA(&ofn))
	{
		return this->Load(ofn.lpstrFile);
	}

	return false;
}

PatchInfo::PatchInfo(uintptr_t patch_address, size_t patch_len)
{
	ASSERT(patch_address);
	ASSERT(patch_len);

	// Very weak check for double patching
	ASSERT(!PatchInfo::Find(patch_address));

	this->address = patch_address;
	this->len = patch_len;

	this->bytes = new char[patch_len];
	memcpy(this->bytes, reinterpret_cast<void*>(patch_address), patch_len);
	this->map.insert({ patch_address, this });
}

PatchInfo::~PatchInfo()
{
	this->map.erase(this->address);
	delete[] bytes;
}

PatchInfo* PatchInfo::Find(uintptr_t address)
{
	if (PatchInfo::map.find(address) == PatchInfo::map.end())
	{
		return nullptr;
	}
	return PatchInfo::map.at(address);
}

bool PatchInfo::SanityCheck()
{
	auto& map = PatchInfo::map;

	auto leftovers = map.size();
	if (leftovers > 0)
	{
		std::string patches;

		auto iter = map.begin();
		while (iter != map.end())
		{
			auto patch = iter->second;
			patches += "\n- " + std::to_string(patch->address);
			++iter;
		}

		Error("%d leftover patch(es)/hook(s) found: %s", leftovers, patches.c_str());
		return false;
	}
	return true;
}

void Error(const char* message, ...)
{
	char buffer[1024];
	va_list va;
	va_start(va, message);
	vsprintf_s(buffer, 1024, message, va);

	MessageBoxA(NULL, buffer, PROJECT_NAME, MB_ICONERROR | MB_SYSTEMMODAL);
}

void PatchNOP(uintptr_t address, int count)
{
	new PatchInfo(address, count);
	memset(reinterpret_cast<void*>(address), 0x90, count);
}

void PatchJMP(uintptr_t address, void* asm_func, size_t patch_len)
{
	ASSERT(patch_len >= 5);

	new PatchInfo(address, patch_len);

	ptrdiff_t relative = reinterpret_cast<uintptr_t>(asm_func) - address - 5;

	// Write the jump instruction
	Write<uint8_t>(address, 0xE9);

	// Write the relative address to jump to
	Write<ptrdiff_t>(address + 1, relative);

	// Write nops until we've reached length
	memset(reinterpret_cast<void*>(address + 5), 0x90, patch_len - 5);
}

void Unpatch(uintptr_t address, bool force_unpatch)
{
	auto patch = PatchInfo::Find(address);
	if (!patch)
	{
		if (force_unpatch)
		{
			Error("Tried to Unpatch() non-existent patch %08X in force mode.", address);
			DIE();
		}
		return;
	}

	memcpy(reinterpret_cast<void*>(address), patch->Bytes(), patch->Len());
	delete patch;
}

int WideStringToString(wchar_t* wide_str, size_t wide_str_len, char* str, size_t str_len)
{
	return WideCharToMultiByte(CP_UTF8, 0, wide_str, wide_str_len, str, str_len, NULL, NULL);
}

int StringToWideString(char* str, size_t str_len, wchar_t* wide_str, size_t wide_str_len)
{
	return MultiByteToWideChar(CP_UTF8, 0, str, str_len, wide_str, wide_str_len);
}