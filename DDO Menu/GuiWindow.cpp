#pragma once
#include "GuiWindow.h"
#include "GameHook/Variables.h"
#include "GameHook/GameHooks.h"

GuiWindow::GuiWindow()
{
    // Initialize settings
    this->hWnd = nullptr;
    this->hModule = nullptr;
    this->hProcess = nullptr;
    this->lpModuleAddress = nullptr;
    this->initialPosition = ImVec2(0.0f, 0.0f);
    this->uiStatus = static_cast<DWORD>(GuiState::GuiState_Reset);
    this->showMenu = false;

    // Set font path
    LPSTR lpBuffer = new char[MAX_PATH] {};
    ::GetEnvironmentVariable("WINDIR", lpBuffer, MAX_PATH);
    ::strcat_s(lpBuffer, MAX_PATH, "\\Fonts\\Arial.ttf");
    this->fontPath = lpBuffer;
    delete[] lpBuffer;

    // Set window title
    this->windowTitle = this->windowTitle.append(WINDOW_NAME).append("v") +
        std::to_string(MAJOR_VERSION) + "." +
        std::to_string(MINOR_VERSION) + "." +
        std::to_string(REVISION_VERSION);

    // Allocate memory
    this->lpBuffer = (LPBYTE)::VirtualAlloc(NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (this->lpBuffer)
        ::memset(this->lpBuffer, 0xCC, 0x1000);
}

GuiWindow::~GuiWindow()
{
    ::VirtualFree(this->lpBuffer, 0, MEM_RELEASE);
}

static bool CALLBACK EnumHwndCallback(HWND hWnd, LPARAM lParam)
{
    const auto isMainWindow = [hWnd]() {
        return ::GetWindow(hWnd, GW_OWNER) == NULL && ::IsWindowVisible(hWnd);
        };

    DWORD dwProcessId = 0;
    ::GetWindowThreadProcessId(hWnd, &dwProcessId);

    if (::GetCurrentProcessId() != dwProcessId || !isMainWindow() || hWnd == ::GetConsoleWindow())
        return true;

    *(HWND*)lParam = hWnd;

    return false;
}

void GuiWindow::Initialize()
{
    do
    {
        ::EnumWindows((WNDENUMPROC)EnumHwndCallback, (LPARAM)&this->hWnd);
        ::Sleep(200);
    } while (!this->hWnd);

    this->hProcess = ::GetCurrentProcess();
    this->hModule = ::GetModuleHandle(MODULE_NAME);
    this->lpModuleAddress = (LPBYTE)this->hModule;
}

void GuiWindow::Update()
{
    // Set window flags to disable title bar, resizing, scrollbars, mouse wheel scrolling, and saved settings
    const ImGuiWindowFlags windowflags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin(WINDOW_NAME, nullptr, windowflags);

    // Check if the window state needs to be reset
    if (this->uiStatus & static_cast<DWORD>(GuiState::GuiState_Reset))
    {
        ImGui::SetWindowPos(this->initialPosition);
        ImGui::SetWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT));
        this->uiStatus &= ~static_cast<DWORD>(GuiState::GuiState_Reset);
    }

    // Get window padding and position
    ImVec2& windowPadding = ImGui::GetStyle().WindowPadding;
    ImVec2 windowPosition = ImGui::GetWindowPos();

    // Display CloseButton in the top-right corner
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::CloseButton(CLOSE_BUTTON_ID, ImVec2(windowPosition.x + WINDOW_WIDTH - FONT_SIZE, windowPosition.y)))
        this->uiStatus |= static_cast<DWORD>(GuiState::GuiState_Exiting);
    ImGui::PopStyleVar();

    // Display shortcut hint in the bottom-right corner
    const std::string hotKey = std::string("Press INSERT to Toggle Menu");
    ImVec2 textSize = ImGui::CalcTextSize(hotKey.c_str());
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH - windowPadding.x - textSize.x, WINDOW_HEIGHT - windowPadding.y - textSize.y));
    ImGui::Text(hotKey.c_str());

    // Display author information and build date in the bottom-left corner
    const std::string authorInfo = std::string(AUTHOR_INFO) + __DATE__;
    ImGui::SetCursorPosY(WINDOW_HEIGHT - windowPadding.y - textSize.y);
    ImGui::Text(authorInfo.c_str());

    // Check if an exit operation is required
    if (this->uiStatus & static_cast<DWORD>(GuiState::GuiState_Exiting))
        this->ExitButton();

    // Display the window title at the top of the ImGui window
    ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y));
    ImGui::Text(windowTitle.c_str());

    // Insert your custom ImGui code here
    if (ImGui::BeginTabBar("Tabs"))
    {
        if (ImGui::BeginTabItem("General Camera"))
        {
            ImGui::Text("WORK IN PROGRESS");
            if (ImGui::Checkbox("Detach Camera", &isCameraDetach))
            {
                DetachCamera(isCameraDetach);
            }
            ImGui::Checkbox("Toggle Camera Edit", &LockCamera);
            if (LockCamera)
            {
                ImGui::SliderFloat("Camera Zoom", &NormalCameraZoom, 30.0f, 999.0f, "%.3f");
                ImGui::SliderFloat("Camera Y", &NormalCameraY, -999.0f, 999.0f, "%.3f");
                ImGui::SliderFloat("Camera X", &NormalCameraX, -999.0f, 999.0f, "%.3f");
                ImGui::SliderFloat("Camera Fov", &NormalCameraFov, 1.0f, 120.0f, "%.3f");
                ImGui::Checkbox("UnlockNormalCameraLimit", &UnlockNormalCameraLimit);
                if (UnlockNormalCameraLimit)
                {
                    NormalCameraPitchNegativeLimit = -80.0f;
                    NormalCameraPitchPositiveLimit = 80.0f;
                }
                else
                {
                    NormalCameraPitchNegativeLimit = -30.0f;
                    NormalCameraPitchPositiveLimit = 30.0f;
                }
                if (ImGui::Button("Reset"))
                {
                    NormalCameraPitchNegativeLimit = -30.0f;
                    NormalCameraPitchPositiveLimit = 30.0f;
                    NormalCameraZoom = 400.0f;
                    NormalCameraY = 180.0f;
                    NormalCameraX = 0.0f;
                }
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Combat Camera"))
        {
            ImGui::Text("WORK IN PROGRESS");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Character"))
        {
            ImGui::Text("WORK IN PROGRESS");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Customize Hud"))
        {
            ImGui::Text("WORK IN PROGRESS");
            if (ImGui::Checkbox("Toggle Hud", &isDisableHud))
            {
                DisableHud(isDisableHud);
            }    
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Photo Mode"))
        {
            ImGui::Text("WORK IN PROGRESS");



            if (ImGui::Checkbox("Enable Photo Mode Everywhere", &PhotoModeEverywhere))
            {
                ToggleEnablePhotoModeEveryWhere(PhotoModeEverywhere);
            }
            else ToggleEnablePhotoModeEveryWhere(PhotoModeEverywhere);

            if (ImGui::Checkbox("Disable Photo Camera Collision", &PhotoModeCameraCollision))
            {
                TogglePhotoModeCollison(PhotoModeCameraCollision);
            }
            else TogglePhotoModeCollison(PhotoModeCameraCollision);

            ImGui::Checkbox("Manual Control Photo Mode Camera", &isControlPhotoModeCamera);
            if (isControlPhotoModeCamera)
            {
                ImGui::Text("Press J or K While in Photo Mode to Rotate Camera Left/Right");
                ImGui::Text("Press N or M While in Photo Mode to Rotate Camera Up/down");
                ImGui::Text("Press G or H While in Photo Mode to Zoom Camera In/Out");
            }

            ImGui::Checkbox("Enable Camera Roll Control", &isControlCameraRoll);
            if (isControlCameraRoll)
            {
                ImGui::Text("Press I or O While in Photo Mode to Roll Camera Left/Right");
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void GuiWindow::ExitButton()
{
    //ImGui::SetCursorPos(ImVec2(0, 0));
    //ImGui::BeginChildFrame(CHILD_FRAME_ID, ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT));
    //ImGui::SetCursorPos(ImVec2(0, 0));
    //ImGui::BeginChild("Exiting", ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT));

    //std::string strText = std::string("Do you want to exit this program?");
    //ImVec2 textSize = ImGui::CalcTextSize(strText.c_str());
    //ImGui::SetCursorPos(ImVec2((WINDOW_WIDTH - textSize.x) * 0.5f, WINDOW_HEIGHT * 0.382f - textSize.y * 0.5f));
    //ImGui::Text(strText.c_str());

    //ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH * 0.5f - 120, WINDOW_HEIGHT * 0.618f));
    //if (ImGui::Button("Confirm", ImVec2(100.0f, 50.0f)))
    //    this->uiStatus |= static_cast<DWORD>(GuiState::GuiState_Detach);

    //ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH * 0.5f + 20, WINDOW_HEIGHT * 0.618f));
    //if (ImGui::Button("Cancel", ImVec2(100.0f, 50.0f)))
    //    this->uiStatus &= ~static_cast<DWORD>(GuiState::GuiState_Exiting);

    //ImGui::EndChild();
    //ImGui::EndChildFrame();
    showMenu = false;
}