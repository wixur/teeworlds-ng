#include "../game/game_variables.h"

MACRO_CONFIG_INT(volume, 200, 0, 255)
MACRO_CONFIG_INT(cpu_throttle, 0, 0, 1)
MACRO_CONFIG_STR(player_name, 32, "nameless tee")
MACRO_CONFIG_STR(clan_name, 32, "")
MACRO_CONFIG_STR(password, 32, "")


MACRO_CONFIG_INT(debug, 0, 0, 1)
MACRO_CONFIG_INT(stress, 0, 0, 0)
MACRO_CONFIG_STR(cl_stress_server, 32, "localhost")

MACRO_CONFIG_INT(gfx_screen_width, 800, 0, 0)
MACRO_CONFIG_INT(gfx_screen_height, 600, 0, 0)
MACRO_CONFIG_INT(gfx_fullscreen, 1, 0, 1)
MACRO_CONFIG_INT(gfx_color_depth, 24, 16, 24)
MACRO_CONFIG_INT(gfx_vsync, 1, 0, 1)
MACRO_CONFIG_INT(gfx_display_all_modes, 0, 0, 1)
MACRO_CONFIG_INT(gfx_texture_compression, 0, 0, 1)
MACRO_CONFIG_INT(gfx_high_detail, 1, 0, 1)
MACRO_CONFIG_INT(gfx_texture_quality, 1, 0, 1)

MACRO_CONFIG_INT(key_screenshot, 'Q', 32, 512)

MACRO_CONFIG_STR(masterserver, 128, "master.teewars.com")

MACRO_CONFIG_STR(sv_name, 128, "unnamed server")
MACRO_CONFIG_STR(sv_bindaddr, 128, "")
MACRO_CONFIG_INT(sv_port, 8303, 0, 0)
MACRO_CONFIG_INT(sv_sendheartbeats, 1, 0, 1)
MACRO_CONFIG_STR(sv_map, 128, "dm1")

MACRO_CONFIG_INT(sv_max_clients, 8, 1, 8)

