#include <string.h>

#include <engine/e_client_interface.h>

extern "C" {
	#include <engine/e_config.h>
	#include <engine/client/ec_font.h>
	#include <engine/e_console.h>
};

#include <game/generated/gc_data.h>
#include <game/g_game.h>
#include <game/g_version.h>

#include <game/g_layers.h>

#include "gc_client.h"
#include "gc_skin.h"
#include "gc_render.h"
#include "gc_map_image.h"
#include "gc_console.h"

extern unsigned char internal_data[];

extern void menu_init();
extern bool menu_active;
extern bool menu_game_active;

extern "C" void modc_console_init()
{
	client_console_init();
}

//binds_save()

static void load_sounds_thread(void *)
{
	// load sounds
	for(int s = 0; s < data->num_sounds; s++)
	{
		//render_loading(current/total);
		for(int i = 0; i < data->sounds[s].num_sounds; i++)
		{
			int id = snd_load_wv(data->sounds[s].sounds[i].filename);
			data->sounds[s].sounds[i].id = id;
		}
	}
}

extern "C" void modc_init()
{
	static FONT_SET default_font;
	int64 start = time_get();
	
	// setup input stack
	input_stack.add_handler(console_input_special_binds, 0); // F1-Fx binds
	input_stack.add_handler(console_input_cli, 0); // console
	input_stack.add_handler(chat_input_handle, 0); // chat
	//input_stack.add_handler() // ui
	input_stack.add_handler(console_input_normal_binds, 0); // binds
	
	
	int before = gfx_memory_usage();
	font_set_load(&default_font, "data/fonts/default_font%d.tfnt", "data/fonts/default_font%d.png", "data/fonts/default_font%d_b.png", 14, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 36);
	dbg_msg("font", "gfx memory used for font textures: %d", gfx_memory_usage()-before);
	
	gfx_text_set_default_font(&default_font);

	particle_reset();
	menu_init();
	
	// setup sound channels
	snd_set_channel(CHN_GUI, 1.0f, 0.0f);
	snd_set_channel(CHN_MUSIC, 1.0f, 0.0f);
	snd_set_channel(CHN_WORLD, 0.9f, 1.0f);
	snd_set_channel(CHN_GLOBAL, 1.0f, 0.0f);

	// load the data container
	data = load_data_from_memory(internal_data);

	// TODO: should be removed
	snd_set_listener_pos(0.0f, 0.0f);

	float total = data->num_images;
	float current = 0;
	
	// load textures
	for(int i = 0; i < data->num_images; i++)
	{
		render_loading(current/total);
		data->images[i].id = gfx_load_texture(data->images[i].filename, IMG_AUTO);
		current++;
	}

	skin_init();
	
	//load_sounds_thread(0);
	thread_create(load_sounds_thread, 0);

	// load sounds
	/*
	for(int s = 0; s < data->num_sounds; s++)
	{
		render_loading(current/total);
		for(int i = 0; i < data->sounds[s].num_sounds; i++)
		{
			int id;
			//if (strcmp(data->sounds[s].sounds[i].filename + strlen(data->sounds[s].sounds[i].filename) - 3, ".wv") == 0)
			id = snd_load_wv(data->sounds[s].sounds[i].filename);
			//else
			//	id = snd_load_wav(data->sounds[s].sounds[i].filename);

			data->sounds[s].sounds[i].id = id;
		}

		current++;
	}*/
	
	
	int64 end = time_get();
	dbg_msg("", "%f.2ms", ((end-start)*1000)/(float)time_freq());
}

extern "C" void modc_save_config()
{
	binds_save();
}

extern "C" void modc_entergame()
{
}

extern "C" void modc_shutdown()
{
	// shutdown the menu
}


player_core predicted_prev_player;
player_core predicted_player;
static int predicted_tick = 0;
static int last_new_predicted_tick = -1;

extern "C" void modc_predict()
{
	player_core before_prev_player = predicted_prev_player;
	player_core before_player = predicted_player;

	// repredict player
	world_core world;
	world.tuning = tuning;
	int local_cid = -1;

	// search for players
	for(int i = 0; i < snap_num_items(SNAP_CURRENT); i++)
	{
		SNAP_ITEM item;
		const void *data = snap_get_item(SNAP_CURRENT, i, &item);
		int client_id = item.id;

		if(item.type == NETOBJTYPE_PLAYER_CHARACTER)
		{
			const NETOBJ_PLAYER_CHARACTER *character = (const NETOBJ_PLAYER_CHARACTER *)data;
			client_datas[client_id].predicted.world = &world;
			world.players[client_id] = &client_datas[client_id].predicted;

			client_datas[client_id].predicted.read(character);
		}
		else if(item.type == NETOBJTYPE_PLAYER_INFO)
		{
			const NETOBJ_PLAYER_INFO *info = (const NETOBJ_PLAYER_INFO *)data;
			if(info->local)
				local_cid = client_id;
		}
	}
	
	// we can't predict without our own id
	if(local_cid == -1)
		return;

	// predict
	for(int tick = client_tick()+1; tick <= client_predtick(); tick++)
	{
		// fetch the local
		if(tick == client_predtick() && world.players[local_cid])
			predicted_prev_player = *world.players[local_cid];
		
		// first calculate where everyone should move
		for(int c = 0; c < MAX_CLIENTS; c++)
		{
			if(!world.players[c])
				continue;

			mem_zero(&world.players[c]->input, sizeof(world.players[c]->input));
			if(local_cid == c)
			{
				// apply player input
				int *input = client_get_input(tick);
				if(input)
					world.players[c]->input = *((NETOBJ_PLAYER_INPUT*)input);
			}

			world.players[c]->tick();
		}

		// move all players and quantize their data
		for(int c = 0; c < MAX_CLIENTS; c++)
		{
			if(!world.players[c])
				continue;

			world.players[c]->move();
			world.players[c]->quantize();
		}
		
		if(tick > last_new_predicted_tick)
		{
			last_new_predicted_tick = tick;
			
			if(local_cid != -1 && world.players[local_cid])
			{
				vec2 pos = world.players[local_cid]->pos;
				int events = world.players[local_cid]->triggered_events;
				if(events&COREEVENT_GROUND_JUMP) snd_play_random(CHN_WORLD, SOUND_PLAYER_JUMP, 1.0f, pos);
				if(events&COREEVENT_AIR_JUMP)
				{
					effect_air_jump(pos);
					snd_play_random(CHN_WORLD, SOUND_PLAYER_AIRJUMP, 1.0f, pos);
				}
				//if(events&COREEVENT_HOOK_LAUNCH) snd_play_random(CHN_WORLD, SOUND_HOOK_LOOP, 1.0f, pos);
				//if(events&COREEVENT_HOOK_ATTACH_PLAYER) snd_play_random(CHN_WORLD, SOUND_HOOK_ATTACH_PLAYER, 1.0f, pos);
				if(events&COREEVENT_HOOK_ATTACH_GROUND) snd_play_random(CHN_WORLD, SOUND_HOOK_ATTACH_GROUND, 1.0f, pos);
				//if(events&COREEVENT_HOOK_RETRACT) snd_play_random(CHN_WORLD, SOUND_PLAYER_JUMP, 1.0f, pos);
			}


			/*
			dbg_msg("predict", "%d %d %d", tick,
				(int)world.players[c]->pos.x, (int)world.players[c]->pos.y,
				(int)world.players[c]->vel.x, (int)world.players[c]->vel.y);*/
		}
		
		if(tick == client_predtick() && world.players[local_cid])
			predicted_player = *world.players[local_cid];
	}
	
	if(config.debug && predicted_tick == client_predtick())
	{
		if(predicted_player.pos.x != before_player.pos.x ||
			predicted_player.pos.y != before_player.pos.y)
		{
			dbg_msg("client", "prediction error, (%d %d) (%d %d)", 
				(int)before_player.pos.x, (int)before_player.pos.y,
				(int)predicted_player.pos.x, (int)predicted_player.pos.y);
		}

		if(predicted_prev_player.pos.x != before_prev_player.pos.x ||
			predicted_prev_player.pos.y != before_prev_player.pos.y)
		{
			dbg_msg("client", "prediction error, prev (%d %d) (%d %d)", 
				(int)before_prev_player.pos.x, (int)before_prev_player.pos.y,
				(int)predicted_prev_player.pos.x, (int)predicted_prev_player.pos.y);
		}
	}
	
	predicted_tick = client_predtick();
}


extern "C" void modc_newsnapshot()
{
	static int snapshot_count = 0;
	snapshot_count++;
	
	// secure snapshot
	{
		int num = snap_num_items(SNAP_CURRENT);
		for(int index = 0; index < num; index++)
		{
			SNAP_ITEM item;
			void *data = snap_get_item(SNAP_CURRENT, index, &item);
			if(netobj_secure(item.type, data, item.datasize) != 0)
			{
				if(config.debug)
					dbg_msg("game", "invalidated %d %d (%s) %d", index, item.type, netobj_get_name(item.type), item.id);
				snap_invalidate_item(SNAP_CURRENT, index);
			}
		}
	}
	
	
	process_events(SNAP_CURRENT);

	if(config.dbg_stress)
	{
		if((client_tick()%250) == 0)
		{
			NETMSG_CL_SAY msg;
			msg.team = -1;
			msg.message = "galenskap!!!!";
			msg.pack(MSGFLAG_VITAL);
			client_send_msg();
		}
	}

	clear_object_pointers();

	// setup world view
	{
		// 1. fetch local player
		// 2. set him to the center
		int num = snap_num_items(SNAP_CURRENT);
		for(int i = 0; i < num; i++)
		{
			SNAP_ITEM item;
			const void *data = snap_get_item(SNAP_CURRENT, i, &item);

			if(item.type == NETOBJTYPE_PLAYER_INFO)
			{
				const NETOBJ_PLAYER_INFO *info = (const NETOBJ_PLAYER_INFO *)data;
				
				client_datas[info->cid].team = info->team;
				
				if(info->local)
				{
					netobjects.local_info = info;
					const void *data = snap_find_item(SNAP_CURRENT, NETOBJTYPE_PLAYER_CHARACTER, item.id);
					if(data)
					{
						netobjects.local_character = (const NETOBJ_PLAYER_CHARACTER *)data;
						local_character_pos = vec2(netobjects.local_character->x, netobjects.local_character->y);

						const void *p = snap_find_item(SNAP_PREV, NETOBJTYPE_PLAYER_CHARACTER, item.id);
						if(p)
							netobjects.local_prev_character = (NETOBJ_PLAYER_CHARACTER *)p;
					}
				}
			}
			else if(item.type == NETOBJTYPE_GAME)
				netobjects.gameobj = (NETOBJ_GAME *)data;
			else if(item.type == NETOBJTYPE_FLAG)
			{
				netobjects.flags[item.id%2] = (const NETOBJ_FLAG *)data;
			}
		}
	}

	for(int i = 0; i < MAX_CLIENTS; i++)
		client_datas[i].update_render_info();
}

extern "C" void modc_render()
{
	// this should be moved around abit
	if(client_state() == CLIENTSTATE_ONLINE)
		render_game();
	else
		menu_render();

	input_stack.dispatch_input();
	console_render();
}

extern "C" void modc_rcon_line(const char *line)
{
	console_rcon_print(line);
}

extern "C" int modc_snap_input(int *data)
{
	static NETOBJ_PLAYER_INPUT last_data = {0};
	
	// update player state
	if(chat_mode != CHATMODE_NONE)
		input_data.player_state = PLAYERSTATE_CHATTING;
	else if(menu_active)
		input_data.player_state = PLAYERSTATE_IN_MENU;
	else
		input_data.player_state = PLAYERSTATE_PLAYING;
	last_data.player_state = input_data.player_state;
	
	// we freeze the input if chat or menu is activated
	if(menu_active || chat_mode != CHATMODE_NONE || console_active())
	{
		last_data.left = 0;
		last_data.right = 0;
		last_data.hook = 0;
		last_data.jump = 0;
		
		input_data = last_data;
			
		mem_copy(data, &input_data, sizeof(input_data));
		return sizeof(input_data);
	}
	
	picked_up_weapon = -1;

	if(!input_target_lock)
	{
		input_data.target_x = (int)mouse_pos.x;
		input_data.target_y = (int)mouse_pos.y;
		
		if(!input_data.target_x && !input_data.target_y)
			input_data.target_y = 1;
	}

	input_target_lock = 0;

	// stress testing
	if(config.dbg_stress)
	{
		float t = client_localtime();
		mem_zero(&input_data, sizeof(input_data));

		input_data.left = ((int)t/2)&1;
		input_data.right = ((int)t/2+1)&1;
		input_data.jump = ((int)t);
		input_data.fire = ((int)(t*10));
		input_data.hook = ((int)(t*2))&1;
		input_data.wanted_weapon = ((int)t)%NUM_WEAPONS;
		input_data.target_x = (int)(sinf(t*3)*100.0f);
		input_data.target_y = (int)(cosf(t*3)*100.0f);
	}

	// copy and return size	
	last_data = input_data;
	mem_copy(data, &input_data, sizeof(input_data));
	return sizeof(input_data);
}

void menu_do_disconnected();
void menu_do_connecting();
void menu_do_connected();

extern "C" void modc_statechange(int state, int old)
{
	clear_object_pointers();
	
	if(state == CLIENTSTATE_OFFLINE)
	{
	 	menu_do_disconnected();
	 	menu_game_active = false;
	}
	else if(state == CLIENTSTATE_LOADING)
		menu_do_connecting();
	else if(state == CLIENTSTATE_CONNECTING)
		menu_do_connecting();
	else if (state == CLIENTSTATE_ONLINE)
	{
		menu_active = false;
	 	menu_game_active = true;
	 	//snapshot_count = 0;
	 	
		menu_do_connected();
	}
}

NETOBJ_PROJECTILE extraproj_projectiles[MAX_EXTRA_PROJECTILES];
int extraproj_num;

void extraproj_reset()
{
	extraproj_num = 0;
}

char server_motd[900] = {0};

extern "C" void modc_message(int msgtype)
{
	// special messages
	if(msgtype == NETMSGTYPE_SV_EXTRA_PROJECTILE)
	{
		int num = msg_unpack_int();
		
		for(int k = 0; k < num; k++)
		{
			NETOBJ_PROJECTILE proj;
			for(unsigned i = 0; i < sizeof(NETOBJ_PROJECTILE)/sizeof(int); i++)
				((int *)&proj)[i] = msg_unpack_int();
				
			if(msg_unpack_error())
				return;
				
			if(extraproj_num != MAX_EXTRA_PROJECTILES)
			{
				extraproj_projectiles[extraproj_num] = proj;
				extraproj_num++;
			}
		}
		
		return;
	}
	else if(msgtype == NETMSGTYPE_SV_TUNE_PARAMS)
	{
		// unpack the new tuning
		tuning_params new_tuning;
		int *params = (int *)&new_tuning;
		for(unsigned i = 0; i < sizeof(tuning_params)/sizeof(int); i++)
			params[i] = msg_unpack_int();

		// check for unpacking errors
		if(msg_unpack_error())
			return;
			
		// apply new tuning
		tuning = new_tuning;
	}
	
	// normal 
	void *rawmsg = netmsg_secure_unpack(msgtype);
	if(!rawmsg)
	{
		dbg_msg("client", "dropped weird message '%s' (%d), failed on '%s'", netmsg_get_name(msgtype), msgtype, netmsg_failed_on());
		return;
	}
		
	if(msgtype == NETMSGTYPE_SV_CHAT)
	{
		NETMSG_SV_CHAT *msg = (NETMSG_SV_CHAT *)rawmsg;
		chat_add_line(msg->cid, msg->team, msg->message);

		if(msg->cid >= 0)
			snd_play(CHN_GUI, data->sounds[SOUND_CHAT_CLIENT].sounds[0].id, 0);
		else
			snd_play(CHN_GUI, data->sounds[SOUND_CHAT_SERVER].sounds[0].id, 0);
	}
	else if(msgtype == NETMSGTYPE_SV_MOTD)
	{
		NETMSG_SV_MOTD *msg = (NETMSG_SV_MOTD *)rawmsg;

		// process escaping			
		str_copy(server_motd, msg->message, sizeof(server_motd));
		for(int i = 0; server_motd[i]; i++)
		{
			if(server_motd[i] == '\\')
			{
				if(server_motd[i+1] == 'n')
				{
					server_motd[i] = ' ';
					server_motd[i+1] = '\n';
					i++;
				}
			}
		}
			
		dbg_msg("game", "MOTD: %s", server_motd);
	}
	else if(msgtype == NETMSGTYPE_SV_SETINFO)
	{
		NETMSG_SV_SETINFO *msg = (NETMSG_SV_SETINFO *)rawmsg;
		
		str_copy(client_datas[msg->cid].name, msg->name, 64);
		str_copy(client_datas[msg->cid].skin_name, msg->skin, 64);
		
		// make sure that we don't set a special skin on the client
		if(client_datas[msg->cid].skin_name[0] == 'x' || client_datas[msg->cid].skin_name[1] == '_')
			str_copy(client_datas[msg->cid].skin_name, "default", 64);
		
		client_datas[msg->cid].skin_info.color_body = skin_get_color(msg->color_body);
		client_datas[msg->cid].skin_info.color_feet = skin_get_color(msg->color_feet);
		client_datas[msg->cid].skin_info.size = 64;
		
		// find new skin
		client_datas[msg->cid].skin_id = skin_find(client_datas[msg->cid].skin_name);
		if(client_datas[msg->cid].skin_id < 0)
			client_datas[msg->cid].skin_id = 0;
		
		if(msg->use_custom_color)
			client_datas[msg->cid].skin_info.texture = skin_get(client_datas[msg->cid].skin_id)->color_texture;
		else
		{
			client_datas[msg->cid].skin_info.texture = skin_get(client_datas[msg->cid].skin_id)->org_texture;
			client_datas[msg->cid].skin_info.color_body = vec4(1,1,1,1);
			client_datas[msg->cid].skin_info.color_feet = vec4(1,1,1,1);
		}

		client_datas[msg->cid].update_render_info();
	}
    else if(msgtype == NETMSGTYPE_SV_WEAPON_PICKUP)
    {
    	NETMSG_SV_WEAPON_PICKUP *msg = (NETMSG_SV_WEAPON_PICKUP *)rawmsg;
        picked_up_weapon = msg->weapon+1;
    }
	else if(msgtype == NETMSGTYPE_SV_READY_TO_ENTER)
	{
		client_entergame();
	}
	else if(msgtype == NETMSGTYPE_SV_KILLMSG)
	{
		NETMSG_SV_KILLMSG *msg = (NETMSG_SV_KILLMSG *)rawmsg;
		
		// unpack messages
		killmsg kill;
		kill.killer = msg->killer;
		kill.victim = msg->victim;
		kill.weapon = msg->weapon;
		kill.mode_special = msg->mode_special;
		kill.tick = client_tick();

		// add the message
		killmsg_current = (killmsg_current+1)%killmsg_max;
		killmsgs[killmsg_current] = kill;
	}
	else if (msgtype == NETMSGTYPE_SV_EMOTICON)
	{
		NETMSG_SV_EMOTICON *msg = (NETMSG_SV_EMOTICON *)rawmsg;

		// apply
		client_datas[msg->cid].emoticon = msg->emoticon;
		client_datas[msg->cid].emoticon_start = client_tick();
	}
	else if(msgtype == NETMSGTYPE_SV_SOUND_GLOBAL)
	{
		NETMSG_SV_SOUND_GLOBAL *msg = (NETMSG_SV_SOUND_GLOBAL *)rawmsg;
		snd_play_random(CHN_GLOBAL, msg->soundid, 1.0f, vec2(0,0));
	}
}

extern "C" void modc_connected()
{
	// init some stuff
	layers_init();
	col_init();
	img_init();
	flow_init();
	
	render_tilemap_generate_skip();
	
	//tilemap_init();
	chat_reset();
	particle_reset();
	extraproj_reset();
	
	clear_object_pointers();
	last_new_predicted_tick = -1;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		client_datas[i].name[0] = 0;
		client_datas[i].skin_id = 0;
		client_datas[i].team = 0;
		client_datas[i].emoticon = 0;
		client_datas[i].emoticon_start = -1;
		client_datas[i].skin_info.texture = skin_get(0)->color_texture;
		client_datas[i].skin_info.color_body = vec4(1,1,1,1);
		client_datas[i].skin_info.color_feet = vec4(1,1,1,1);
		client_datas[i].update_render_info();
	}

	for(int i = 0; i < killmsg_max; i++)
		killmsgs[i].tick = -100000;
	send_info(true);
}

extern "C" const char *modc_net_version() { return TEEWARS_NETVERSION; }
