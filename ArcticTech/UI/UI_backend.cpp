#include "UI.h"

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
        IDirect3DTexture9* antiaim = nullptr;
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

    D3DXCreateTextureFromFileInMemory(DirectXDevice, ui_logo, sizeof(ui_logo), &pic::logo);

    D3DXCreateTextureFromFileInMemory(DirectXDevice, aimbot, sizeof(aimbot), &pic::tab::aimbot);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, movement, sizeof(movement), &pic::tab::antiaim);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, visuals, sizeof(visuals), &pic::tab::visuals);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, misc, sizeof(misc), &pic::tab::misc);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, players, sizeof(players), &pic::tab::players);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, configs, sizeof(configs), &pic::tab::configs);
    D3DXCreateTextureFromFileInMemory(DirectXDevice, scripts, sizeof(scripts), &pic::tab::scripts);

    m_WindowSize = ImVec2(950, 750);
    m_ItemSpacing = ImVec2(24, 24);

    SetupUI();

    m_bIsInitialized = true;
}

void CMenu::Release() {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
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
        ImGui::GetStyle().ItemSpacing = ImVec2(24, 24);
        ImGui::GetStyle().WindowPadding = ImVec2(0, 0);
        ImGui::GetStyle().ScrollbarSize = 8.f;

        ImGui::SetNextWindowSize(m_WindowSize);

        ImGui::Begin("MENU", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
        {
            const ImVec2& window_pos = ImGui::GetWindowPos();
            const ImVec2& window_size = ImGui::GetContentRegionMax();
            const ImVec2& item_spacing = ImGui::GetStyle().ItemSpacing;

            m_WindowSize = window_size;
            m_ItemSpacing = item_spacing;

            ImGui::GetBackgroundDrawList()->AddRectFilled(window_pos + ImVec2(0, 0), window_pos + ImVec2(window_size), ImGui::GetColorU32(c::background::bg), c::background::rounding);

            ImGui::GetWindowDrawList()->AddRectFilled(window_pos + ImVec2(item_spacing), window_pos + ImVec2(181, (window_size.y - item_spacing.y)), ImGui::GetColorU32(c::child::bg), c::child::rounding);

            ImGui::GetWindowDrawList()->AddRectFilled(window_pos + ImVec2(181 + item_spacing.x, item_spacing.y), window_pos + ImVec2((window_size.x - item_spacing.x), (item_spacing.y + 58)), ImGui::GetColorU32(c::child::bg), c::child::rounding);

            ImGui::GetWindowDrawList()->AddImage(pic::logo, window_pos + ImVec2(181 / 2 - 128 / 2 + item_spacing.x / 2, item_spacing.y - 16), window_pos + ImVec2(181 / 2 + 128 / 2 + item_spacing.x / 2, item_spacing.y - 16 + 128), ImVec2(0, 0), ImVec2(1, 1));

            static int tabs = 0;
            const char* name_array[7] = { "Aimbot", "Anti aim", "Player", "Visuals", "Misc", "Configs", "Scripts" };
            IDirect3DTexture9* pic_array[7] = { pic::tab::aimbot, pic::tab::antiaim, pic::tab::players, pic::tab::visuals, pic::tab::misc, pic::tab::configs, pic::tab::scripts };
            ImVec2 pic_size[7] = { ImVec2(14, 14), ImVec2(14.74f, 14), ImVec2(13.4f, 14), ImVec2(14, 9.33f), ImVec2(14, 11.74f), ImVec2(8.75f, 14), ImVec2(15, 12) };

            ImGui::SetCursorPos(ImVec2((item_spacing.x * 2), (48 + (item_spacing.y * 3))));

            ImGui::BeginGroup();
            {
                for (int i = 0; i < 7; i++)
                    if (ImGui::Tab(i == tabs, pic_array[i], name_array[i], ImVec2(133 - ImGui::GetStyle().ItemSpacing.x, 44), pic_size[i]))
                        tabs = i;
            }
            ImGui::EndGroup();

            static float tab_alpha = 0.f; 
            static float tab_add; 
            static int active_tab = 0;

            tab_alpha = ImClamp(tab_alpha + (4.f * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);
            if (tab_alpha == 0.f && tab_add == 0.f) 
                active_tab = tabs;

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * ImGui::GetStyle().Alpha);

            for (auto group : m_Groupboxes[active_tab]) {
                group->Render();
            }

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

void CMenu::RecalculateGroupboxes() {
    const float groupbox_width = (m_WindowSize.x / 2) - ((181 / 2) + (m_ItemSpacing.x + 12));
    const ImVec2 base_position((181 + m_ItemSpacing.x), (m_ItemSpacing.y * 2) + 58);
    const ImVec2 container_size(groupbox_width * 2 + m_ItemSpacing.x, m_WindowSize.y - base_position.y - m_ItemSpacing.y);

    for (int i = 0; i < 7; i++) {
        std::vector<CMenuGroupbox*>& groupboxes = m_Groupboxes[i];

        float total_relative[2] = { 0.f, 0.f };
        int n_groupboxes[2] = { 0, 0 };
        for (auto gb : groupboxes) {
            total_relative[gb->column] += gb->relative_size;
            n_groupboxes[gb->column]++;
        }

        float available_space[2] = { container_size.y - m_ItemSpacing.y * (n_groupboxes[0] - 1), container_size.y - m_ItemSpacing.y * (n_groupboxes[1] - 1) };
        float current_position[2] = { 0.f, 0.f };

        for (auto gb : groupboxes) {
            gb->position.y = base_position.y + current_position[gb->column];
            gb->position.x = base_position.x + (groupbox_width + m_ItemSpacing.x) * gb->column;
            gb->size.x = groupbox_width;
            gb->size.y = available_space[gb->column] * (gb->relative_size / total_relative[gb->column]);

            current_position[gb->column] += gb->size.y + m_ItemSpacing.y;
        }
    }
}

void CMenu::AddGroupBox(const std::string& tab, const std::string& groupbox, float realtive_size, int column) {
    int tab_id = 0;
    if (tab == "Anti aim")
        tab_id = 1;
    else if (tab == "Player")
        tab_id = 2;
    else if (tab == "Visuals")
        tab_id = 3;
    else if (tab == "Misc")
        tab_id = 4;
    else if (tab == "Config")
        tab_id = 5;
    else if (tab == "Scripts")
        tab_id = 6;

    CMenuGroupbox* gb = new CMenuGroupbox;

    if (column == -1) {
        int n_columns[2]{ 0, 0 };

        for (auto gb : m_Groupboxes[tab_id]) {
            n_columns[gb->column]++;
        }

        gb->column = (n_columns[0] <= n_columns[1]) ? 0 : 1;
    }
    else {
        gb->column = column;
    }

    gb->name = groupbox;

    m_Groupboxes[tab_id].emplace_back(gb);

    RecalculateGroupboxes();
}

void CMenuGroupbox::Render() {
    ImGui::SetCursorPos(position);

    ImGui::BeginGroup();
    ImGui::BeginChild(name.c_str(), size);

    for (auto el : widgets) {
        if (!el->visible || el->GetType() == WidgetType::ColorPicker || el->GetType() == WidgetType::KeyBind)
            continue;

        el->Render();
    }

    ImGui::EndChild();
    ImGui::EndGroup();
}

void CCheckBox::Render() {
    if (ImGui::Checkbox(name.c_str(), &value)) {
        for (auto& cb : callbacks)
            cb();
        for (auto lcb : lua_callbacks)
            lcb();
    }

    if (additional) {
        ImGui::SetCursorPos(ImGui::GetCursorPos() - ImVec2(0, (ImGui::GetStyle().ItemSpacing.y * 2) + 4));
        additional->Render();
    }
}

void CSliderInt::Render() {
    if (ImGui::SliderInt(name.c_str(), &value, min, max, format.c_str(), flags)) {
        for (auto& cb : callbacks)
            cb();
        for (auto lcb : lua_callbacks)
            lcb();
    }
}

void CSliderFloat::Render() {
    if (ImGui::SliderFloat(name.c_str(), &value, min, max, format.c_str(), flags)) {
        for (auto& cb : callbacks)
            cb();
        for (auto lcb : lua_callbacks)
            lcb();
    }
}

void CKeyBind::Render() {
    if (ImGui::Keybind(name.c_str(), &key, &mode, false)) {
        for (auto& cb : callbacks)
            cb();
        for (auto lcb : lua_callbacks)
            lcb();
    }
}

void CLabel::Render() {
    ImGui::Text(name.c_str());

    if (additional) {
        ImGui::SetCursorPos(ImGui::GetCursorPos() - ImVec2(0, (ImGui::GetStyle().ItemSpacing.y * 2) + 4));
        additional->Render();
    }
}

void CColorPicker::Render() {
    if (ImGui::ColorEdit4(name.c_str(), value, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | (has_alpha ? 0 : ImGuiColorEditFlags_NoAlpha))) {
        for (auto& cb : callbacks)
            cb();
        for (auto lcb : lua_callbacks)
            lcb();
    }
}

void CComboBox::Render() {
    if (ImGui::Combo(name.c_str(), &value, elements.data(), static_cast<int>(elements.size()), 5, ImGui::GetContentRegionMax().x - ImGui::GetStyle().WindowPadding.x)) {
        for (auto& cb : callbacks)
            cb();
        for (auto lcb : lua_callbacks)
            lcb();
    }
}

void CMultiCombo::Render() {
    if (ImGui::MultiCombo(name.c_str(), value, elements.data(), elements.size(), ImGui::GetContentRegionMax().x - ImGui::GetStyle().WindowPadding.x)) {
        for (auto& cb : callbacks)
            cb();
        for (auto lcb : lua_callbacks)
            lcb();
    }
}

void CButton::Render() {
    if (ImGui::Button(name.c_str())) {
        for (auto& cb : callbacks)
            cb();
        for (auto lcb : lua_callbacks)
            lcb();
    }
}

void CInputBox::Render() {
    if (ImGui::TextField(name.c_str(), nullptr, buf, 64, flags)) {
        for (auto& cb : callbacks)
            cb();
        for (auto lcb : lua_callbacks)
            lcb();
    }
}

CMenuGroupbox* CMenu::FindGroupbox(const std::string& tab, const std::string& groupbox) {
    int tab_id = 0;
    if (tab == "Anti aim")
        tab_id = 1;
    else if (tab == "Player")
        tab_id = 2;
    else if (tab == "Visuals")
        tab_id = 3;
    else if (tab == "Misc")
        tab_id = 4;
    else if (tab == "Config")
        tab_id = 5;
    else if (tab == "Scripts")
        tab_id = 6;

    for (auto gb : m_Groupboxes[tab_id]) {
        if (gb->name == groupbox)
            return gb;
    }

    return nullptr;
}

IBaseWidget* CMenu::FindItem(const std::string& tab, const std::string& groupbox, const std::string& name, WidgetType type) {
    CMenuGroupbox* gb = FindGroupbox(tab, groupbox);

    if (!gb)
        return nullptr;

    for (auto item : gb->widgets)
        if (item->name == name && (type == WidgetType::Any || type == item->GetType()))
            return item;

    return nullptr;
}

CCheckBox* CMenu::AddCheckBox(const std::string& tab, const std::string& groupbox, const std::string& name, bool init) {
    CMenuGroupbox* gb = FindGroupbox(tab, groupbox);

    if (!gb)
        return nullptr;

    CCheckBox* item = new CCheckBox;

    item->name = name;
    item->parent = gb;
    item->value = init;

    gb->widgets.emplace_back(item);

    return item;
}

CSliderInt* CMenu::AddSliderInt(const std::string& tab, const std::string& groupbox, const std::string& name, int min, int max, int init, const std::string& format, ImGuiSliderFlags flags) {
    CMenuGroupbox* gb = FindGroupbox(tab, groupbox);

    if (!gb)
        return nullptr;

    CSliderInt* item = new CSliderInt;

    item->name = name;
    item->parent = gb;
    item->min = min;
    item->max = max;
    item->value = init;
    item->format = format;
    item->flags = flags;

    gb->widgets.emplace_back(item);

    return item;
}

CSliderFloat* CMenu::AddSliderFloat(const std::string& tab, const std::string& groupbox, const std::string& name, float min, float max, float init, const std::string& format, ImGuiSliderFlags flags) {
    CMenuGroupbox* gb = FindGroupbox(tab, groupbox);

    if (!gb)
        return nullptr;

    CSliderFloat* item = new CSliderFloat;

    item->name = name;
    item->parent = gb;
    item->min = min;
    item->max = max;
    item->value = init;
    item->format = format;
    item->flags = flags;

    gb->widgets.emplace_back(item);

    return item;
}

CKeyBind* CMenu::AddKeyBind(const std::string& tab, const std::string& groupbox, const std::string& name) {
    IBaseWidget* parent_item = FindItem(tab, groupbox, name);

    if (!parent_item)
        parent_item = AddLabel(tab, groupbox, name);

    CKeyBind* item = new CKeyBind;

    item->name = name;
    item->parent = parent_item->parent;
    parent_item->additional = item;

    item->parent->widgets.emplace_back(item);

    return item;
}

CLabel* CMenu::AddLabel(const std::string& tab, const std::string& groupbox, const std::string& name) {
    CMenuGroupbox* gb = FindGroupbox(tab, groupbox);

    if (!gb)
        return nullptr;

    CLabel* item = new CLabel;

    item->name = name;
    item->parent = gb;

    gb->widgets.emplace_back(item);

    return item;
}

CColorPicker* CMenu::AddColorPicker(const std::string& tab, const std::string& groupbox, const std::string& name, Color init, bool has_alpha) {
    IBaseWidget* parent_item = FindItem(tab, groupbox, name);

    if (!parent_item)
        parent_item = AddLabel(tab, groupbox, name);

    CColorPicker* item = new CColorPicker;

    item->name = name;
    item->parent = parent_item->parent;
    parent_item->additional = item;
    item->value[0] = init.r / 255.f;
    item->value[1] = init.g / 255.f;
    item->value[2] = init.b / 255.f;
    item->value[3] = init.a / 255.f;
    item->has_alpha = has_alpha;

    item->parent->widgets.emplace_back(item);

    return item;
}

CComboBox* CMenu::AddComboBox(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<const char*> items) {
    CMenuGroupbox* gb = FindGroupbox(tab, groupbox);

    if (!gb)
        return nullptr;

    CComboBox* item = new CComboBox;

    item->name = name;
    item->parent = gb;
    item->elements = items;

    gb->widgets.emplace_back(item);
}

CMultiCombo* CMenu::AddMultiCombo(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<const char*> items) {
    CMenuGroupbox* gb = FindGroupbox(tab, groupbox);

    if (!gb)
        return nullptr;

    CMultiCombo* item = new CMultiCombo;

    item->name = name;
    item->parent = gb;
    item->elements = items;
    item->value = new bool[items.size()];

    gb->widgets.emplace_back(item);

    return item;
}

CButton* CMenu::AddButton(const std::string& tab, const std::string& groupbox, const std::string& name) {
    CMenuGroupbox* gb = FindGroupbox(tab, groupbox);

    if (!gb)
        return nullptr;

    CButton* item = new CButton;

    item->name = name;
    item->parent = gb;

    gb->widgets.emplace_back(item);

    return item;
}

CInputBox* CMenu::AddInput(const std::string& tab, const std::string& groupbox, const std::string& name, const std::string& init, ImGuiInputTextFlags flags) {
    CMenuGroupbox* gb = FindGroupbox(tab, groupbox);

    if (!gb)
        return nullptr;

    CInputBox* item = new CInputBox;

    item->name = name;
    item->parent = gb;
    item->flags = flags;
    
    ZeroMemory(item->buf, 64);
    std::memcpy(item->buf, init.c_str(), init.size());

    gb->widgets.emplace_back(item);

    return item;
}