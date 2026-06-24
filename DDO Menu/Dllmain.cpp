#include "GuiWindow.h"
#include "MinHook/include/MinHook.h"
#include "GameHook/GameHooks.h"

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

bool KeyDown = false;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(WINAPI* Reset)(LPDIRECT3DDEVICE9 lpDirect3Device9, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef HRESULT(WINAPI* EndScene)(LPDIRECT3DDEVICE9 lpDirect3Device9);
HRESULT WINAPI HookReset(LPDIRECT3DDEVICE9 lpDirect3Device9, D3DPRESENT_PARAMETERS* pPresentationParameters);
HRESULT WINAPI HookEndScene(LPDIRECT3DDEVICE9 lpDirect3Device9);
LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static Reset        Original_Reset;
static EndScene     Original_EndScene;
static WNDPROC      Original_WndProc;
static HMODULE      g_hInstance;
static HANDLE       g_hEndEvent;
static ULONG_PTR*   g_lpVTable;
static GuiWindow*   g_GuiWindow;

void InitHook()
{
    ULONG_PTR* lpVTable = (ULONG_PTR*)g_lpVTable[0];
    MH_Initialize();

    // Reset
    LPVOID lpTarget = (LPVOID)lpVTable[16];
    MH_CreateHook(lpTarget, &HookReset, (void**)&Original_Reset);
    MH_EnableHook(lpTarget);

    // EndScene
    lpTarget = (LPVOID)lpVTable[42];
    MH_CreateHook(lpTarget, &HookEndScene, (void**)&Original_EndScene);
    MH_EnableHook(lpTarget);
    bool initGameHook = false;
    while (initGameHook == false)
    {
        CreateMemoryHook((void*)GameCameraAddr, (void**)&oGameplayCameraFunc, hGameplayCameraFunc);
        CreateMemoryHook((void*)PhotoModeCameraAddr, (void**)&oPhotoModeCameraFunc, hPhotoModeCameraFunc);
        CreateMemoryHook((void*)PhotoModePitchLimitAddr, (void**)&oPhotoModePitchLimitFunc, hPhotoModePitchLimitFunc);
        CreateMemoryHook((void*)GlobalCameraRollAddr, (void**)&oGlobalCameraRollFunc, hGlobalCameraRollFunc);
        CreateMemoryHook((void*)PhotoModeCameraControlAddr, (void**)&oPhotoModeCameraControlFunc, hPhotoModeCameraControlFunc);
        initGameHook = true;
    }


}

void ReleaseHook()
{
    ::SetWindowLongPtr(g_GuiWindow->hWnd, GWLP_WNDPROC, (LONG_PTR)Original_WndProc);
    MH_DisableHook(MH_ALL_HOOKS);

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    ::SetEvent(g_hEndEvent);
}

LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    if (g_GuiWindow->showMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return ::CallWindowProc(Original_WndProc, hWnd, uMsg, wParam, lParam);
}

inline void InitImGui(LPDIRECT3DDEVICE9 lpDirect3Device9)
{
    ImGui::CreateContext();

    ImFontConfig fontConfig{};
    fontConfig.GlyphOffset.y = -1.75f;

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF(g_GuiWindow->fontPath.c_str(), FONT_SIZE, &fontConfig);
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui_ImplWin32_Init(g_GuiWindow->hWnd);
    ImGui_ImplDX9_Init(lpDirect3Device9);
    Original_WndProc = (WNDPROC)::SetWindowLongPtr(g_GuiWindow->hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

HRESULT WINAPI HookEndScene(LPDIRECT3DDEVICE9 lpDirect3Device9)
{
    static bool bImGuiInit = false;

    if (!bImGuiInit)
    {

        InitImGui(lpDirect3Device9);
        bImGuiInit = true;
    }
    else if (g_GuiWindow->uiStatus & static_cast<DWORD>(GuiWindow::GuiState::GuiState_Detach))
    {
        ReleaseHook();
        return Original_EndScene(lpDirect3Device9);
    }
    //to do
    //Rewrite PhotoMode Check, grab PhotoMode from Game Function maybe?
    PhotoModeCheck = IsPhotoModeOn();
    ControlPhotoMode();

    auto InsertKey = GetAsyncKeyState(VK_INSERT);
    if (InsertKey < 0)
    {
        if (KeyDown == false)
        {
            KeyDown = true;
            g_GuiWindow->showMenu = !g_GuiWindow->showMenu;
            ToggleInput();
        }
    }
    else
        KeyDown = false;

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = g_GuiWindow->showMenu;

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (g_GuiWindow->showMenu) {
        //ImGui::ShowDemoWindow();
        g_GuiWindow->Update();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return Original_EndScene(lpDirect3Device9);
}

HRESULT WINAPI HookReset(LPDIRECT3DDEVICE9 lpDirect3Device9, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    ImGui_ImplDX9_InvalidateDeviceObjects();

    HRESULT hResult = Original_Reset(lpDirect3Device9, pPresentationParameters);

    ImGui_ImplDX9_CreateDeviceObjects();

    return hResult;
}

DWORD WINAPI ThreadEntry(LPVOID lpParameter)
{
    g_hInstance = (HMODULE)lpParameter;
    g_hEndEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    g_GuiWindow = new GuiWindow();
    g_GuiWindow->Initialize();

    WNDCLASSEX windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = ::DefWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = ::GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = NULL;
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = "Dear ImGui DirectX9";
    windowClass.hIconSm = NULL;

    ::RegisterClassEx(&windowClass);
    HWND hWnd = ::CreateWindow(
        windowClass.lpszClassName,
        windowClass.lpszClassName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        windowClass.hInstance,
        NULL);

    D3DPRESENT_PARAMETERS d3dpp{};
    d3dpp.BackBufferWidth = 0;
    d3dpp.BackBufferHeight = 0;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferCount = 1;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = 0;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.Windowed = 1;
    d3dpp.EnableAutoDepthStencil = 0;
    d3dpp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
    d3dpp.Flags = 0;
    d3dpp.FullScreen_RefreshRateInHz = 0;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    LPDIRECT3D9 lpDirect3D9 = ::Direct3DCreate9(D3D_SDK_VERSION);
    LPDIRECT3DDEVICE9 lpDirect3Device9;
    if (SUCCEEDED(lpDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpDirect3Device9)))
    {
        g_lpVTable = (ULONG_PTR*)lpDirect3Device9;

        InitHook();

        lpDirect3Device9->Release();
        lpDirect3D9->Release();
        g_lpVTable = nullptr;
    }
    ::DestroyWindow(hWnd);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

    if (g_hEndEvent)
        ::WaitForSingleObject(g_hEndEvent, INFINITE);
    delete g_GuiWindow;
    ::FreeLibraryAndExitThread(g_hInstance, EXIT_SUCCESS);

    return 0;
}

bool DDO()
{
    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        if (::GetModuleHandle("d3d9.dll") == NULL)
            return false;

        ::DisableThreadLibraryCalls(hModule);
        ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadEntry, hModule, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        ::Sleep(100);
        MH_Uninitialize();
        break;
    }

    return true;
}