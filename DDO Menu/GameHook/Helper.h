#pragma once
#include "../MinHook/include/MinHook.h"

inline void WriteMemory(uintptr_t Addr, char* Bytes, int Size)
{
	unsigned long OldProtection;
	VirtualProtect((LPVOID)(Addr), Size, PAGE_EXECUTE_READWRITE, &OldProtection);
	memcpy((LPVOID)Addr, Bytes, Size);
	FlushInstructionCache(GetCurrentProcess(), (LPCVOID)Addr, Size);
	VirtualProtect((LPVOID)(Addr), Size, OldProtection, NULL);
}

inline bool CreateMemoryHook(void* GameAddress, void** Original, void* Function)
{
	if (MH_CreateHook(GameAddress, Function, Original) != MH_OK || MH_EnableHook(GameAddress) != MH_OK) {
		return false;
	}
	return true;
}