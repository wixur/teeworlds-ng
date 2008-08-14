/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <base/math.hpp>

#include <engine/e_config.h>
#include <engine/e_server_interface.h>
#include <game/g_version.hpp>
#include <game/g_collision.hpp>
#include <game/g_layers.hpp>


#include <game/g_game.hpp>

#include "gamecontext.hpp"
#include "gamemodes/dm.hpp"
#include "gamemodes/tdm.hpp"
#include "gamemodes/ctf.hpp"

TUNING_PARAMS tuning;


void send_tuning_params(int cid)
{
	/*
	msg_pack_start(NETMSGTYPE_SV_TUNE_PARAMS, MSGFLAG_VITAL);
	int *params = (int *)&tuning;
	for(unsigned i = 0; i < sizeof(tuning_params)/sizeof(int); i++)
		msg_pack_int(params[i]);
	msg_pack_end();
	server_send_msg(cid);
	*/
}

// Server hooks
void mods_client_direct_input(int client_id, void *input)
{
	if(!game.world.paused)
		game.players[client_id].on_direct_input((NETOBJ_PLAYER_INPUT *)input);
	
	/*
	if(i->fire)
	{
		msg_pack_start(MSG_EXTRA_PROJECTILE, 0);
		msg_pack_end();
		server_send_msg(client_id);
	}*/
}

void mods_client_predicted_input(int client_id, void *input)
{
	if(!game.world.paused)
		game.players[client_id].on_predicted_input((NETOBJ_PLAYER_INPUT *)input);
	
	/*
	{
		
		on_predicted_input()
		if (memcmp(&game.players[client_id].input, input, sizeof(NETOBJ_PLAYER_INPUT)) != 0)
			game.players[client_id].last_action = server_tick();

		//game.players[client_id].previnput = game.players[client_id].input;
		game.players[client_id].input = *(NETOBJ_PLAYER_INPUT*)input;
		game.players[client_id].num_inputs++;
		
		if(game.players[client_id].input.target_x == 0 && game.players[client_id].input.target_y == 0)
			game.players[client_id].input.target_y = -1;
	}*/
}

// Server hooks
void mods_tick()
{
	game.tick();
}

void mods_snap(int client_id)
{
	game.snap(client_id);
}

void mods_client_enter(int client_id)
{
	//game.world.insert_entity(&game.players[client_id]);
	game.players[client_id].respawn();
	dbg_msg("game", "join player='%d:%s'", client_id, server_clientname(client_id));


	char buf[512];
	str_format(buf, sizeof(buf), "%s entered and joined the %s", server_clientname(client_id), game.controller->get_team_name(game.players[client_id].team));
	game.send_chat(-1, GAMECONTEXT::CHAT_ALL, buf); 

	dbg_msg("game", "team_join player='%d:%s' team=%d", client_id, server_clientname(client_id), game.players[client_id].team);
}

void mods_connected(int client_id)
{
	game.players[client_id].init(client_id);
	//game.players[client_id].client_id = client_id;
	
	// Check which team the player should be on
	if(config.sv_tournament_mode)
		game.players[client_id].team = -1;
	else
		game.players[client_id].team = game.controller->get_auto_team(client_id);

	// send motd
	NETMSG_SV_MOTD msg;
	msg.message = config.sv_motd;
	msg.pack(MSGFLAG_VITAL);
	server_send_msg(client_id);
}

void mods_client_drop(int client_id)
{
	game.players[client_id].on_disconnect();

}

void mods_message(int msgtype, int client_id)
{
	void *rawmsg = netmsg_secure_unpack(msgtype);
	if(!rawmsg)
	{
		dbg_msg("server", "dropped weird message '%s' (%d), failed on '%s'", netmsg_get_name(msgtype), msgtype, netmsg_failed_on());
		return;
	}
	
	if(msgtype == NETMSGTYPE_CL_SAY)
	{
		NETMSG_CL_SAY *msg = (NETMSG_CL_SAY *)rawmsg;
		int team = msg->team;
		if(team)
			team = game.players[client_id].team;
		else
			team = GAMECONTEXT::CHAT_ALL;
		
		if(config.sv_spamprotection && game.players[client_id].last_chat+time_freq() > time_get())
		{
			// consider this as spam
		}
		else
		{
			game.players[client_id].last_chat = time_get();
			game.send_chat(client_id, team, msg->message);
		}
	}
	else if (msgtype == NETMSGTYPE_CL_SETTEAM)
	{
		NETMSG_CL_SETTEAM *msg = (NETMSG_CL_SETTEAM *)rawmsg;

		// Switch team on given client and kill/respawn him
		if(game.controller->can_join_team(msg->team, client_id))
			game.players[client_id].set_team(msg->team);
		else
		{
			char buf[128];
			str_format(buf, sizeof(buf), "Only %d active players are allowed", config.sv_max_clients-config.sv_spectator_slots);
			game.send_broadcast(buf, client_id);
		}
	}
	else if (msgtype == NETMSGTYPE_CL_CHANGEINFO || msgtype == NETMSGTYPE_CL_STARTINFO)
	{
		NETMSG_CL_CHANGEINFO *msg = (NETMSG_CL_CHANGEINFO *)rawmsg;
		game.players[client_id].use_custom_color = msg->use_custom_color;
		game.players[client_id].color_body = msg->color_body;
		game.players[client_id].color_feet = msg->color_feet;

		// check for invalid chars
		/*
		unsigned char *p = (unsigned char *)name;
		while (*p)
		{
			if(*p < 32)
				*p = ' ';
			p++;
		}*/

		// copy old name
		char oldname[MAX_NAME_LENGTH];
		str_copy(oldname, server_clientname(client_id), MAX_NAME_LENGTH);
		
		server_setclientname(client_id, msg->name);
		if(msgtype == NETMSGTYPE_CL_CHANGEINFO && strcmp(oldname, server_clientname(client_id)) != 0)
		{
			char chattext[256];
			str_format(chattext, sizeof(chattext), "%s changed name to %s", oldname, server_clientname(client_id));
			game.send_chat(-1, GAMECONTEXT::CHAT_ALL, chattext);
		}
		
		// set skin
		str_copy(game.players[client_id].skin_name, msg->skin, sizeof(game.players[client_id].skin_name));
		
		game.controller->on_player_info_change(&game.players[client_id]);
		
		if(msgtype == NETMSGTYPE_CL_STARTINFO)
		{
			// a client that connected!
			
			// send all info to this client
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(game.players[i].client_id != -1)
					game.send_info(i, client_id);
			}

			// send tuning parameters to client
			send_tuning_params(client_id);
			
			//
			NETMSG_SV_READYTOENTER m;
			m.pack(MSGFLAG_VITAL|MSGFLAG_FLUSH);
			server_send_msg(client_id);			
		}
		
		game.send_info(client_id, -1);
	}
	else if (msgtype == NETMSGTYPE_CL_EMOTICON)
	{
		NETMSG_CL_EMOTICON *msg = (NETMSG_CL_EMOTICON *)rawmsg;
		game.send_emoticon(client_id, msg->emoticon);
	}
	else if (msgtype == NETMSGTYPE_CL_KILL)
	{
		//PLAYER *pplayer = get_player(client_id);
		game.players[client_id].kill_character(); //(client_id, -1);
	}
}

static void con_tune_param(void *result, void *user_data)
{
	const char *param_name = console_arg_string(result, 0);
	float new_value = console_arg_float(result, 1);

	if(tuning.set(param_name, new_value))
	{
		dbg_msg("tuning", "%s changed to %.2f", param_name, new_value);
		send_tuning_params(-1);
	}
	else
		console_print("No such tuning parameter");
}

static void con_tune_reset(void *result, void *user_data)
{
	TUNING_PARAMS p;
	tuning = p;
	send_tuning_params(-1);
	console_print("tuning reset");
}

static void con_tune_dump(void *result, void *user_data)
{
	for(int i = 0; i < tuning.num(); i++)
	{
		float v;
		tuning.get(i, &v);
		dbg_msg("tuning", "%s %.2f", tuning.names[i], v);
	}
}


static void con_restart(void *result, void *user_data)
{
	if(console_arg_num(result))
		game.controller->do_warmup(console_arg_int(result, 0));
	else
		game.controller->startround();
}

static void con_broadcast(void *result, void *user_data)
{
	game.send_broadcast(console_arg_string(result, 0), -1);
}

static void con_say(void *result, void *user_data)
{
	game.send_chat(-1, GAMECONTEXT::CHAT_ALL, console_arg_string(result, 0));
}

static void con_set_team(void *result, void *user_data)
{
	int client_id = clamp(console_arg_int(result, 0), 0, (int)MAX_CLIENTS);
	int team = clamp(console_arg_int(result, 1), -1, 1);
	
	dbg_msg("", "%d %d", client_id, team);
	
	if(game.players[client_id].client_id != client_id)
		return;
	
	game.players[client_id].set_team(team);
}

void mods_console_init()
{
	MACRO_REGISTER_COMMAND("tune", "si", con_tune_param, 0);
	MACRO_REGISTER_COMMAND("tune_reset", "", con_tune_reset, 0);
	MACRO_REGISTER_COMMAND("tune_dump", "", con_tune_dump, 0);

	MACRO_REGISTER_COMMAND("restart", "?i", con_restart, 0);
	MACRO_REGISTER_COMMAND("broadcast", "r", con_broadcast, 0);
	MACRO_REGISTER_COMMAND("say", "r", con_say, 0);
	MACRO_REGISTER_COMMAND("set_team", "ii", con_set_team, 0);
}

void mods_init()
{
	//if(!data) /* only load once */
		//data = load_data_from_memory(internal_data);
		
	for(int i = 0; i < NUM_NETOBJTYPES; i++)
		snap_set_staticsize(i, netobj_get_size(i));

	layers_init();
	col_init();

	// reset everything here
	//world = new GAMEWORLD;
	//players = new PLAYER[MAX_CLIENTS];

	// select gametype
	if(strcmp(config.sv_gametype, "ctf") == 0)
		game.controller = new GAMECONTROLLER_CTF;
	else if(strcmp(config.sv_gametype, "tdm") == 0)
		game.controller = new GAMECONTROLLER_TDM;
	else
		game.controller = new GAMECONTROLLER_DM;

	// setup core world
	//for(int i = 0; i < MAX_CLIENTS; i++)
	//	game.players[i].core.world = &game.world.core;

	// create all entities from the game layer
	MAPITEM_LAYER_TILEMAP *tmap = layers_game_layer();
	TILE *tiles = (TILE *)map_get_data(tmap->data);
	
	/*
	num_spawn_points[0] = 0;
	num_spawn_points[1] = 0;
	num_spawn_points[2] = 0;
	*/
	
	for(int y = 0; y < tmap->height; y++)
	{
		for(int x = 0; x < tmap->width; x++)
		{
			int index = tiles[y*tmap->width+x].index - ENTITY_OFFSET;
			vec2 pos(x*32.0f+16.0f, y*32.0f+16.0f);
			game.controller->on_entity(index, pos);
		}
	}

	//game.world.insert_entity(game.controller);

	if(config.dbg_dummies)
	{
		for(int i = 0; i < config.dbg_dummies ; i++)
		{
			mods_connected(MAX_CLIENTS-i-1);
			mods_client_enter(MAX_CLIENTS-i-1);
			if(game.controller->gametype != GAMETYPE_DM)
				game.players[MAX_CLIENTS-i-1].team = i&1;
		}
	}
}

void mods_shutdown()
{
	delete game.controller;
	game.controller = 0;
}

void mods_presnap() {}
void mods_postsnap()
{
	game.events.clear();
}

extern "C" const char *mods_net_version() { return GAME_NETVERSION; }
extern "C" const char *mods_version() { return GAME_VERSION; }
