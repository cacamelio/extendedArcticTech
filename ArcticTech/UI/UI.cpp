#include "UI.h"
#include "../SDK/Config.h"

#include "../Features/Visuals/SkinChanger.h"
#include "../Features/Visuals/World.h"

void CMenu::SetupUI() {
	AddGroupBox("Aimbot", "Aimbot");
	AddGroupBox("Aimbot", "Settings");

	AddGroupBox("Anti aim", "Angles");
	AddGroupBox("Anti aim", "Fake lag", 1.f, 1);
	AddGroupBox("Anti aim", "Other", 1.f, 1);

	AddGroupBox("Player", "ESP");
	AddGroupBox("Player", "Chams");

	AddGroupBox("Visuals", "Other ESP");
	AddGroupBox("Visuals", "Effects");

	AddGroupBox("Misc", "Miscellaneous");
	AddGroupBox("Misc", "Movement");

	AddGroupBox("Skins", "Skins");
	AddGroupBox("Skins", "Models");

	AddGroupBox("Config", "Config");

	AddGroupBox("Scripts", "Scripts");

	config.ragebot.aimbot.enabled = AddCheckBox("Aimbot", "Aimbot", "Enabled");
	config.ragebot.aimbot.extrapolation = AddComboBox("Aimbot", "Aimbot", "Extrapolation", { "Disable", "Enable", "Force" });
	config.ragebot.aimbot.doubletap = AddCheckBox("Aimbot", "Aimbot", "Double Tap");
	config.ragebot.aimbot.doubletap_key = AddKeyBind("Aimbot", "Aimbot", "Double Tap");
	config.ragebot.aimbot.defensive_doubletap = AddCheckBox("Aimbot", "Aimbot", "Lag Peek");
	config.ragebot.aimbot.force_teleport = AddKeyBind("Aimbot", "Aimbot", "Force Teleport");
	config.ragebot.aimbot.force_body_aim = AddKeyBind("Aimbot", "Aimbot", "Force Body Aim");
	config.ragebot.aimbot.minimum_damage_override_key = AddKeyBind("Aimbot", "Aimbot", "Min. damage override");
	config.ragebot.aimbot.peek_assist = AddCheckBox("Aimbot", "Aimbot", "Peek Assist");
	config.ragebot.aimbot.peek_assist_color = AddColorPicker("Aimbot", "Aimbot", "Peek Assist");
	config.ragebot.aimbot.peek_assist_keybind = AddKeyBind("Aimbot", "Aimbot", "Peek Assist key");
	config.ragebot.aimbot.show_aimpoints = AddCheckBox("Aimbot", "Aimbot", "Show aim points");
	config.ragebot.aimbot.resolver_treshold = AddSliderInt("Aimbot", "Aimbot", "Resolver treshold", 0, 100, 50, "%.d%%");
	config.ragebot.aimbot.show_debug_data = AddCheckBox("Aimbot", "Aimbot", "Show debug data");
	config.ragebot.aimbot.threads = AddCheckBox("Aimbot", "Aimbot", "Multithread");

	config.ragebot.selected_weapon = AddComboBox("Aimbot", "Settings", "Current Weapon", { "Global", "AWP", "Autosniper", "Scout", "Deagle", "Pistol" });

	auto setup_weapon_config = [this](weapon_settings_t& settings) {
		settings.hitboxes = AddMultiCombo("Aimbot", "Settings", std::format("[{}] {}", settings.weapon_name, "Hitbox"), { "Head", "Chest", "Stomach", "Arms", "Legs", "Feet" });
		settings.multipoints = AddMultiCombo("Aimbot", "Settings", std::format("[{}] {}", settings.weapon_name, "Multipoints"), { "Head", "Chest", "Stomach" });
		settings.head_point_scale = AddSliderInt("Aimbot", "Settings", std::format("[{}] {}", settings.weapon_name, "Head scale"), 0, 100, 50, "%");
		settings.body_point_scale = AddSliderInt("Aimbot", "Settings", std::format("[{}] {}", settings.weapon_name, "Body scale"), 0, 100, 50, "%");
		settings.hitchance = AddSliderInt("Aimbot", "Settings", std::format("[{}] {}", settings.weapon_name, "Hitchance"), 0, 100, 50, "%");
		settings.minimum_damage = AddSliderInt("Aimbot", "Settings", std::format("[{}] {}", settings.weapon_name, "Minimum damage"), 1, 130, 30);
		settings.minimum_damage_override = AddSliderInt("Aimbot", "Settings", std::format("[{}] {}", settings.weapon_name, "Minimum damage override"), 1, 130, 10);
		settings.auto_stop = AddMultiCombo("Aimbot", "Settings", std::format("[{}] {}", settings.weapon_name, "Auto stop"), { "Full stop", "Early", "Move between shots" });
		settings.auto_scope = AddCheckBox("Aimbot", "Settings", std::format("[{}] {}", settings.weapon_name, "Auto scope"));
	};

	setup_weapon_config(config.ragebot.weapons.global);
	setup_weapon_config(config.ragebot.weapons.awp);
	setup_weapon_config(config.ragebot.weapons.autosniper);
	setup_weapon_config(config.ragebot.weapons.scout);
	setup_weapon_config(config.ragebot.weapons.deagle);
	setup_weapon_config(config.ragebot.weapons.pistol);

	config.antiaim.angles.pitch = AddComboBox("Anti aim", "Angles", "Pitch", { "Disabled", "Down" });
	config.antiaim.angles.yaw = AddComboBox("Anti aim", "Angles", "Yaw", { "Forward", "Backward", "At target" });
	config.antiaim.angles.yaw_jitter = AddCheckBox("Anti aim", "Angles", "Yaw jitter");
	config.antiaim.angles.modifier_value = AddSliderInt("Anti aim", "Angles", "Modifier value", -180, 180, 0, "%d°");
	config.antiaim.angles.manual_left = AddKeyBind("Anti aim", "Angles", "Manual left");
	config.antiaim.angles.manual_right = AddKeyBind("Anti aim", "Angles", "Manual right");
	config.antiaim.angles.body_yaw = AddCheckBox("Anti aim", "Angles", "Body yaw");
	config.antiaim.angles.body_yaw_options = AddMultiCombo("Anti aim", "Angles", "Body yaw options", { "Jitter", "Random jitter", "Extended", "Freestand" });
	config.antiaim.angles.body_yaw_limit = AddSliderInt("Anti aim", "Angles", "Limit", 0, 58, 58, "%d°");
	config.antiaim.angles.inverter = AddKeyBind("Anti aim", "Angles", "Inverter");

	config.antiaim.fakelag.enabled = AddCheckBox("Anti aim", "Fake lag", "Enabled");
	config.antiaim.fakelag.limit = AddSliderInt("Anti aim", "Fake lag", "Limit", 1, 15, 13);
	config.antiaim.fakelag.variability = AddSliderInt("Anti aim", "Fake lag", "Variabaility", 1, 14, 1);
	config.antiaim.fakelag.triggers = AddMultiCombo("Anti aim", "Fake lag", "Triggers", { "Move", "Air", "Break LC", "Peek" });

	config.antiaim.misc.fake_duck = AddKeyBind("Anti aim", "Other", "Fake duck");
	config.antiaim.misc.slow_walk = AddKeyBind("Anti aim", "Other", "Slow walk");
	config.antiaim.misc.slow_walk_type = AddComboBox("Anti aim", "Other", "Type", { "Force accuracy", "Custom" });
	config.antiaim.misc.custom_slow_walk = AddSliderInt("Anti aim", "Other", "Custom speed", 0, 100, 50, "%d%");
	config.antiaim.misc.animations = AddMultiCombo("Anti aim", "Other", "Animations", { "Lean", "Static legs in air", "Backward legs" });
	config.antiaim.misc.leg_movement = AddComboBox("Anti aim", "Other", "Leg movement", { "Default", "Sliding", "Walking" });

	config.visuals.esp.enable = AddCheckBox("Player", "ESP", "Enable");
	config.visuals.esp.dormant = AddCheckBox("Player", "ESP", "Dormant");
	config.visuals.esp.dormant_color = AddColorPicker("Player", "ESP", "Dormant", Color(255, 255, 255, 200));
	config.visuals.esp.bounding_box = AddCheckBox("Player", "ESP", "Bounding box");
	config.visuals.esp.box_color = AddColorPicker("Player", "ESP", "Bounding box");
	config.visuals.esp.health_bar = AddCheckBox("Player", "ESP", "Health bar");
	config.visuals.esp.custom_health = AddCheckBox("Player", "ESP", "Custom health");
	config.visuals.esp.custom_health_color = AddColorPicker("Player", "ESP", "Custom health");
	config.visuals.esp.name = AddCheckBox("Player", "ESP", "Name");
	config.visuals.esp.name_color = AddColorPicker("Player", "ESP", "Name");
	config.visuals.esp.flags = AddMultiCombo("Player", "ESP", "Flags", { "Armor", "Zoom", "Fake duck", "Exploit", "Break LC", "Bomb", "Resolver" });
	config.visuals.esp.weapon_text = AddCheckBox("Player", "ESP", "Weapon text");
	config.visuals.esp.weapon_text_color = AddColorPicker("Player", "ESP", "Weapon text");
	config.visuals.esp.weapon_icon = AddCheckBox("Player", "ESP", "Weapon icon");
	config.visuals.esp.weapon_icon_color = AddColorPicker("Player", "ESP", "Weapon icon");
	config.visuals.esp.ammo = AddCheckBox("Player", "ESP", "Ammo");
	config.visuals.esp.ammo_color = AddColorPicker("Player", "ESP", "Ammo", Color(80, 140, 200));
	config.visuals.esp.glow = AddCheckBox("Player", "ESP", "Glow");
	config.visuals.esp.glow_color = AddColorPicker("Player", "ESP", "Glow", Color(180, 60, 120));
	config.visuals.esp.hitsound = AddCheckBox("Player", "ESP", "Hit sound");
	config.visuals.esp.show_server_hitboxes = AddCheckBox("Player", "ESP", "Show sever hitboxes");
	config.visuals.esp.shared_esp = AddCheckBox("Player", "ESP", "Shared ESP");
	config.visuals.esp.share_with_enemies = AddCheckBox("Player", "ESP", "Share with enemies");
	config.visuals.esp.hitmarker = AddCheckBox("Player", "ESP", "Hitmarker");
	config.visuals.esp.hitmarker_color = AddColorPicker("Player", "ESP", "Hitmarker");
	config.visuals.esp.damage_marker = AddCheckBox("Player", "ESP", "Damage marker");
	config.visuals.esp.damage_marker_color = AddColorPicker("Player", "ESP", "Damage marker");

	config.visuals.chams.enemy = AddCheckBox("Player", "Chams", "Player");
	config.visuals.chams.enemy_invisible = AddCheckBox("Player", "Chams", "Player behind wall");
	config.visuals.chams.enemy_color = AddColorPicker("Player", "Chams", "Player", Color(150, 190, 70));
	config.visuals.chams.enemy_invisible_color = AddColorPicker("Player", "Chams", "Player behind wall", Color(60, 120, 160));
	config.visuals.chams.enemy_type = AddComboBox("Player", "Chams", "Enemy type", { "Default", "Solid", "Glow", "Glow overlay" });
	config.visuals.chams.enemy_second_color = AddColorPicker("Player", "Chams", "Enemy type");
	config.visuals.chams.enemy_glow_thickness = AddSliderFloat("Player", "Chams", "Enemy glow thickness", 0.1f, 9.f, 1.f, "%.1f");
	config.visuals.chams.shot_chams = AddCheckBox("Player", "Chams", "Shot");
	config.visuals.chams.shot_chams_color = AddColorPicker("Player", "Chams", "Shot", Color(100, 100, 100));
	config.visuals.chams.shot_chams_type = AddComboBox("Player", "Chams", "Shot type", { "Default", "Solid", "Glow", "Glow overlay" });
	config.visuals.chams.shot_chams_second_color = AddColorPicker("Player", "Chams", "Shot type");
	config.visuals.chams.shot_chams_thickness = AddSliderFloat("Player", "Chams", "Shot glow thickness", 0.1f, 9.f, 1.f, "%.1f");
	config.visuals.chams.shot_chams_duration = AddSliderInt("Player", "Chams", "Shot chams duration", 1, 10, 4, "s");
	config.visuals.chams.local_player = AddCheckBox("Player", "Chams", "Local player");
	config.visuals.chams.local_player_color = AddColorPicker("Player", "Chams", "Local player", Color(100, 100, 100));
	config.visuals.chams.local_player_type = AddComboBox("Player", "Chams", "Local type", { "Default", "Solid", "Glow", "Glow overlay" });
	config.visuals.chams.local_player_second_color = AddColorPicker("Player", "Chams", "Local type");
	config.visuals.chams.local_glow_thickness = AddSliderFloat("Player", "Chams", "Local glow thickness", 0.1f, 9.f, 1.f, "%.1f");
	config.visuals.chams.attachments = AddCheckBox("Player", "Chams", "Attachments");
	config.visuals.chams.attachments_color = AddColorPicker("Player", "Chams", "Attachments");
	config.visuals.chams.attachments_type = AddComboBox("Player", "Chams", "Attachments type", { "Default", "Solid", "Glow", "Glow overlay" });
	config.visuals.chams.attachments_second_color = AddColorPicker("Player", "Chams", "Attachments type");
	config.visuals.chams.attachments_glow_thickness = AddSliderFloat("Player", "Chams", "Attachments glow thickness", 0.1f, 9.f, 1.f, "%.1f");
	config.visuals.chams.viewmodel = AddCheckBox("Player", "Chams", "Viewmodel");
	config.visuals.chams.viewmodel_color = AddColorPicker("Player", "Chams", "Viewmodel");
	config.visuals.chams.viewmodel_type = AddComboBox("Player", "Chams", "Viewmodel type", { "Default", "Solid", "Glow", "Glow overlay" });
	config.visuals.chams.viewmodel_second_color = AddColorPicker("Player", "Chams", "Viewmodel type");
	config.visuals.chams.viewmodel_glow_thickness = AddSliderFloat("Player", "Chams", "Viewmodel glow thickness", 0.1f, 9.f, 1.f, "%.1f");
	config.visuals.chams.scope_blend = AddSliderInt("Player", "Chams", "Scope blend", 0, 100, 100, "%d%%");
	config.visuals.chams.disable_model_occlusion = AddCheckBox("Player", "Chams", "Disable model occlusion");

	config.visuals.other_esp.radar = AddCheckBox("Visuals", "Other ESP", "Radar");
	config.visuals.other_esp.dropped_weapons = AddMultiCombo("Visuals", "Other ESP", "Dropped weapons", { "Text", "Icon", "Glow" });
	config.visuals.other_esp.dropped_weapons_color = AddColorPicker("Visuals", "Other ESP", "Dropped weapons");
	config.visuals.other_esp.sniper_crosshair = AddCheckBox("Visuals", "Other ESP", "Sniper crosshair");
	config.visuals.other_esp.penetration_crosshair = AddCheckBox("Visuals", "Other ESP", "Penetration crosshair");
	config.visuals.other_esp.bomb = AddCheckBox("Visuals", "Other ESP", "Bomb");
	config.visuals.other_esp.bomb_color = AddColorPicker("Visuals", "Other ESP", "Bomb", Color(150, 200, 60));
	config.visuals.other_esp.grenades = AddCheckBox("Visuals", "Other ESP", "Grenades");
	config.visuals.other_esp.molotov_radius = AddCheckBox("Visuals", "Other ESP", "Molotov radius");
	config.visuals.other_esp.molotov_radius_color = AddColorPicker("Visuals", "Other ESP", "Molotov radius", Color(255, 0, 0));
	config.visuals.other_esp.grenade_trajecotry = AddCheckBox("Visuals", "Other ESP", "Grenade trajectory");
	config.visuals.other_esp.grenade_trajectory_color = AddColorPicker("Visuals", "Other ESP", "Grenade trajectory", Color(250, 60, 60));
	config.visuals.other_esp.grenade_trajectory_hit_color = AddColorPicker("Visuals", "Other ESP", "Grenade trajectory (hit)", Color(150, 200, 60));
	config.visuals.other_esp.grenade_proximity_warning = AddCheckBox("Visuals", "Other ESP", "Grenade proximity warning");
	config.visuals.other_esp.grenade_predict_color = AddColorPicker("Visuals", "Other ESP", "Grenade predict color");
	config.visuals.other_esp.particles = AddMultiCombo("Visuals", "Other ESP", "Particles", { "Molotov", "Smoke" });
	config.visuals.other_esp.indicators = AddMultiCombo("Visuals", "Other ESP", "Indicators", { "Double tap", "Min. damage" });

	config.visuals.effects.fov = AddSliderInt("Visuals", "Effects", "Field of view", 80, 130, 90);
	config.visuals.effects.removals = AddMultiCombo("Visuals", "Effects", "Removals", { "Post effects", "Fog", "Shadows", "Smoke", "Flashbang", "Scope", "Blood", "Sprites" });
	config.visuals.effects.world_color_enable = AddCheckBox("Visuals", "Effects", "World color");
	config.visuals.effects.world_color = AddColorPicker("Visuals", "Effects", "World color");
	config.visuals.effects.props_color_enable = AddCheckBox("Visuals", "Effects", "Props color");
	config.visuals.effects.props_color = AddColorPicker("Visuals", "Effects", "Props color");
	config.visuals.effects.thirdperson = AddCheckBox("Visuals", "Effects", "Force thirdperson");
	config.visuals.effects.thirdperson_bind = AddKeyBind("Visuals", "Effects", "Force thirdperson");
	config.visuals.effects.thirdperson_distance = AddSliderInt("Visuals", "Effects", "Thirdperson distance", 25, 200, 100);
	config.visuals.effects.aspect_ratio = AddSliderFloat("Visuals", "Effects", "Aspect ratio", 0, 2, 0, "%.2f");
	config.visuals.effects.client_impacts = AddCheckBox("Visuals", "Effects", "Client impacts");
	config.visuals.effects.client_impacts_color = AddColorPicker("Visuals", "Effects", "Client impacts", Color(255, 0, 0, 125));
	config.visuals.effects.server_impacts = AddCheckBox("Visuals", "Effects", "Server impacts");
	config.visuals.effects.server_impacts_color = AddColorPicker("Visuals", "Effects", "Server impacts", Color(0, 0, 255, 125));
	config.visuals.effects.impacts_duration = AddSliderInt("Visuals", "Effects", "Duration", 1, 10, 4, "%ds");
	config.visuals.effects.override_skybox = AddComboBox("Visuals", "Effects", "Override skybox", { "Disabled", "Night 1", "Night 2", "Night 3" });
	config.visuals.effects.override_fog = AddCheckBox("Visuals", "Effects", "Override fog");
	config.visuals.effects.fog_color = AddColorPicker("Visuals", "Effects", "Override fog");
	config.visuals.effects.fog_start = AddSliderInt("Visuals", "Effects", "Fog start", 0, 1000, 200);
	config.visuals.effects.fog_end = AddSliderInt("Visuals", "Effects", "Fog end", 0, 1000, 500);
	config.visuals.effects.fog_density = AddSliderInt("Visuals", "Effects", "Fog density", 0, 100, 50);
	config.visuals.effects.preserve_killfeed = AddCheckBox("Visuals", "Effects", "Preserve killfeed");
	config.visuals.effects.optimizations = AddMultiCombo("Visuals", "Effects", "Additional removals", { "Teammates", "Ragdolls", "Decals" });
	config.visuals.effects.custom_sun_direction = AddCheckBox("Visuals", "Effects", "Custom sun direction");
	config.visuals.effects.sun_pitch = AddSliderInt("Visuals", "Effects", "Sun pitch", 0, 90, 0);
	config.visuals.effects.sun_yaw = AddSliderInt("Visuals", "Effects", "Sun yaw", -180, 180, 0);
	config.visuals.effects.sun_distance = AddSliderInt("Visuals", "Effects", "Sun distance", 0, 2000, 400);
	config.visuals.effects.viewmodel_scope_alpha = AddSliderInt("Visuals", "Effects", "Viewmodel scope alpha", 0, 100, 0, "%d%%");

	config.misc.miscellaneous.anti_untrusted = AddCheckBox("Misc", "Miscellaneous", "Anti untrusted");
	config.misc.miscellaneous.logs = AddMultiCombo("Misc", "Miscellaneous", "Logs", { "Damage", "Aimbot", "Purchuases" });
	config.misc.miscellaneous.auto_buy = AddMultiCombo("Misc", "Miscellaneous", "Auto buy", { "AWP", "Scout", "Autosniper", "Deagle / R8", "Five-Seven / Tec-9", "Taser", "Armor", "Smoke", "Molotov", "HeGrenade", "Flashbang", "Defuse kit" });
	config.misc.miscellaneous.filter_console = AddCheckBox("Misc", "Miscellaneous", "Filter console");
	config.misc.miscellaneous.clantag = AddCheckBox("Misc", "Miscellaneous", "Clantag");
	config.misc.miscellaneous.ad_block = AddCheckBox("Misc", "Miscellaneous", "Ad block");

	config.misc.movement.auto_jump = AddCheckBox("Misc", "Movement", "Auto jump");
	config.misc.movement.auto_strafe = AddCheckBox("Misc", "Movement", "Auto strafe");
	config.misc.movement.auto_strafe_smooth = AddSliderInt("Misc", "Movement", "Auto strafe smooth", 0, 100, 50, "%d%%");
	config.misc.movement.compensate_throwable = AddCheckBox("Misc", "Movement", "Compensate throwable");
	config.misc.movement.edge_jump = AddCheckBox("Misc", "Movement", "Edge jump");
	config.misc.movement.edge_jump_key = AddKeyBind("Misc", "Movement", "Edge jump");
	config.misc.movement.infinity_duck = AddCheckBox("Misc", "Movement", "Infinity duck");
	config.misc.movement.quick_stop = AddCheckBox("Misc", "Movement", "Quick stop");

	config.skins.override_knife = AddCheckBox("Skins", "Models", "Override knife");
	config.skins.knife_model = AddComboBox("Skins", "Models", "Knife model", SkinChanger->GetUIKnifeModels());

	config.skins.override_agent = AddCheckBox("Skins", "Models", "Override agent");
	config.skins.agent_model_t = AddComboBox("Skins", "Models", "Agent model T Side", {
		"Getaway Sally | The Professionals",
		"Number K | The Professionals",
		"Little Kev | The Professionals",
		"Safecracker Voltzmann | The Professionals",
		"Bloody Darryl The Strapped | The Professionals",
		"Sir Bloody Loudmouth Darryl | The Professionals",
		"Sir Bloody Darryl Royale | The Professionals",
		"Sir Bloody Skullhead Darryl | The Professionals",
		"Sir Bloody Silent Darryl | The Professionals",
		"Sir Bloody Miami Darryl | The Professionals",
		"Street Soldier | Phoenix",
		"Soldier | Phoenix",
		"Slingshot | Phoenix",
		"Enforcer | Phoenix",
		"Mr. Muhlik | Elite Crew",
		"Prof. Shahmat | Elite Crew",
		"Osiris | Elite Crew",
		"Ground Rebel | Elite Crew",
		"The Elite Mr. Muhlik | Elite Crew",
		"Trapper | Guerrilla Warfare",
		"Trapper Aggressor | Guerrilla Warfare",
		"Vypa Sista of the Revolution | Guerrilla Warfare",
		"Col. Mangos Dabisi | Guerrilla Warfare",
		"Arno The Overgrown | Guerrilla Warfare",
		"Medium Rare' Crasswater | Guerrilla Warfare",
		"Crasswater The Forgotten | Guerrilla Warfare",
		"Elite Trapper Solman | Guerrilla Warfare",
		"The Doctor' Romanov | Sabre",
		"Blackwolf | Sabre",
		"Maximus | Sabre",
		"Dragomir | Sabre",
		"Rezan The Ready | Sabre",
		"Rezan the Redshirt | Sabre",
		"Dragomir | Sabre Footsoldier",
		"Danger zone agent A",
		"Danger zone agent B",
	});

	config.skins.agent_model_ct = AddComboBox("Skins", "Models", "Agent model CT Side", {
		"Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman",
		"Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman",
		"Lieutenant Rex Krikey | SEAL Frogman",
		"Michael Syfers | FBI Sniper",
		"Operator | FBI SWAT",
		"Special Agent Ava | FBI",
		"Markus Delrow | FBI HRT",
		"Sous-Lieutenant Medic | Gendarmerie Nationale",
		"Chem-Haz Capitaine | Gendarmerie Nationale",
		"Chef d'Escadron Rouchard | Gendarmerie Nationale",
		"Aspirant | Gendarmerie Nationale",
		"Officer Jacques Beltram | Gendarmerie Nationale",
		"D Squadron Officer | NZSAS",
		"B Squadron Officer | SAS",
		"Seal Team 6 Soldier | NSWC SEAL",
		"Buckshot | NSWC SEAL",
		"Lt. Commander Ricksaw | NSWC SEAL",
		"Blueberries' Buckshot | NSWC SEAL",
		"3rd Commando Company | KSK",
		"Two Times' McCoy | TACP Cavalry",
		"Two Times' McCoy | USAF TACP",
		"Primeiro Tenente | Brazilian 1st Battalion",
		"Cmdr. Mae 'Dead Cold' Jamison | SWAT",
		"1st Lieutenant Farlow | SWAT",
		"John 'Van Healen' Kask | SWAT",
		"Bio-Haz Specialist | SWAT",
		"Sergeant Bombson | SWAT",
		"Chem-Haz Specialist | SWAT",
		"Lieutenant 'Tree Hugger' Farlow | SWAT",
		"GIGN A"
	});

	Config->config_list = AddComboBox("Config", "Config", "cfglist", {});
	Config->config_name = AddInput("Config", "Config", "Config");
	Config->load_button = AddButton("Config", "Config", "Load");
	Config->save_button = AddButton("Config", "Config", "Save");

	Config->lua_list = AddComboBox("Scripts", "Scripts", "lualist", {});
	Config->lua_button = AddButton("Scripts", "Scripts", "Load");
	Config->lua_button_unload = AddButton("Scripts", "Scripts", "Unload");
	Config->lua_refresh = AddButton("Scripts", "Scripts", "Refresh");
	Config->lua_save = AddButton("Scripts", "Scripts", "Save");

	Config->Init();

	config.ragebot.selected_weapon->SetCallback([]() {
		const int selected_weapon = config.ragebot.selected_weapon->get();
		auto& weapon_configs = config.ragebot.weapons;

		weapon_configs.global.SetVisible(selected_weapon == 0);
		weapon_configs.awp.SetVisible(selected_weapon == 1);
		weapon_configs.autosniper.SetVisible(selected_weapon == 2);
		weapon_configs.scout.SetVisible(selected_weapon == 3);
		weapon_configs.deagle.SetVisible(selected_weapon == 4);
		weapon_configs.pistol.SetVisible(selected_weapon == 5);
	});

	auto world_modulation_callback = []() {
		World->Modulation();
	};

	config.visuals.effects.world_color_enable->SetCallback(world_modulation_callback);
	config.visuals.effects.world_color->SetCallback(world_modulation_callback);
	config.visuals.effects.props_color_enable->SetCallback(world_modulation_callback);
	config.visuals.effects.props_color->SetCallback(world_modulation_callback);

	config.visuals.effects.override_skybox->SetCallback([]() {
		World->SkyBox();
	});

	auto world_fog_callback = []() {
		World->Fog();

		config.visuals.effects.fog_density->SetVisible(config.visuals.effects.override_fog->get());
		config.visuals.effects.fog_start->SetVisible(config.visuals.effects.override_fog->get());
		config.visuals.effects.fog_end->SetVisible(config.visuals.effects.override_fog->get());
	};

	config.visuals.effects.override_fog->SetCallback(world_fog_callback);
	config.visuals.effects.fog_color->SetCallback(world_fog_callback);
	config.visuals.effects.fog_density->SetCallback(world_fog_callback);
	config.visuals.effects.fog_start->SetCallback(world_fog_callback);
	config.visuals.effects.fog_end->SetCallback(world_fog_callback);

	config.visuals.effects.removals->SetCallback([]() {
		World->RemoveBlood();
		World->Smoke();
	});

	config.visuals.effects.custom_sun_direction->SetCallback([]() {
		config.visuals.effects.sun_pitch->SetVisible(config.visuals.effects.custom_sun_direction->get());
		config.visuals.effects.sun_yaw->SetVisible(config.visuals.effects.custom_sun_direction->get());
		config.visuals.effects.sun_distance->SetVisible(config.visuals.effects.custom_sun_direction->get());

		World->SunDirection();
	});

	auto sun_update = []() {
		World->SunDirection();
	};

	config.visuals.effects.sun_pitch->SetCallback(sun_update);
	config.visuals.effects.sun_yaw->SetCallback(sun_update);
	config.visuals.effects.sun_distance->SetCallback(sun_update);
}