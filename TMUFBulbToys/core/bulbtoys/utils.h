#pragma once

#include <Windows.h>
#include <unordered_map>

#include "git.h"

#define PROJECT_NAME "BulbToys"

#define DIE() do { *((int*)0xDEAD) = 0; PostQuitMessage(0); } while (false)
#define ASSERT(cond) do { if (!(cond)) { Error("Assertion failed: " #cond); DIE(); } } while (false)

#define MYPRINTF(dest, size, fmt, ...) _snprintf_s(dest, size, size - 1, fmt, __VA_ARGS__)

// Callable function based on address
#define FUNC(address, return_t, callconv, name, ...) inline return_t (callconv* name)(__VA_ARGS__) = reinterpret_cast<decltype(name)>(address)

/* ===== Structs ===== */

// Base interface for all struct-files. Use IFile instead, which automatically implements Size() for you
struct IFileBase
{
	// Size of the data being saved
	virtual size_t Size() const = 0;

	// Return false if struct/class data is bogus, true otherwise - returning false prompts a warning when saving/loading
	// Use this as a sanity check for when you're about to save or load a struct to/from a file
	virtual bool Validate() = 0;

	// Save the struct to a file named filename
	virtual void Save(const char* filename) final;

	// Prompt the user where to save the file
	virtual void SaveDialog(const char* title = "Save File", const char* filter = "All Files (*.*)\0*.*\0", const char* default_extension = nullptr) final;

	// Load the struct from a file named filename. Object remains unchanged if this returns false!
	virtual bool Load(const char* filename, bool allow_undersize = false) final;

	// Prompt the user which file to load. Object remains unchanged if this returns false!
	virtual bool LoadDialog(const char* title = "Load File", const char* filter = "All Files (*.*)\0*.*\0", const char* default_extension = nullptr,
		bool allow_undersize = false) final;
};

// Interface for structs/classes that can be saved to files in a binary format
// If you wish to save game structs/classes, it's best to make an adapter class first, as this will append its own vtable and fuck up your member offsets
template <typename T>
struct IFile : IFileBase
{
	virtual size_t Size() const override final { return sizeof(T); }
};

class PatchInfo
{
	static inline std::unordered_map<uintptr_t, PatchInfo*> map;

	char* bytes = nullptr;

	uintptr_t address = 0;
	size_t len = 0;
public:
	PatchInfo(uintptr_t address, size_t len);
	~PatchInfo();

	inline char* Bytes() { return bytes; }
	inline size_t Len() { return len; }

	static PatchInfo* Find(uintptr_t address);

	static bool SanityCheck();
};

// Wrapper for string literals, primarily used for template arguments
template <size_t N>
struct StringLiteral
{
	char str[N];
	constexpr StringLiteral(const char(&str)[N])
	{
		for (int i = 0; i < N; i++)
		{
			this->str[i] = str[i];
		}
	}
};

// Simple RAII implementation of VirtualProtect
class Unprotect
{
	void* address;
	size_t size;
	DWORD protection_flags;
public:
	Unprotect(void* address, size_t size) : address(address), size(size) { VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &protection_flags); }
	Unprotect(uintptr_t address, size_t size) : Unprotect(reinterpret_cast<void*>(address), size) {}
	~Unprotect() { VirtualProtect(address, size, protection_flags, &protection_flags); }
};

/* ===== Templates ===== */

template <size_t size = 1024>
bool CopyToClipboard(const char* text, ...)
{
	auto mem = GlobalAlloc(GMEM_MOVEABLE, size);
	if (!mem)
	{
		return false;
	}

	auto ptr = GlobalLock(mem);
	if (!ptr)
	{
		GlobalFree(mem);
		return false;
	}

	char buffer[size] { 0 };
	va_list va;
	va_start(va, text);
	vsprintf_s(buffer, size, text, va);

	memcpy(ptr, buffer, size);
	GlobalUnlock(mem);

	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, mem);
	CloseClipboard();

	// The clipboard calls GlobalFree for us, no need to do it ourselves
	return true;
}

template <size_t size = 1024>
void Error(const char* message, ...)
{
	char buffer[size] { 0 };
	va_list va;
	va_start(va, message);
	vsprintf_s(buffer, size, message, va);

	MessageBoxA(NULL, buffer, PROJECT_NAME, MB_ICONERROR | MB_SYSTEMMODAL);
}

template <typename T>
inline T Read(uintptr_t address)
{
	return *reinterpret_cast<T*>(address);
}

template <typename T>
inline void Write(uintptr_t address, T value)
{
	*reinterpret_cast<T*>(address) = value;
}

template <typename T>
inline void WriteArray(uintptr_t address, std::initializer_list<T> list)
{
	T* array = reinterpret_cast<T*>(address);

	for (int i = 0; i < list.size(); i++)
	{
		array[i] = list.begin()[i];
	}
}

// Returns THE ADDRESS to the virtual function at <index> in the virtual table of this_
template <int index>
inline uintptr_t Virtual(uintptr_t this_)
{
	return Read<uintptr_t>(Read<uintptr_t>(this_) + index * 4);
}

// Returns A POINTER TO THE ADDRESS of the virtual function at <index> in the virtual table of this_
template <int index>
inline uintptr_t PtrVirtual(uintptr_t this_)
{
	return Read<uintptr_t>(this_) + index * 4;
}

// Identical to Write, except it gets added to the patch map (for later unpatching)
template <typename T>
inline void Patch(uintptr_t address, T value)
{
	new PatchInfo(address, sizeof(T));
	Write<T>(address, value);
}

// Identical to WriteArray, except it gets added to the patch map (for later unpatching)
template <typename T>
inline void PatchArray(uintptr_t address, std::initializer_list<T> list)
{
	new PatchInfo(address, sizeof(T) * list.size());
	WriteArray<T>(address, list);
}

/* ===== Functions ===== */

void PatchCall(uintptr_t address, void* func);

// Recommended to ONLY use for __declspec(naked) inline __asm only functions
// NOTE: The jump instruction is 5 bytes
void PatchJMP(uintptr_t address, void* asm_func, size_t patch_len = 5);

void PatchNOP(uintptr_t address, int count = 1);

void Unpatch(uintptr_t address, bool force_unpatch = true);

int WideStringToString(wchar_t* wide_str, size_t wide_str_len, char* str = nullptr, size_t str_len = 0);
int StringToWideString(char* str, size_t str_len, wchar_t* wide_str = nullptr, size_t wide_str_len = 0);