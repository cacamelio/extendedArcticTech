#include "UI.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "../ImGui/imgui.h"
#include "../ImGui/backends/imgui_impl_win32.h"
#include "../ImGui/backends/imgui_impl_dx9.h"
#include "../ImGui/imgui_internal.h"
#include <d3d9.h>
#include <tchar.h>

#include "../ImGui/imgui_settings.h"

#include "logo.h"
#include "mulish_font.h"
#include "icons.h"

#include "../SDK/Interfaces.h"

CMenu* Menu = new CMenu;

namespace pic {
    IDirect3DTexture9* logo = nullptr;
    IDirect3DTexture9* user = nullptr;

    namespace tab {
        IDirect3DTexture9* aimbot = nullptr;
        IDirect3DTexture9* movement = nullptr;
        IDirect3DTexture9* visuals = nullptr;
        IDirect3DTexture9* misc = nullptr;
        IDirect3DTexture9* players = nullptr;
        IDirect3DTexture9* configs = nullptr;
        IDirect3DTexture9* scripts = nullptr;
    }
}

namespace font {
    ImFont* general = nullptr;
    ImFont* tab = nullptr;
}

static ImGuiIO* im_io;

void CMenu::Setup() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    im_io = &ImGui::GetIO();

    ImFontConfig cfg;

    font::general = im_io->Fonts->AddFontFromMemoryTTF(mulish, sizeof(mulish), 19.f, &cfg, im_io->Fonts->GetGlyphRangesCyrillic());
    font::tab = im_io->Fonts->AddFontFromMemoryTTF(mulish, sizeof(mulish), 15.f, &cfg, im_io->Fonts->GetGlyphRangesCyrillic());

    im_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    im_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    D3DDEVICE_CREATION_PARAMETERS creationParameters = { };
    if (FAILED(DirectXDevice->GetCreationParameters(&creationParameters)))
        return;

    // store window pointer
    HWND hWindow = creationParameters.hFocusWindow;
    if (hWindow == nullptr)
        return;

    ImGui_ImplWin32_Init(hWindow);
    ImGui_ImplDX9_Init(DirectXDevice);

    D3DXCreateTextureFromFileInMemory(DirectXDevice, logo, sizeof(logo), &pic::logo);

    D3DXCreateTextureFromFileInMemory(DirectXDevice, aimbot, sizeof(aimbot), &pic::tab::aimbot);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, movement, sizeof(movement), &pic::tab::movement);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, visuals, sizeof(visuals), &pic::tab::visuals);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, misc, sizeof(misc), &pic::tab::misc);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, players, sizeof(players), &pic::tab::players);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, configs, sizeof(configs), &pic::tab::configs);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, scripts, sizeof(scripts), &pic::tab::scripts);

    m_bIsInitialized = true;
}

void CMenu::Release() {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    ImGui::Shutdown();
}

void CMenu::Render() {
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    static bool insert_pressed = false;

    if (GetAsyncKeyState(VK_INSERT) & 0x8000) {
        if (!insert_pressed) {
            m_bMenuOpened = !m_bMenuOpened;
            insert_pressed = true;
        }
    }
    else {
        insert_pressed = false;
    }

    if (m_bMenuOpened) {
        static float color[4] = { 234 / 255.f, 88 / 255.f, 12 / 255.f };

        c::accent = ImColor(color[0], color[1], color[2]);

        ImGui::GetStyle().ItemSpacing = ImVec2(24, 24);
        ImGui::GetStyle().WindowPadding = ImVec2(0, 0);
        ImGui::GetStyle().ScrollbarSize = 8.f;

        ImGui::SetNextWindowSizeConstraints(ImVec2(c::background::size), ImGui::GetIO().DisplaySize);

        ImGui::Begin("MENU", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
        {
            const ImVec2& p = ImGui::GetWindowPos(), c = ImGui::GetContentRegionMax(), i = ImGui::GetStyle().ItemSpacing;

            ImGui::GetBackgroundDrawList()->AddRectFilled(p + ImVec2(0, 0), p + ImVec2(c), ImGui::GetColorU32(c::background::bg), c::background::rounding);

            ImGui::GetWindowDrawList()->AddRectFilled(p + ImVec2(i), p + ImVec2(181, (c.y - i.y)), ImGui::GetColorU32(c::child::bg), c::child::rounding);

            ImGui::SetCursorPos(ImVec2(181 + i.x + (i.x / 2), i.y + (i.y / 2)));

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, (12 * 2)));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(16, 16));

            ImGui::BeginGroup();
            {
                static int select1 = 0;
                const char* items1[5]{ "Default", "125%", "150%", "175%", "200%" };

                if (ImGui::Button("Global", ImVec2(60, 35)));

                ImGui::SameLine(0, (i.x / 2));

                ImGui::Combo("##Dropdown example", &select1, items1, IM_ARRAYSIZE(items1), 5, 180);

                switch (select1) {
                case 0:
                    c::background::size = ImVec2(850, 650);
                    break;
                case 1:
                    c::background::size = ImVec2(900, 700);
                    break;
                case 2:
                    c::background::size = ImVec2(950, 750);
                    break;
                case 3:
                    c::background::size = ImVec2(1000, 800);
                    break;
                case 4:
                    c::background::size = ImVec2(1050, 850);
                    break;
                case 5:
                    c::background::size = ImVec2(1100, 900);

                    break;
                }

                ImGui::SetCursorPos(ImVec2((c.x - 181) - (i.x / 2) + (i.x / 2), i.y + (i.y / 2)));

                static char search[64] = { "" };
                ImGui::InputTextEx("##Search", "Search", search, 64, ImVec2(145, 35), NULL);

            }
            ImGui::EndGroup();

            ImGui::PopStyleVar(2);

            ImGui::GetWindowDrawList()->AddRectFilled(p + ImVec2(181 + i.x, i.y), p + ImVec2((c.x - i.x), (i.y + 58)), ImGui::GetColorU32(c::child::bg), c::child::rounding);

            ImGui::GetWindowDrawList()->AddImage(pic::logo, p + ImVec2((181 / 2) + (i.x / 2) + (40 / 2), i.y + 24), p + ImVec2((181 / 2) + (i.x / 2) - (40 / 2), ((i.y * 2) + 48)), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::accent));

            static int tabs = 0;
            const char* name_array[7] = { "Aimbot", "Moving", "Visuals", "Misc", "Player", "Configs", "Scripts" };
            IDirect3DTexture9* pic_array[7] = { pic::tab::aimbot, pic::tab::movement, pic::tab::visuals, pic::tab::misc, pic::tab::players, pic::tab::configs, pic::tab::scripts };
            ImVec2 pic_size[7] = { ImVec2(14, 14), ImVec2(14.74f, 14), ImVec2(14, 9.33f), ImVec2(14, 11.74f), ImVec2(8.75f, 14), ImVec2(13.4f, 14), ImVec2(15, 12) };

            ImGui::SetCursorPos(ImVec2((i.x * 2), (48 + (i.y * 3))));

            ImGui::BeginGroup();
            {

                for (int i = 0; i < 7; i++)
                    if (ImGui::Tab(i == tabs, pic_array[i], name_array[i], ImVec2(133 - ImGui::GetStyle().ItemSpacing.x, 44), pic_size[i])) tabs = i;

            }
            ImGui::EndGroup();

            static float tab_alpha = 0.f; /* */ static float tab_add; /* */ static int active_tab = 0;

            tab_alpha = ImClamp(tab_alpha + (4.f * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);
            if (tab_alpha == 0.f && tab_add == 0.f) active_tab = tabs;

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * ImGui::GetStyle().Alpha);

            ImGui::SetCursorPos(ImVec2((181 + i.x), (i.y * 2) + 58));

            ImGui::BeginGroup();
            {
                ImGui::BeginChild("Child One", ImVec2((c.x / 2) - ((181 / 2) + (i.x + 12)), c.y - (i.y * 3) - 58));
                {

                    static bool checkbox_e = true;
                    ImGui::Checkbox("Enabled checkbox", &checkbox_e);

                    static bool checkbox_d = false;
                    ImGui::Checkbox("Disabled checkbox", &checkbox_d);

                    static int slider_int = 96;
                    ImGui::SliderInt("Slider integer", &slider_int, 1, 100, "%d%%");

                    static float slider_float = 0.f;
                    ImGui::SliderFloat("Slider float", &slider_float, 0.f, 100.f, "%.1ff");

                    static bool checkkey = false;
                    static int key_check = 0, key_mode = 0;
                    ImGui::Keybindbox("Keybind example", "Tooltip example, could be a really big text.", &checkkey, &key_check, &key_mode);

                    static int key = 0, mode = 0;
                    ImGui::TooltipBind("Keybind", "Tooltip example, could be a really big text.", &key, &mode);

                    ImGui::ColorEdit4("Color picker", color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild("Child Two", ImVec2((c.x / 2) - ((181 / 2) + (i.x + 12)), (c.y - (i.y * 3) - 58)));
                {

                    static int select = 0;
                    const char* items[5]{ "One", "Two", "Three", "Four", "Five" };
                    ImGui::Combo("Combo", &select, items, IM_ARRAYSIZE(items), 5, ImGui::GetContentRegionMax().x - ImGui::GetStyle().WindowPadding.x);

                    static bool multi_num[5] = { false, true, true, true, false };
                    const char* multi_items[5] = { "One", "Two", "Three", "Four", "Five" };
                    ImGui::MultiCombo("Multi combo", multi_num, multi_items, 5, ImGui::GetContentRegionMax().x - ImGui::GetStyle().WindowPadding.x);

                    static char input[64] = { "" };
                    ImGui::TextField("Text input", "Hello, this is a text input.", input, 64);
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::PopStyleVar();

        }
        ImGui::End();
    }

    ImGui::Render();

    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void CMenu::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}