#include "UI.h"
#include "../SDK/Config.h"

#include "../Features/Visuals/SkinChanger.h"
#include "../Features/Visuals/World.h"

void CMenu::SetupUI() {
	AddTab("Aimbot", pic::tab::aimbot, ImVec2(14, 14));
	AddTab("Anti aim", pic::tab::antiaim, ImVec2(14.74f, 14));
	AddTab("Player", pic::tab::players, ImVec2(8.75f, 14));
	AddTab("Visuals", pic::tab::visuals, ImVec2(14, 9.33f));
	AddTab("Misc", pic::tab::misc, ImVec2(14, 11.74f));
	AddTab("Skins", pic::tab::skins, ImVec2(16, 16));
	AddTab("Config", pic::tab::configs, ImVec2(13.4f, 14));
	AddTab("Scripts", pic::tab::scripts, ImVec2(15, 12));

	auto aimbot = AddGroupBox("Aimbot", "Aimbot");
	auto aim_settings = AddGroupBox("Aimbot", "Settings");
	
	auto aa_angles = AddGroupBox("Anti aim", "Angles");
	auto fake_lag = AddGroupBox("Anti aim", "Fake lag", 1.f, 1);
	auto aa_other = AddGroupBox("Anti aim", "Other", 1.f, 1);

	auto player_esp = AddGroupBox("Player", "ESP");
	auto chams = AddGroupBox("Player", "Chams");

	auto other_esp = AddGroupBox("Visuals", "Other ESP");
	auto effects = AddGroupBox("Visuals", "Effects");

	auto misc = AddGroupBox("Misc", "Miscellaneous");
	auto movement = AddGroupBox("Misc", "Movement");

	auto skins = AddGroupBox("Skins", "Skins"); 
	auto models = AddGroupBox("Skins", "Models");

	auto configs = AddGroupBox("Config", "Config");

	auto scripts = AddGroupBox("Scripts", "Scripts");

	config.ragebot.aimbot.enabled = aimbot->AddCheckBox("Enabled");
	config.ragebot.aimbot.extrapolation = aimbot->AddComboBox("Extrapolation", { "Disable", "Enable", "Force" });
	config.ragebot.aimbot.doubletap = aimbot->AddCheckBox("Double Tap");
	config.ragebot.aimbot.doubletap_key = aimbot->AddKeyBind("Double Tap");
	config.ragebot.aimbot.doubletap_options = aimbot->AddMultiCombo("Double Tap options", { "Break LC", "Lag Peek", "Immediate teleport"});
	config.ragebot.aimbot.hide_shots = aimbot->AddCheckBox("Hide Shots");
	config.ragebot.aimbot.hide_shots_key = aimbot->AddKeyBind("Hide Shots");
	config.ragebot.aimbot.force_teleport = aimbot->AddKeyBind("Force Teleport");
	config.ragebot.aimbot.force_body_aim = aimbot->AddKeyBind("Force Body Aim");
	config.ragebot.aimbot.minimum_damage_override_key = aimbot->AddKeyBind("Min. damage override");
	config.ragebot.aimbot.peek_assist = aimbot->AddCheckBox("Peek Assist");
	config.ragebot.aimbot.peek_assist_color = aimbot->AddColorPicker("Peek Assist");
	config.ragebot.aimbot.peek_assist_keybind = aimbot->AddKeyBind("Peek Assist key");
	config.ragebot.aimbot.show_aimpoints = aimbot->AddCheckBox("Show aim points");
	config.ragebot.aimbot.show_debug_data = aimbot->AddCheckBox("Show debug data");
	config.ragebot.aimbot.threads = aimbot->AddCheckBox("Multithread");
	config.ragebot.aimbot.roll_resolver = aimbot->AddCheckBox("Roll Resolver");
	config.ragebot.aimbot.roll_resolver_key = aimbot->AddKeyBind("Roll Resolver");
	config.ragebot.aimbot.roll_angle = aimbot->AddSliderInt("Roll Angle", -90, 90, 0);

	config.ragebot.selected_weapon = aim_settings->AddComboBox("Current Weapon", { "Global", "AWP", "Autosniper", "Scout", "Deagle", "Revolver", "Pistol" });

	auto setup_weapon_config = [this, aim_settings](weapon_settings_t& settings) {
		settings.hitboxes = aim_settings->AddMultiCombo(std::format("[{}] {}", settings.weapon_name, "Hitbox"), { "Head", "Chest", "Stomach", "Arms", "Legs", "Feet" });
		settings.multipoints = aim_settings->AddMultiCombo(std::format("[{}] {}", settings.weapon_name, "Multipoints"), { "Head", "Chest", "Stomach" });
		settings.head_point_scale = aim_settings->AddSliderInt(std::format("[{}] {}", settings.weapon_name, "Head scale"), 0, 100, 50, "%d%%");
		settings.body_point_scale = aim_settings->AddSliderInt(std::format("[{}] {}", settings.weapon_name, "Body scale"), 0, 100, 50, "%d%%");
		settings.hitchance = aim_settings->AddSliderInt(std::format("[{}] {}", settings.weapon_name, "Hitchance"), 0, 100, 50, "%d%%");
		settings.minimum_damage = aim_settings->AddSliderInt(std::format("[{}] {}", settings.weapon_name, "Minimum damage"), 1, 130, 30);
		settings.minimum_damage_override = aim_settings->AddSliderInt(std::format("[{}] {}", settings.weapon_name, "Minimum damage override"), 1, 130, 10);
		settings.auto_stop = aim_settings->AddMultiCombo(std::format("[{}] {}", settings.weapon_name, "Auto stop"), { "Full stop", "Early", "Move between shots", "In Air" });
		settings.auto_scope = aim_settings->AddCheckBox(std::format("[{}] {}", settings.weapon_name, "Auto scope"));
	};

	setup_weapon_config(config.ragebot.weapons.global);
	setup_weapon_config(config.ragebot.weapons.awp);
	setup_weapon_config(config.ragebot.weapons.autosniper);
	setup_weapon_config(config.ragebot.weapons.scout);
	setup_weapon_config(config.ragebot.weapons.deagle);
	setup_weapon_config(config.ragebot.weapons.revolver);
	setup_weapon_config(config.ragebot.weapons.pistol);

	config.antiaim.angles.pitch = aa_angles->AddComboBox("Pitch", { "Disabled", "Down" });
	config.antiaim.angles.yaw = aa_angles->AddComboBox("Yaw", { "Forward", "Backward", "At target" });
	config.antiaim.angles.yaw_jitter = aa_angles->AddCheckBox("Yaw jitter");
	config.antiaim.angles.modifier_value = aa_angles->AddSliderInt("Modifier value", -180, 180, 0);
	config.antiaim.angles.manual_left = aa_angles->AddKeyBind("Manual left");
	config.antiaim.angles.manual_right = aa_angles->AddKeyBind("Manual right");
	config.antiaim.angles.manual_options = aa_angles->AddMultiCombo("Manual options", {"Disable jitters", "Freestand"});
	config.antiaim.angles.body_yaw = aa_angles->AddCheckBox("Body yaw");
	config.antiaim.angles.body_yaw_options = aa_angles->AddMultiCombo("Body yaw options", { "Jitter", "Random jitter", "Extended", "Freestand" });
	config.antiaim.angles.body_yaw_limit = aa_angles->AddSliderInt("Limit", 0, 58, 58);
	config.antiaim.angles.inverter = aa_angles->AddKeyBind("Inverter");
	config.antiaim.angles.legacy_desync = aa_angles->AddCheckBox("Legacy Desync");

	config.antiaim.fakelag.enabled = fake_lag->AddCheckBox("Enabled");
	config.antiaim.fakelag.limit = fake_lag->AddSliderInt("Limit", 1, 15, 13);
	config.antiaim.fakelag.variability = fake_lag->AddSliderInt("Variabaility", 1, 14, 1);
	config.antiaim.fakelag.triggers = fake_lag->AddMultiCombo("Triggers", { "Move", "Air", "Break LC", "Peek" });

	config.antiaim.misc.fake_duck = aa_other->AddKeyBind("Fake duck");
	config.antiaim.misc.slow_walk = aa_other->AddKeyBind("Slow walk");
	config.antiaim.misc.slow_walk_type = aa_other->AddComboBox("Type", { "Force accuracy", "Custom" });
	config.antiaim.misc.custom_slow_walk = aa_other->AddSliderInt("Custom speed", 0, 100, 50, "%d%%");
	config.antiaim.misc.animations = aa_other->AddMultiCombo("Animations", { "Lean", "Static legs in air", "Backward legs" });
	config.antiaim.misc.leg_movement = aa_other->AddComboBox("Leg movement", { "Default", "Sliding", "Walking" });

	config.visuals.esp.enable = player_esp->AddCheckBox("Enable");
	config.visuals.esp.dormant = player_esp->AddCheckBox("Dormant");
	config.visuals.esp.dormant_color = player_esp->AddColorPicker("Dormant", Color(255, 255, 255, 200));
	config.visuals.esp.bounding_box = player_esp->AddCheckBox("Bounding box");
	config.visuals.esp.box_color = player_esp->AddColorPicker("Bounding box");
	config.visuals.esp.health_bar = player_esp->AddCheckBox("Health bar");
	config.visuals.esp.custom_health = player_esp->AddCheckBox("Custom health");
	config.visuals.esp.custom_health_color = player_esp->AddColorPicker("Custom health");
	config.visuals.esp.name = player_esp->AddCheckBox("Name");
	config.visuals.esp.name_color = player_esp->AddColorPicker("Name");
	config.visuals.esp.flags = player_esp->AddMultiCombo("Flags", { "Armor", "Zoom", "Fake duck", "Exploit", "Break LC", "Bomb", "Resolver" });
	config.visuals.esp.weapon_text = player_esp->AddCheckBox("Weapon text");
	config.visuals.esp.weapon_text_color = player_esp->AddColorPicker("Weapon text");
	config.visuals.esp.weapon_icon = player_esp->AddCheckBox("Weapon icon");
	config.visuals.esp.weapon_icon_color = player_esp->AddColorPicker("Weapon icon");
	config.visuals.esp.ammo = player_esp->AddCheckBox("Ammo");
	config.visuals.esp.ammo_color = player_esp->AddColorPicker("Ammo", Color(80, 140, 200));
	config.visuals.esp.glow = player_esp->AddCheckBox("Glow");
	config.visuals.esp.glow_color = player_esp->AddColorPicker("Glow", Color(180, 60, 120));
	config.visuals.esp.hitsound = player_esp->AddCheckBox("Hit sound");
	config.visuals.esp.show_server_hitboxes = player_esp->AddCheckBox("Show sever hitboxes");
	config.visuals.esp.shared_esp = player_esp->AddCheckBox("Shared ESP");
	config.visuals.esp.share_with_enemies = player_esp->AddCheckBox("Share with enemies");
	config.visuals.esp.hitmarker = player_esp->AddCheckBox("Hitmarker");
	config.visuals.esp.hitmarker_color = player_esp->AddColorPicker("Hitmarker");
	config.visuals.esp.damage_marker = player_esp->AddCheckBox("Damage marker");
	config.visuals.esp.damage_marker_color = player_esp->AddColorPicker("Damage marker");

	config.visuals.chams.enemy = chams->AddCheckBox("Player");
	config.visuals.chams.enemy_invisible = chams->AddCheckBox("Player behind wall");
	config.visuals.chams.enemy_color = chams->AddColorPicker("Player", Color(150, 190, 70));
	config.visuals.chams.enemy_invisible_color = chams->AddColorPicker("Player behind wall", Color(60, 120, 160));
	config.visuals.chams.enemy_second_color = chams->AddColorPicker("Enemy second color");
	config.visuals.chams.enemy_type = chams->AddComboBox("Enemy type", { "Default", "Solid", "Glow", "Glow overlay" });
	config.visuals.chams.enemy_glow_thickness = chams->AddSliderFloat("Enemy glow thickness", 0.1f, 9.f, 1.f, "%.1f");
	config.visuals.chams.shot_chams = chams->AddCheckBox("Shot");
	config.visuals.chams.shot_chams_color = chams->AddColorPicker("Shot", Color(100, 100, 100));
	config.visuals.chams.shot_chams_second_color = chams->AddColorPicker("Shot second color");
	config.visuals.chams.shot_chams_type = chams->AddComboBox("Shot type", { "Default", "Solid", "Glow", "Glow overlay" });
	config.visuals.chams.shot_chams_thickness = chams->AddSliderFloat("Shot glow thickness", 0.1f, 9.f, 1.f, "%.1f");
	config.visuals.chams.shot_chams_duration = chams->AddSliderInt("Shot chams duration", 1, 10, 4, "s");
	config.visuals.chams.shot_chams_options = chams->AddMultiCombo("Shot chams options", { "Behind walls", "Last shot only" });
	config.visuals.chams.local_player = chams->AddCheckBox("Local player");
	config.visuals.chams.local_player_color = chams->AddColorPicker("Local player", Color(100, 100, 100));
	config.visuals.chams.local_player_second_color = chams->AddColorPicker("Local second color");
	config.visuals.chams.local_player_type = chams->AddComboBox("Local type", { "Default", "Solid", "Glow", "Glow outlined", "Glow owerlay" });
	config.visuals.chams.local_glow_thickness = chams->AddSliderFloat("Local glow thickness", 0.1f, 9.f, 1.f, "%.1f");
	config.visuals.chams.attachments = chams->AddCheckBox("Attachments");
	config.visuals.chams.attachments_color = chams->AddColorPicker("Attachments");
	config.visuals.chams.attachments_second_color = chams->AddColorPicker("Attachments second color");
	config.visuals.chams.attachments_type = chams->AddComboBox("Attachments type", { "Default", "Solid", "Glow", "Glow overlay" });
	config.visuals.chams.attachments_glow_thickness = chams->AddSliderFloat("Attachments glow thickness", 0.1f, 9.f, 1.f, "%.1f");
	config.visuals.chams.viewmodel = chams->AddCheckBox("Viewmodel");
	config.visuals.chams.viewmodel_color = chams->AddColorPicker("Viewmodel");
	config.visuals.chams.viewmodel_second_color = chams->AddColorPicker("Viewmodel second color");
	config.visuals.chams.viewmodel_type = chams->AddComboBox("Viewmodel type", { "Default", "Solid", "Glow", "Glow overlay" });
	config.visuals.chams.viewmodel_glow_thickness = chams->AddSliderFloat("Viewmodel glow thickness", 0.1f, 9.f, 1.f, "%.1f");
	config.visuals.chams.disable_model_occlusion = chams->AddCheckBox("Disable model occlusion");

	config.visuals.other_esp.radar = other_esp->AddCheckBox("Radar");
	config.visuals.other_esp.dropped_weapons = other_esp->AddMultiCombo("Dropped weapons", { "Text", "Icon", "Glow" });
	config.visuals.other_esp.dropped_weapons_color = other_esp->AddColorPicker("Dropped weapons");
	config.visuals.other_esp.sniper_crosshair = other_esp->AddCheckBox("Sniper crosshair");
	config.visuals.other_esp.penetration_crosshair = other_esp->AddCheckBox("Penetration crosshair");
	config.visuals.other_esp.bomb = other_esp->AddCheckBox("Bomb");
	config.visuals.other_esp.bomb_color = other_esp->AddColorPicker("Bomb", Color(150, 200, 60));
	config.visuals.other_esp.grenades = other_esp->AddCheckBox("Grenades");
	config.visuals.other_esp.molotov_radius = other_esp->AddCheckBox("Molotov radius");
	config.visuals.other_esp.molotov_radius_color = other_esp->AddColorPicker("Molotov radius", Color(255, 0, 0));
	config.visuals.other_esp.grenade_trajecotry = other_esp->AddCheckBox("Grenade trajectory");
	config.visuals.other_esp.grenade_trajectory_color = other_esp->AddColorPicker("Grenade trajectory", Color(250, 60, 60));
	config.visuals.other_esp.grenade_trajectory_hit_color = other_esp->AddColorPicker("Grenade trajectory (hit)", Color(150, 200, 60));
	config.visuals.other_esp.grenade_proximity_warning = other_esp->AddCheckBox("Grenade proximity warning");
	config.visuals.other_esp.grenade_predict_color = other_esp->AddColorPicker("Grenade predict color");
	config.visuals.other_esp.particles = other_esp->AddMultiCombo("Particles", { "Molotov", "Smoke" });
	config.visuals.other_esp.indicators = other_esp->AddMultiCombo("Indicators", { "Exploits", "Min. damage", "Body aim", "Lag Compensation", "Roll Resolver"});

	config.visuals.effects.fov = effects->AddSliderInt("Field of view", 80, 130, 90);
	config.visuals.effects.removals = effects->AddMultiCombo("Removals", { "Post effects", "Fog", "Shadows", "Smoke", "Flashbang", "Blood", "Sprites" });
	config.visuals.effects.remove_scope = effects->AddComboBox("Remove scope", { "Disabled", "Remove overlay", "Remove all" });
	config.visuals.effects.world_color_enable = effects->AddCheckBox("World color");
	config.visuals.effects.world_color = effects->AddColorPicker("World color");
	config.visuals.effects.props_color_enable = effects->AddCheckBox("Props color");
	config.visuals.effects.props_color = effects->AddColorPicker("Props color");
	config.visuals.effects.thirdperson = effects->AddCheckBox("Force thirdperson");
	config.visuals.effects.thirdperson_bind = effects->AddKeyBind("Force thirdperson");
	config.visuals.effects.thirdperson_distance = effects->AddSliderInt("Thirdperson distance", 25, 200, 100);
	config.visuals.effects.aspect_ratio = effects->AddSliderFloat("Aspect ratio", 0, 2, 0, "%.2f");
	config.visuals.effects.client_impacts = effects->AddCheckBox("Client impacts");
	config.visuals.effects.client_impacts_color = effects->AddColorPicker("Client impacts", Color(255, 0, 0, 125));
	config.visuals.effects.server_impacts = effects->AddCheckBox("Server impacts");
	config.visuals.effects.server_impacts_color = effects->AddColorPicker("Server impacts", Color(0, 0, 255, 125));
	config.visuals.effects.impacts_duration = effects->AddSliderInt("Duration", 1, 10, 4, "%ds");
	config.visuals.effects.override_skybox = effects->AddComboBox("Override skybox", { "Disabled", "Night 1", "Night 2", "Night 3" });
	config.visuals.effects.override_fog = effects->AddCheckBox("Override fog");
	config.visuals.effects.fog_color = effects->AddColorPicker("Override fog");
	config.visuals.effects.fog_start = effects->AddSliderInt("Fog start", 0, 1000, 200);
	config.visuals.effects.fog_end = effects->AddSliderInt("Fog end", 0, 1000, 500);
	config.visuals.effects.fog_density = effects->AddSliderInt("Fog density", 0, 100, 50);
	config.visuals.effects.preserve_killfeed = effects->AddCheckBox("Preserve killfeed");
	config.visuals.effects.custom_sun_direction = effects->AddCheckBox("Custom sun direction");
	config.visuals.effects.sun_pitch = effects->AddSliderInt("Sun pitch", 0, 90, 0);
	config.visuals.effects.sun_yaw = effects->AddSliderInt("Sun yaw", -180, 180, 0);
	config.visuals.effects.sun_distance = effects->AddSliderInt("Sun distance", 0, 2000, 400);
	config.visuals.effects.scope_blend = effects->AddSliderInt("Scope blend", 0, 100, 30, "%d%%");
	config.visuals.effects.viewmodel_scope_alpha = effects->AddSliderInt("Viewmodel scope alpha", 0, 100, 0, "%d%%");

	config.misc.miscellaneous.anti_untrusted = misc->AddCheckBox("Anti untrusted");
	config.misc.miscellaneous.logs = misc->AddMultiCombo("Logs", { "Damage", "Aimbot", "Purchuases" });
	config.misc.miscellaneous.auto_buy = misc->AddMultiCombo("Auto buy", { "AWP", "Scout", "Autosniper", "Deagle / R8", "Five-Seven / Tec-9", "Taser", "Armor", "Smoke", "Molotov", "HeGrenade", "Flashbang", "Defuse kit" });
	config.misc.miscellaneous.filter_console = misc->AddCheckBox("Filter console");
	config.misc.miscellaneous.clantag = misc->AddCheckBox("Clantag");
	config.misc.miscellaneous.ad_block = misc->AddCheckBox("Ad block");

	config.misc.movement.auto_jump = movement->AddCheckBox("Auto jump");
	config.misc.movement.auto_strafe = movement->AddCheckBox("Auto strafe");
	config.misc.movement.auto_strafe_smooth = movement->AddSliderInt("Auto strafe smooth", 0, 100, 50, "%d%%");
	config.misc.movement.compensate_throwable = movement->AddCheckBox("Compensate throwable");
	config.misc.movement.edge_jump = movement->AddCheckBox("Edge jump");
	config.misc.movement.edge_jump_key = movement->AddKeyBind("Edge jump");
	config.misc.movement.infinity_duck = movement->AddCheckBox("Infinity duck");
	config.misc.movement.quick_stop = movement->AddCheckBox("Quick stop");


	config.skins.paint_kits = skins->AddComboBox("Paint kits", SkinChanger->GetUIPaintKits());
	config.skins.glove_paint_kits = skins->AddComboBox("Glove Paint kits", SkinChanger->GetUIPaintKitsGloves());

	config.skins.override_knife = models->AddCheckBox("Override knife");
	config.skins.knife_model = models->AddComboBox("Knife model", SkinChanger->GetUIKnifeModels());

	config.skins.mask_changer = models->AddCheckBox("Enable mask changer");
	config.skins.mask_changer_models = models->AddComboBox("Mask models", {
		"None",
		"Dallas",
		"Battle Mask",
		"Evil Clown",
		"Anaglyph",
		"Boar",
		"Bunny",
		"Bunny Gold",
		"Chains",
		"Chicken",
		"Devil Plastic",
		"Hoxton",
		"Pumpkin",
		"Samurai",
		"Sheep Bloody",
		"Sheep Gold",
		"Sheep Model",
		"Skull",
		"Template",
		"Wolf",
		"Doll",
	});

	config.skins.override_agent = models->AddCheckBox("Override agent");
	config.skins.agent_model_t = models->AddComboBox("Agent model T Side", {
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

	config.skins.agent_model_ct = models->AddComboBox("Agent model CT Side", {
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

	Config->config_list = configs->AddComboBox("cfglist", {});
	Config->config_name = configs->AddInput("Config");
	Config->load_button = configs->AddButton("Load");
	Config->save_button = configs->AddButton("Save");

	Config->lua_list = scripts->AddComboBox("lualist", {});
	Config->lua_button = scripts->AddButton("Load");
	Config->lua_button_unload = scripts->AddButton("Unload");
	Config->lua_refresh = scripts->AddButton("Refresh");
	Config->lua_save = scripts->AddButton("Save");

	Config->Init();
	
	config.ragebot.selected_weapon->SetCallback([]() {
		const int selected_weapon = config.ragebot.selected_weapon->get();
		auto& weapon_configs = config.ragebot.weapons;

		weapon_configs.global.SetVisible(selected_weapon == 0);
		weapon_configs.awp.SetVisible(selected_weapon == 1);
		weapon_configs.autosniper.SetVisible(selected_weapon == 2);
		weapon_configs.scout.SetVisible(selected_weapon == 3);
		weapon_configs.deagle.SetVisible(selected_weapon == 4);
		weapon_configs.revolver.SetVisible(selected_weapon == 5);
		weapon_configs.pistol.SetVisible(selected_weapon == 6);
	});

	auto world_modulation_callback = []() {
		ctx.update_nightmode = true;
	};

	config.visuals.effects.world_color_enable->SetCallback(world_modulation_callback);
	config.visuals.effects.world_color->SetCallback(world_modulation_callback);
	config.visuals.effects.props_color_enable->SetCallback(world_modulation_callback);
	config.visuals.effects.props_color->SetCallback(world_modulation_callback);

	config.visuals.effects.override_skybox->SetCallback([]() {
		World->SkyBox();
	});

	config.skins.knife_model->SetCallback([]() {
		Utils::ForceFullUpdate();
	});

	config.skins.paint_kits->SetCallback([](){
		Utils::ForceFullUpdate();
		SkinChanger->UpdateSkins();
	});

	config.skins.glove_paint_kits->SetCallback([]() {
		Utils::ForceFullUpdate();
		SkinChanger->UpdateSkins();
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
		World->Smoke();
		ctx.update_remove_blood = true;
	});

	config.visuals.effects.custom_sun_direction->SetCallback([]() {
		config.visuals.effects.sun_pitch->SetVisible(config.visuals.effects.custom_sun_direction->get());
		config.visuals.effects.sun_yaw->SetVisible(config.visuals.effects.custom_sun_direction->get());
		config.visuals.effects.sun_distance->SetVisible(config.visuals.effects.custom_sun_direction->get());

		World->SunDirection();
	});

	config.visuals.esp.shared_esp->SetCallback([]() {
		config.visuals.esp.share_with_enemies->SetVisible(config.visuals.esp.shared_esp->get());
	});
}