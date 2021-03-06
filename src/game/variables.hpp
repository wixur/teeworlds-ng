/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */


/* client */
MACRO_CONFIG_INT(cl_predict, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Predict client movements")
MACRO_CONFIG_INT(cl_nameplates, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show nameplates")
MACRO_CONFIG_INT(cl_nameplates_always, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Always show nameplats disregarding of distance")
MACRO_CONFIG_INT(cl_autoswitch_weapons, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Auto switch weapon on pickup")

MACRO_CONFIG_INT(cl_showfps, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show ingame FPS counter")

MACRO_CONFIG_INT(cl_airjumpindicator, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(cl_threadsoundloading, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(cl_warning_teambalance, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Warn about team balance")

MACRO_CONFIG_INT(cl_mouse_deadzone, 300, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(cl_mouse_followfactor, 60, 0, 200, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(cl_mouse_max_distance, 800, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(ed_showkeys, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(cl_flow, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(cl_show_welcome, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(cl_motd_time, 10, 0, 100, CFGFLAG_CLIENT|CFGFLAG_SAVE, "How long to show the server message of the day")

MACRO_CONFIG_STR(cl_version_server, 100, "version.teeworlds.com", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Server to use to check for new versions")

MACRO_CONFIG_STR(cl_languagefile, 255, "", CFGFLAG_CLIENT|CFGFLAG_SAVE, "What language file to use")

MACRO_CONFIG_INT(player_use_custom_color, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(player_color_body, 65408, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player body color")
MACRO_CONFIG_INT(player_color_feet, 65408, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player feet color")
MACRO_CONFIG_STR(player_skin, 64, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player skin")

MACRO_CONFIG_INT(ui_page, 5, 0, 9, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface page")
MACRO_CONFIG_STR(ui_server_address, 128, "localhost:8303", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface server address")
MACRO_CONFIG_INT(ui_scale, 100, 1, 100000, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface scale")

MACRO_CONFIG_INT(ui_color_hue, 160, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color hue")
MACRO_CONFIG_INT(ui_color_sat, 70, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color saturation")
MACRO_CONFIG_INT(ui_color_lht, 175, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color lightness")
MACRO_CONFIG_INT(ui_color_alpha, 228, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface alpha")

MACRO_CONFIG_INT(gfx_noclip, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Disable clipping")
/* Teeworlds-NG client variables */

MACRO_CONFIG_INT(cl_laser_color_red_outline, 75, 0, 1000, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color red outline")
MACRO_CONFIG_INT(cl_laser_color_green_outline, 75, 0, 1000, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color green outline")
MACRO_CONFIG_INT(cl_laser_color_blue_outline, 250, 0, 1000, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color blue outline")
MACRO_CONFIG_INT(cl_laser_color_red_inline, 500, 0, 1000, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color red outline")
MACRO_CONFIG_INT(cl_laser_color_green_inline, 500, 0, 1000, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color green outline")
MACRO_CONFIG_INT(cl_laser_color_blue_inline, 1000, 0, 1000, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color blue outline")
MACRO_CONFIG_INT(cl_nameplates_shadow, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Nameplates shadow")
MACRO_CONFIG_INT(cl_show_ghost, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show latency ghost behind you")
MACRO_CONFIG_INT(gfx_custom_hud_colors, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Custom hud colors")
MACRO_CONFIG_INT(cl_new_scoreboard, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Use new scoreboard")
MACRO_CONFIG_INT(cl_new_scoreboard_id, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show id in new scoreboard")
MACRO_CONFIG_INT(cl_new_scoreboard_full, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Hide stats in new scoreboard")

MACRO_CONFIG_INT(cl_hud_miniscore_transparent, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Transparent miniscore")
MACRO_CONFIG_STR(cl_hud_view, 128, "hnanc", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Hud view")
MACRO_CONFIG_INT(cl_hud_tdtw_timer, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Timer as in tdtw")


MACRO_CONFIG_INT(gfx_red_hud_color_r, 255, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(gfx_red_hud_color_g, 127, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(gfx_red_hud_color_b, 127, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(gfx_blue_hud_color_r, 127, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(gfx_blue_hud_color_g, 159, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(gfx_blue_hud_color_b, 255, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(gfx_player_hud_color_r, 255, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(gfx_player_hud_color_g, 255, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(gfx_player_hud_color_b, 255, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(gfx_spectator_hud_color_r, 192, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(gfx_spectator_hud_color_g, 192, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(gfx_spectator_hud_color_b, 192, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(cl_hud_center, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

/* server */
MACRO_CONFIG_INT(sv_warmup, 0, 0, 0, CFGFLAG_SERVER, "Number of seconds to do warpup before round starts")
MACRO_CONFIG_STR(sv_motd, 900, "", CFGFLAG_SERVER, "Message of the day to display for the clients")
MACRO_CONFIG_INT(sv_teamdamage, 0, 0, 1, CFGFLAG_SERVER, "Team damage")
MACRO_CONFIG_STR(sv_maprotation, 768, "", CFGFLAG_SERVER, "Maps to rotate between")
MACRO_CONFIG_INT(sv_rounds_per_map, 1, 1, 100, CFGFLAG_SERVER, "Number of rounds on each map before rotating")
MACRO_CONFIG_INT(sv_powerups, 1, 0, 1, CFGFLAG_SERVER, "Allow powerups like ninja")
MACRO_CONFIG_INT(sv_scorelimit, 20, 0, 1000, CFGFLAG_SERVER, "Score limit (0 disables)")
MACRO_CONFIG_INT(sv_timelimit, 0, 0, 1000, CFGFLAG_SERVER, "Time limit in minutes (0 disables)")
MACRO_CONFIG_STR(sv_gametype, 32, "dm", CFGFLAG_SERVER, "Game type (dm, tdm, ctf)")
MACRO_CONFIG_INT(sv_tournament_mode, 0, 0, 1, CFGFLAG_SERVER, "Tournament mode. When enabled, players joins the server as spectator")
MACRO_CONFIG_INT(sv_spamprotection, 1, 0, 1, CFGFLAG_SERVER, "Spam protection")

MACRO_CONFIG_INT(sv_spectator_slots, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Number of slots to reserve for spectators")
MACRO_CONFIG_INT(sv_teambalance_time, 1, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before autobalancing teams")

MACRO_CONFIG_INT(sv_vote_kick, 1, 0, 1, CFGFLAG_SERVER, "Allow voting to kick players")
MACRO_CONFIG_INT(sv_vote_kick_bantime, 5, 0, 1440, CFGFLAG_SERVER, "The time to ban a player if kicked by vote. 0 makes it just use kick")
MACRO_CONFIG_INT(sv_vote_scorelimit, 0, 0, 1, CFGFLAG_SERVER, "Allow voting to change score limit")
MACRO_CONFIG_INT(sv_vote_timelimit, 0, 0, 1, CFGFLAG_SERVER, "Allow voting to change time limit")

/* Teeworlds Next generation */
MACRO_CONFIG_INT(sv_gametype_mod, 0, 0, 1, CFGFLAG_SERVER, "Allow use tune, add prefix \"m\" to standart gametypes")
MACRO_CONFIG_STR(sv_items_weapons_start, 10, "HP", CFGFLAG_SERVER, "Spawn weapons")
MACRO_CONFIG_INT(sv_items_weapons_active, 1, 0, 4, CFGFLAG_SERVER, "Start weapon")
MACRO_CONFIG_INT(sv_items_weapons_last, 0, 0, 4, CFGFLAG_SERVER, "Last weapon")
MACRO_CONFIG_INT(sv_items_weapons_queued, -1, 0, 4, CFGFLAG_SERVER, "Que to weapon")

/* debug */
#ifdef CONF_DEBUG /* this one can crash the server if not used correctly */
	MACRO_CONFIG_INT(dbg_dummies, 0, 0, 15, CFGFLAG_SERVER, "")
#endif

MACRO_CONFIG_INT(dbg_focus, 0, 0, 1, CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(dbg_tuning, 0, 0, 1, CFGFLAG_CLIENT, "")
