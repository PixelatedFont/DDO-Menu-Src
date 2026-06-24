#pragma once
#include "Helper.h"
#include "Variables.h"
//to do: clean up this mess
inline uintptr_t CameraInputAddr = 0xB7F99D;
inline uintptr_t KeyboardInputAddr = 0xD68290;
inline uintptr_t GameCameraAddr = 0x45A9F6;
inline uintptr_t HudControlAddr = 0xD6B370;
inline uintptr_t CameraDetachAddr = 0xC3A517;

//Nop these to gain PhotoModeCam
//00F0B726 00F0B72F 00F0B73B

//PhotoMode
inline uintptr_t PhotoModeCollisonAddr = 0x45ECDE;
inline uintptr_t ForceEnablePhotoModeFirstAddr = 0xF0B726;
inline uintptr_t ForceEnablePhotoModeSecondAddr = 0xF0B72F;
inline uintptr_t ForceEnablePhotoModeThirdAddr = 0xF0B73B;
inline uintptr_t PhotoModeCameraAddr = 0x45E21F;
//ASM Rebuild
inline uintptr_t FnCall1 = 0x407940;
inline uintptr_t PhotoModeCameraRetAddr = 0x45E2AA;
//
inline uintptr_t PhotoModePitchLimitAddr = 0x0045E3E9;
//ASM Rebuild
inline uintptr_t PhotoModePitchLimitRetAddr = 0x45E406;
//
inline uintptr_t GlobalCameraRollAddr = 0xC2A691;
inline uintptr_t GlobalCameraRollRetAddr = 0xC2A698;
//
inline uintptr_t PhotoModeCameraControlAddr = 0x45E74F;
inline uintptr_t PhotoModeCameraControlRetAddr = 0x45E768;



inline uintptr_t PhotoModePointer = 0x22043E4;

//Memory Edits
inline void ToggleInput();
inline void DisableHud(bool toggle);
inline bool IsPhotoModeOn();
inline void ControlPhotoMode();
inline void TogglePhotoModeCollison(bool toggle);
inline void ToggleEnablePhotoModeEveryWhere(bool toggle);
inline void DetachCamera(bool toggle);


//Memory Hooks
typedef float*(__stdcall* GameplayCameraFunc)(DWORD* a1, int a2, int a3, float* a4, int a5);
inline GameplayCameraFunc oGameplayCameraFunc = (GameplayCameraFunc)GameCameraAddr;
inline float* __stdcall hGameplayCameraFunc(DWORD* a1, int a2, int a3, float* a4, int a5);

typedef int(__stdcall* PhotoModeCameraFunc)(DWORD* a1, int a2);
inline PhotoModeCameraFunc oPhotoModeCameraFunc = (PhotoModeCameraFunc)PhotoModeCameraAddr;
inline void hPhotoModeCameraFunc();

typedef void(__stdcall* PhotoModePitchLimitFunc)();
inline PhotoModePitchLimitFunc oPhotoModePitchLimitFunc = (PhotoModePitchLimitFunc)PhotoModePitchLimitAddr;
inline void hPhotoModePitchLimitFunc();

typedef void(__stdcall* GlobalCameraRollFunc)();
inline GlobalCameraRollFunc oGlobalCameraRollFunc = (GlobalCameraRollFunc)GlobalCameraRollAddr;
inline void hGlobalCameraRollFunc();

typedef void(__stdcall* PhotoModeCameraControlFunc)();
inline PhotoModeCameraControlFunc oPhotoModeCameraControlFunc = (PhotoModeCameraControlFunc)PhotoModeCameraControlAddr;
inline void hPhotoModeCameraControlFunc();



inline void ToggleInput()
{
	GameInputs = !GameInputs;
	if (GameInputs)
	{
		char EnableCameraInput[] = { 0xC7, 0x86, 0xAC, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		WriteMemory(CameraInputAddr, EnableCameraInput, sizeof(EnableCameraInput));

		char EnableKeyboardInput[] = { 0x32, 0xC0 };
		WriteMemory(KeyboardInputAddr, EnableKeyboardInput, sizeof(EnableKeyboardInput));
	}
	else
	{
		char DisableCameraInput[] = { 0xC7, 0x86, 0xAC, 0x19, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
		WriteMemory(CameraInputAddr, DisableCameraInput, sizeof(DisableCameraInput));

		char DisableKeyboardInput[] = { 0xC3, 0x90 };
		WriteMemory(KeyboardInputAddr, DisableKeyboardInput, sizeof(DisableKeyboardInput));
	}
}


//Memory Hooks
inline float* __stdcall hGameplayCameraFunc(DWORD *a1, int a2, int a3, float* a4, int a5)
{
	__asm
	{

		cmp LockCamera,1
		je CameraHook
		jmp oCode
		//ecx+0x60 is camera roll
		CameraHook:
		push eax
		mov eax, NormalCameraZoom
		mov dword ptr[ecx + 0x4C], eax
		mov eax, NormalCameraY
		mov dword ptr[ecx + 0x50], eax
		mov eax, NormalCameraX
		mov dword ptr[ecx + 0x54], eax
		mov eax, NormalCameraFov
		mov dword ptr[ecx + 0x5C], eax
		mov eax, NormalCameraPitchPositiveLimit
		mov dword ptr[ecx + 0x6C], eax
		mov eax, NormalCameraPitchNegativeLimit
		mov dword ptr[ecx + 0x68], eax
		pop eax
		jmp oCode

		oCode:
		movss xmm1, dword ptr[ecx + 0x6C]
	}
	return oGameplayCameraFunc(a1, a2, a3, a4, a5);
}

inline void __declspec(naked) hPhotoModeCameraFunc()
{
	__asm
	{
		call FnCall1
		fstp dword ptr[esp+0xc]
		movss xmm1, [esp+0x0C]
		mulss xmm1, [esp+0x08]
		movss xmm2, [esi+0xC0]
		comiss xmm2, PhotoModeCameraPositiveXLimit
		addss xmm1, [esi+0xC4]
		movss [esi+0xC4], xmm1
		jna LimitCheck1
		mov[esi+0xC0], 0x43960000
		jmp Continue

		LimitCheck1:
		movss xmm0, PhotoModeCameraNegativeXLimit
		comiss xmm0, xmm2
		jna Continue
		mov[esi+0xC0], 0xC3960000


		Continue:
		comiss xmm1, PhotoModeCameraPositiveYLimit
		jna LimitCheck2
		mov [esi+0xC4], 0x43160000
		jmp End

		LimitCheck2:
		movss xmm0, PhotoModeCameraNegativeYLimit
		comiss xmm0, xmm1
		jna End
		mov[esi+0xC4], 0xC3160000

		End:
		mov eax,[esi+0xC0]
		jmp PhotoModeCameraRetAddr
	}
}

inline void __declspec(naked) hPhotoModePitchLimitFunc()
{
	__asm
	{
		movss xmm1, PhotoModePitchPositiveYLimit
		movss xmm0, [esi]
		mulss xmm1, xmm2
		comiss xmm0, xmm1
		jna Jump1
		movss [esi], xmm1
		jmp Jump1

		Jump1:
		movss xmm0, PhotoModePitchNegativeYLimit
		jmp PhotoModePitchLimitRetAddr
	}
}

inline void __declspec(naked) hGlobalCameraRollFunc()
{
	__asm
	{
		push eax
		mov eax,0x22043E4
		mov eax,[eax]
		mov eax, [eax+0xDB2] //Check if PhotoMode is Running
		cmp eax,1
		pop eax
		je mCode
		jmp oCode

		mCode:
		movss xmm4, CameraRoll
		movaps  xmm1, xmm4
		mulss   xmm1, xmm4
		jmp End

		oCode:
		movaps  xmm1, xmm4
		mulss   xmm1, xmm4
		jmp End

		End:
		jmp GlobalCameraRollRetAddr

	}
}

inline void __declspec(naked) hPhotoModeCameraControlFunc()
{
	__asm
	{
		cmp isControlPhotoModeCamera,1
		je mCode
		jmp oCode

		mCode:
		movss [esp+0x8], xmm0
		movss xmm0, PhotoModeCameraX
		movss [ecx],xmm0
		movss [esp+0x4], xmm0
		movss xmm0, PhotoModeCameraY
		movss[eax], xmm0
		movss xmm7, PhotoModeCameraZoom // XMM7 Register is used later by this function
		movss [esi+0x100], xmm7
		movss [esp], xmm0
		jmp End

		oCode:
		movss[esp + 0x8], xmm0
		movss xmm0, [ecx]
		movss PhotoModeCameraX,xmm0 // Store Current X Axis
		movss[esp + 0x4], xmm0
		movss xmm0, [eax]
		movss PhotoModeCameraY,xmm0 // Store Current Y Axis
		movss[esp], xmm0
		jmp End

		End:
		jmp PhotoModeCameraControlRetAddr
	}

}

//Memory Edits
inline bool IsPhotoModeOn()
{
	uintptr_t PtLv1 = *reinterpret_cast<uintptr_t*>(PhotoModePointer);
	if (PtLv1 != NULL)
	{
		uintptr_t PtLv2 = PtLv1 + 0xDB2;
		auto PhotoModeSwitch = *reinterpret_cast<int*>(PtLv2);;
		if (PhotoModeSwitch == 1)
		{
			return true;
		}
		else return false;
	}
	else return false;
}



inline void TogglePhotoModeCollison(bool toggle)
{
	if (toggle)
	{
		char DisablePhotoModeCameraCollision[] = { 0x90, 0x90 };
		WriteMemory(PhotoModeCollisonAddr, DisablePhotoModeCameraCollision, sizeof(DisablePhotoModeCameraCollision));
	}
	else
	{
		char EnablePhotoModeCameraCollision[] = { 0x76, 0x3C };
		WriteMemory(PhotoModeCollisonAddr, EnablePhotoModeCameraCollision, sizeof(EnablePhotoModeCameraCollision));

	}
}

inline void ToggleEnablePhotoModeEveryWhere(bool toggle)
{
	if (toggle)
	{
		char DisableCheck[] = { 0x90, 0x90 };
		WriteMemory(ForceEnablePhotoModeFirstAddr, DisableCheck, sizeof(DisableCheck));
		WriteMemory(ForceEnablePhotoModeSecondAddr, DisableCheck, sizeof(DisableCheck));
		WriteMemory(ForceEnablePhotoModeThirdAddr, DisableCheck, sizeof(DisableCheck));
	}
	else
	{
		char EnableFirstCheck[] = { 0x75, 0x22 };
		WriteMemory(ForceEnablePhotoModeFirstAddr, EnableFirstCheck, sizeof(EnableFirstCheck));
		char EnableSecondCheck[] = { 0x75, 0x19 };
		WriteMemory(ForceEnablePhotoModeSecondAddr, EnableSecondCheck, sizeof(EnableSecondCheck));
		char EnableThirdCheck[] = { 0x74, 0x0D };
		WriteMemory(ForceEnablePhotoModeThirdAddr, EnableThirdCheck, sizeof(EnableThirdCheck));
	}
}


inline void DisableHud(bool toggle)
{
	if (toggle)
	{
		char DisableHud[] = { 0xB0, 0x00 };
		WriteMemory(HudControlAddr, DisableHud, sizeof(DisableHud));
	}
	else
	{
		char EnableHud[] = { 0xB0, 0x01 };
		WriteMemory(HudControlAddr, EnableHud, sizeof(EnableHud));

	}
}


inline void DetachCamera(bool toggle)
{
	if (toggle)
	{
		char DetachCamera[] = { 0x0F, 0x85 };
		WriteMemory(CameraDetachAddr, DetachCamera, sizeof(DetachCamera));
	}
	else
	{
		char AttachCamera[] = { 0x0F, 0x84 };
		WriteMemory(CameraDetachAddr, AttachCamera, sizeof(AttachCamera));

	}
}


//Misc
inline void ControlPhotoMode()
{
	if (PhotoModeCheck)
	{
		if (isControlCameraRoll)
		{
			if (GetAsyncKeyState(0x4F) && CameraRoll < 2.0f)
			{
				CameraRoll += 0.01f;
			}
			if (GetAsyncKeyState(0x49) && CameraRoll > -2.0f)
			{
				CameraRoll -= 0.01f;
			}
		}
		else CameraRoll = 0.0f;
		if (isControlPhotoModeCamera)
		{
			if (GetAsyncKeyState(0x4A))
			{
				PhotoModeCameraX += 0.01f;
			}

			if (GetAsyncKeyState(0x4B))
			{
				PhotoModeCameraX -= 0.01f;
			}

			if (GetAsyncKeyState(0x4E))
			{
				PhotoModeCameraY += 0.01f;
			}

			if (GetAsyncKeyState(0x4D))
			{
				PhotoModeCameraY -= 0.01f;
			}

			if (GetAsyncKeyState(0x47) && PhotoModeCameraZoom < 3000.0f)
			{
				PhotoModeCameraZoom += 1.0f;
			}

			if (GetAsyncKeyState(0x48) && PhotoModeCameraZoom > -3000.0f)
			{
				PhotoModeCameraZoom -= 1.0f;
			}
		}
		else PhotoModeCameraZoom = 325.0f;
	}
	else
	{
		CameraRoll = 0.0f;
	}
}