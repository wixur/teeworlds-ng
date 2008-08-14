#include <new>

#include <engine/e_server_interface.h>

#include "player.hpp"
#include "gamecontext.hpp"

PLAYER::PLAYER()
{
}

void PLAYER::init(int client_id)
{
	// clear everything
	mem_zero(this, sizeof(*this));
	new(this) PLAYER();
	this->client_id = client_id;
}

void PLAYER::tick()
{
	server_setclientscore(client_id, score);

	// do latency stuff
	{
		CLIENT_INFO info;
		if(server_getclientinfo(client_id, &info))
		{
			latency.accum += info.latency;
			latency.accum_max = max(latency.accum_max, info.latency);
			latency.accum_min = min(latency.accum_min, info.latency);
		}

		if(server_tick()%server_tickspeed() == 0)
		{
			latency.avg = latency.accum/server_tickspeed();
			latency.max = latency.accum_max;
			latency.min = latency.accum_min;
			latency.accum = 0;
			latency.accum_min = 1000;
			latency.accum_max = 0;
		}
	}
	
	if(spawning && !get_character())
		try_respawn();
		
	if(get_character())
		view_pos = get_character()->pos;
}

void PLAYER::snap(int snaping_client)
{
	NETOBJ_PLAYER_INFO *info = (NETOBJ_PLAYER_INFO *)snap_new_item(NETOBJTYPE_PLAYER_INFO, client_id, sizeof(NETOBJ_PLAYER_INFO));

	info->latency = latency.min;
	info->latency_flux = latency.max-latency.min;
	info->local = 0;
	info->cid = client_id;
	info->score = score;
	info->team = team;

	if(client_id == snaping_client)
		info->local = 1;	
}

void PLAYER::on_disconnect()
{
	kill_character();
	
	//game.controller->on_player_death(&game.players[client_id], 0, -1);
		
	char buf[512];
	str_format(buf, sizeof(buf),  "%s has left the game", server_clientname(client_id));
	game.send_chat(-1, GAMECONTEXT::CHAT_ALL, buf);

	dbg_msg("game", "leave player='%d:%s'", client_id, server_clientname(client_id));

	// clear this whole structure
	init(-1);

	/*game.world.remove_entity(&game.players[client_id]);
	game.world.core.players[client_id] = 0x0;
	game.players[client_id].client_id = -1;	*/
}

void PLAYER::on_predicted_input(NETOBJ_PLAYER_INPUT *new_input)
{
	CHARACTER *chr = get_character();
	if(chr)
		chr->on_predicted_input(new_input);
}

void PLAYER::on_direct_input(NETOBJ_PLAYER_INPUT *new_input)
{
	CHARACTER *chr = get_character();
	if(chr)
		chr->on_direct_input(new_input);

	if(!chr && team >= 0 && (new_input->fire&1))
	{
		spawning = true;
		dbg_msg("", "I wanna spawn");
	}
}

CHARACTER *PLAYER::get_character()
{
	if(character.alive)
		return &character;
	return 0;
}

void PLAYER::kill_character()
{
	CHARACTER *chr = get_character();
	if(chr)
		chr->die(-1, -1);
}

void PLAYER::respawn()
{
	spawning = true;
}

void PLAYER::set_team(int new_team)
{
	// clamp the team
	new_team = game.controller->clampteam(new_team);
	if(team == new_team)
		return;
		
	char buf[512];
	str_format(buf, sizeof(buf), "%s joined the %s", server_clientname(client_id), game.controller->get_team_name(new_team));
	game.send_chat(-1, GAMECONTEXT::CHAT_ALL, buf); 
	
	kill_character();
	team = new_team;
	score = 0;
	dbg_msg("game", "team_join player='%d:%s' team=%d", client_id, server_clientname(client_id), team);
	
	game.controller->on_player_info_change(&game.players[client_id]);

	// send all info to this client
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(game.players[i].client_id != -1)
			game.send_info(i, -1);
	}
}

vec2 spawn_points[3][64];
int num_spawn_points[3] = {0};

struct SPAWNEVAL
{
	SPAWNEVAL()
	{
		got = false;
		friendly_team = -1;
//		die_pos = vec2(0,0);
		pos = vec2(100,100);
	}
		
	vec2 pos;
	bool got;
	int friendly_team;
	float score;
//	vec2 die_pos;
};

static float evaluate_spawn(SPAWNEVAL *eval, vec2 pos)
{
	float score = 0.0f;
	CHARACTER *c = (CHARACTER *)game.world.find_first(NETOBJTYPE_CHARACTER);
	for(; c; c = (CHARACTER *)c->typenext())
	{
		// team mates are not as dangerous as enemies
		float scoremod = 1.0f;
		if(eval->friendly_team != -1 && c->team == eval->friendly_team)
			scoremod = 0.5f;
			
		float d = distance(pos, c->pos);
		if(d == 0)
			score += 1000000000.0f;
		else
			score += 1.0f/d;
	}
	
	// weight in the die posititon
	/*
	float d = distance(pos, eval->die_pos);
	if(d == 0)
		score += 1000000000.0f;
	else
		score += 1.0f/d;*/
	
	return score;
}

static void evaluate_spawn_type(SPAWNEVAL *eval, int t)
{
	// get spawn point
	/*
	int start, num;
	map_get_type(t, &start, &num);
	if(!num)
		return;
	*/
	for(int i  = 0; i < num_spawn_points[t]; i++)
	{
		//num_spawn_points[t]
		//mapres_spawnpoint *sp = (mapres_spawnpoint*)map_get_item(start + i, NULL, NULL);
		vec2 p = spawn_points[t][i];// vec2((float)sp->x, (float)sp->y);
		float s = evaluate_spawn(eval, p);
		if(!eval->got || eval->score > s)
		{
			eval->got = true;
			eval->score = s;
			eval->pos = p;
		}
	}
}

void PLAYER::try_respawn()
{
	vec2 spawnpos = vec2(100.0f, -60.0f);
	
	// get spawn point
	SPAWNEVAL eval;
	//eval.die_pos = die_pos;
	
	eval.pos = vec2(100, 100);
	
	if(game.controller->gametype == GAMETYPE_CTF)
	{
		eval.friendly_team = team;
		
		// try first try own team spawn, then normal spawn and then enemy
		evaluate_spawn_type(&eval, 1+(team&1));
		if(!eval.got)
		{
			evaluate_spawn_type(&eval, 0);
			if(!eval.got)
				evaluate_spawn_type(&eval, 1+((team+1)&1));
		}
	}
	else
	{
		if(game.controller->gametype == GAMETYPE_TDM)
			eval.friendly_team = team;
			
		evaluate_spawn_type(&eval, 0);
		evaluate_spawn_type(&eval, 1);
		evaluate_spawn_type(&eval, 2);
	}
	
	spawnpos = eval.pos;

	// check if the position is occupado
	ENTITY *ents[2] = {0};
	int num_ents = game.world.find_entities(spawnpos, 64, ents, 2, NETOBJTYPE_CHARACTER);
	
	if(num_ents == 0)
	{
		spawning = false;
		character.spawn(this, spawnpos, team);
	}
	
	/*
	pos = spawnpos;

	core.pos = pos;
	core.vel = vec2(0,0);
	core.hooked_player = -1;

	health = 10;
	armor = 0;
	jumped = 0;
	
	mem_zero(&ninja, sizeof(ninja));
	
	dead = false;
	player_state = PLAYERSTATE_PLAYING;
	
	game.world.insert_entity(this);

	core.hook_state = HOOK_IDLE;

	mem_zero(&input, sizeof(input));

	// init weapons
	mem_zero(&weapons, sizeof(weapons));
	weapons[WEAPON_HAMMER].got = true;
	weapons[WEAPON_HAMMER].ammo = -1;
	weapons[WEAPON_GUN].got = true;
	weapons[WEAPON_GUN].ammo = 10;

	active_weapon = WEAPON_GUN;
	last_weapon = WEAPON_HAMMER;
	queued_weapon = 0;

	reload_timer = 0;

	// Create sound and spawn effects
	game.create_sound(pos, SOUND_PLAYER_SPAWN);
	game.create_playerspawn(pos);

	game.controller->on_player_spawn(player);
	*/
}
