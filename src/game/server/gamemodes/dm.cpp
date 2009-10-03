/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#include "dm.hpp"
#include <engine/e_config.h>

GAMECONTROLLER_DM::GAMECONTROLLER_DM()
{
    if(config.sv_gametype_mod)
	gametype = "mDM";
    else
	gametype = "DM";
}

void GAMECONTROLLER_DM::tick()
{
	do_player_score_wincheck();
	GAMECONTROLLER::tick();
}
