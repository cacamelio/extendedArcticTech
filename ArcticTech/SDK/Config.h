#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>

#include "../Menu/menu.h"
#include "../Utils/json.hpp"

struct weapon_settings_t {
    std::string weapon_name;

    CMultiComboOld* hitboxes{ nullptr };
    CMultiComboOld* multipoints{ nullptr };
    CSliderOld* head_point_scale{ nullptr };
    CSliderOld* body_point_scale{ nullptr };
    CSliderOld* hitchance{ nullptr };
    CSliderOld* minimum_damage{ nullptr };
    CSliderOld* minimum_damage_override{ nullptr };
    CMultiComboOld* auto_stop{ nullptr };
    CCheckboxOld* auto_scope{ nullptr };

    weapon_settings_t() {}
    weapon_settings_t(const std::string& name) : weapon_name(name) {}

    inline void set_visible(bool visible) {
        hitboxes->set_visible(visible);
        multipoints->set_visible(visible);
        head_point_scale->set_visible(visible);
        body_point_scale->set_visible(visible);
        hitchance->set_visible(visible);
        minimum_damage->set_visible(visible);
        minimum_damage_override->set_visible(visible);
        auto_stop->set_visible(visible);
        auto_scope->set_visible(visible);
    };
};

struct config_t {
    struct ragebot_t {
        struct aimbot_t {
            CCheckboxOld* enabled;
            CComboBoxOld* extrapolation;
            CCheckboxOld* doubletap;
            CKeyBindOld* doubletap_key;
            CSliderOld* doubletap_speed;
            CCheckboxOld* defensive_doubletap;
            CKeyBindOld* force_teleport;
            CKeyBindOld* force_body_aim;
            CKeyBindOld* minimum_damage_override_key;
            CCheckboxOld* peek_assist;
            CColorPickerOld* peek_assist_color;
            CKeyBindOld* peek_assist_keybind;
            CMultiComboOld* low_fps_mitigations;
            CCheckboxOld* show_aimpoints;
            CSliderOld* resolver_treshold;
            CCheckboxOld* show_debug_data;
            CCheckboxOld* threads;
        } aimbot;

        CComboBoxOld* selected_weapon;

        struct weapons_t {
            weapon_settings_t global{ "Global" };
            weapon_settings_t awp{ "AWP" };
            weapon_settings_t autosniper{ "Auto" };
            weapon_settings_t scout{ "Scout" };
            weapon_settings_t deagle{ "Deagle" };
            weapon_settings_t pistol{ "Pistol" };
        } weapons;
    } ragebot;

    struct antiaim_t {
        struct anti_aimbot_angles_t {
            CComboBoxOld* pitch;
            CComboBoxOld* yaw;
            CCheckboxOld* yaw_jitter;
            CSliderOld* modifier_value;
            CKeyBindOld* manual_left;
            CKeyBindOld* manual_right;
            CCheckboxOld* body_yaw;
            CMultiComboOld* body_yaw_options;
            CSliderOld* body_yaw_limit;
            CKeyBindOld* inverter;
        } anti_aimbot_angles;

        struct fakelag_t {
            CCheckboxOld* enabled;
            CSliderOld* limit;
            CSliderOld* variability;
            CMultiComboOld* triggers;
        } fakelag;

        struct misc_t {
            CKeyBindOld* fake_duck;
            CKeyBindOld* slow_walk;
            CComboBoxOld* slow_walk_type;
            CSliderOld* custom_slow_walk;
            CMultiComboOld* animations;
            CComboBoxOld* leg_movement;
        } misc;
    } antiaim;

    struct visuals_t {
        struct esp_t {
            CCheckboxOld* enable;
            CCheckboxOld* dormant;
            CColorPickerOld* dormant_color;
            CCheckboxOld* dynamic_box;
            CCheckboxOld* bounding_box;
            CColorPickerOld* box_color;
            CCheckboxOld* health_bar;
            CCheckboxOld* custom_health;
            CColorPickerOld* custom_health_color;
            CCheckboxOld* name;
            CColorPickerOld* name_color;
            CMultiComboOld* flags;
            CCheckboxOld* weapon_text;
            CColorPickerOld* weapon_text_color;
            CCheckboxOld* weapon_icon;
            CColorPickerOld* weapon_icon_color;
            CCheckboxOld* ammo;
            CColorPickerOld* ammo_color;
            CCheckboxOld* glow;
            CColorPickerOld* glow_color;
            CCheckboxOld* hitsound;
            CCheckboxOld* show_server_hitboxes;
            CCheckboxOld* shared_esp;
            CCheckboxOld* share_with_enemies;
            CCheckboxOld* hitmarker;
            CColorPickerOld* hitmarker_color;
            CCheckboxOld* damage_marker;
            CColorPickerOld* damage_marker_color;
        } esp;

        struct chams_t {
            CCheckboxOld* enemy;
            CColorPickerOld* enemy_color;
            CColorPickerOld* enemy_second_color;
            CCheckboxOld* enemy_invisible;
            CColorPickerOld* enemy_invisible_color;
            CComboBoxOld* enemy_type;
            CSliderOld* enemy_glow_thickness;

            CCheckboxOld* shot_chams;
            CColorPickerOld* shot_chams_color;
            CComboBoxOld* shot_chams_type;
            CColorPickerOld* shot_chams_second_color;
            CSliderOld* shot_chams_thickness;
            CSliderOld* shot_chams_duration;

            CCheckboxOld* local_player;
            CColorPickerOld* local_player_color;
            CComboBoxOld* local_player_type;
            CColorPickerOld* local_player_second_color;
            CSliderOld* local_glow_thickness;

            CCheckboxOld* attachments;
            CColorPickerOld* attachments_color;
            CComboBoxOld* attachments_type;
            CColorPickerOld* attachments_second_color;
            CSliderOld* attachments_glow_thickness;

            CCheckboxOld* viewmodel;
            CColorPickerOld* viewmodel_color;
            CComboBoxOld* viewmodel_type;
            CColorPickerOld* viewmodel_second_color;
            CSliderOld* viewmodel_glow_thickness;

            CSliderOld* scope_blend;
            CCheckboxOld* disable_model_occlusion;
        } chams;

        struct other_esp_t {
            CCheckboxOld* radar;
            CMultiComboOld* dropped_weapons;
            CColorPickerOld* dropped_weapons_color;
            CCheckboxOld* sniper_crosshair;
            CCheckboxOld* penetration_crosshair;
            CCheckboxOld* bomb;
            CColorPickerOld* bomb_color;
            CCheckboxOld* grenades;
            CCheckboxOld* molotov_radius;
            CColorPickerOld* molotov_radius_color;
            CCheckboxOld* grenade_trajecotry;
            CColorPickerOld* grenade_trajectory_color;
            CColorPickerOld* grenade_trajectory_hit_color;
            CCheckboxOld* grenade_proximity_warning;
            CColorPickerOld* grenade_predict_color;
            CMultiComboOld* particles;
            CMultiComboOld* indicators;
        } other_esp;

        struct effects_t {
            CSliderOld* fov;
            CMultiComboOld* removals;
            CCheckboxOld* world_color_enable;
            CColorPickerOld* world_color;
            CCheckboxOld* props_color_enable;
            CColorPickerOld* props_color;
            CCheckboxOld* thirdperson;
            CKeyBindOld* thirdperson_bind;
            CSliderOld* thirdperson_distance;
            CSliderOld* aspect_ratio;
            CCheckboxOld* client_impacts;
            CColorPickerOld* client_impacts_color;
            CCheckboxOld* server_impacts;
            CColorPickerOld* server_impacts_color;
            CSliderOld* impacts_duration;
            CComboBoxOld* override_skybox;
            CCheckboxOld* override_fog;
            CColorPickerOld* fog_color;
            CSliderOld* fog_start;
            CSliderOld* fog_end;
            CSliderOld* fog_density;
            CCheckboxOld* preserve_killfeed;
            CMultiComboOld* optimizations;
            CCheckboxOld* custom_sun_direction;
            CSliderOld* sun_pitch;
            CSliderOld* sun_yaw;
            CSliderOld* sun_distance;
            CSliderOld* viewmodel_scope_alpha;
        } effects;
    } visuals;

    struct misc_t {
        struct miscellaneous_t {
            CCheckboxOld* anti_untrusted;
            CMultiComboOld* logs;
            CMultiComboOld* auto_buy;
            CCheckboxOld* filter_console;
            CCheckboxOld* clantag;
            CCheckboxOld* ad_block;
        } miscellaneous;

        struct movement_t {
            CCheckboxOld* auto_jump;
            CCheckboxOld* auto_strafe;
            CCheckboxOld* compensate_throwable;
            CSliderOld* auto_strafe_smooth;
            CCheckboxOld* edge_jump;
            CKeyBindOld* edge_jump_key;
            CCheckboxOld* infinity_duck;
            CCheckboxOld* quick_stop;
        } movement;
    } misc;

    struct skinchanger_t {
        CCheckboxOld* override_knife;
        CCheckboxOld* override_agent;
        CComboBoxOld* knife_model;
        CComboBoxOld* agent_model_ct;
        CComboBoxOld* agent_model_t;
    } skins;
};

extern config_t config;

struct config_item_t {
    IBaseElement* item;
    std::string name;
};

class CConfig {
    std::vector<config_item_t> items;

    inline void add(IBaseElement* item) { 
        items.emplace_back(config_item_t{ item, item->parent->name + "_" + item->name + "_" + std::to_string(item->GetItemType()) });
    };

public:
    CListBox* lua_list;
    CButtonOld* lua_button;
    CButtonOld* lua_button_unload;
    CButtonOld* lua_refresh;
    CButtonOld* lua_save;

    CListBox* config_list;
    CInputBoxOld* config_name;
    CButtonOld* load_button;
    CButtonOld* save_button;
    CButtonOld* delete_button;
    CButtonOld* import_button;
    CButtonOld* export_button;

    void parse(nlohmann::json& cfg);
    nlohmann::json dump();

    CConfig() {
        std::filesystem::create_directory(std::filesystem::current_path().string() + "/at");
    }

    std::vector<std::string> get_all_configs() {
        std::vector<std::string> result;

        for (const auto& file : std::filesystem::directory_iterator(std::filesystem::current_path().string() + "/at"))
            if (file.path().extension() == ".cfg")
                result.push_back(file.path().stem().string());

        return result;
    }

    void Init();
};

extern CConfig* Config;