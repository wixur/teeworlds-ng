#include <string.h>
#include <engine/e_client_interface.h>
#include <game/generated/g_protocol.hpp>
#include <game/generated/gc_data.hpp>
#include <game/client/gameclient.hpp>
#include <game/client/animstate.hpp>
#include <game/client/render.hpp>
#include <game/client/components/motd.hpp>
#include "scoreboard.hpp"


SCOREBOARD::SCOREBOARD()
{
	on_reset();
}

void SCOREBOARD::con_key_scoreboard(void *result, void *user_data)
{
	((SCOREBOARD *)user_data)->active = console_arg_int(result, 0) != 0;
}

void SCOREBOARD::on_reset()
{
	active = false;
}

void SCOREBOARD::on_console_init()
{
	MACRO_REGISTER_COMMAND("+scoreboard", "", CFGFLAG_CLIENT, con_key_scoreboard, this, "Show scoreboard");
}

void SCOREBOARD::render_goals(float x, float y, float w)
{
	float h = 50.0f;

	gfx_blend_normal();
	gfx_texture_set(-1);
	gfx_quads_begin();
	gfx_setcolor(0,0,0,0.5f);
	draw_round_rect(x-10.f, y-10.f, w, h, 10.0f);
	gfx_quads_end();

	// render goals
	//y = ystart+h-54;
	float tw = 0.0f;
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->score_limit)
	{
		char buf[64];
		str_format(buf, sizeof(buf), "%s: %d" ,localize("Score limit"), gameclient.snap.gameobj->score_limit);
		gfx_text(0, x+20.0f, y, 22.0f, buf, -1);
		tw += gfx_text_width(0, 22.0f, buf, -1);
	}
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->time_limit)
	{
		char buf[64];
		str_format(buf, sizeof(buf), "%s: %d" ,localize("Time limit"), gameclient.snap.gameobj->time_limit);
		gfx_text(0, x+220.0f, y, 22.0f, buf, -1);
		tw += gfx_text_width(0, 22.0f, buf, -1);
	}
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->round_num && gameclient.snap.gameobj->round_current)
	{
		char buf[64];
		str_format(buf, sizeof(buf), "%s: %d" ,localize("Round"), gameclient.snap.gameobj->round_current, gameclient.snap.gameobj->round_num);
		gfx_text(0, x+450.0f, y, 22.0f, buf, -1);

	/*[48c3fd4c][game/scoreboard]: timelimit x:219.428558
	[48c3fd4c][game/scoreboard]: round x:453.142822*/
	}
}

void SCOREBOARD::render_spectators(float x, float y, float w)
{
	char buffer[1024*4];
	int count = 0;
	float h = 120.0f;

	str_copy(buffer, localize("Spectators"), sizeof(buffer));

	gfx_blend_normal();
	gfx_texture_set(-1);
	gfx_quads_begin();
	gfx_setcolor(0,0,0,0.5f);
	draw_round_rect(x-10.f, y-10.f, w, h, 10.0f);
	gfx_quads_end();

	for(int i = 0; i < snap_num_items(SNAP_CURRENT); i++)
	{
		SNAP_ITEM item;
		const void *data = snap_get_item(SNAP_CURRENT, i, &item);

		if(item.type == NETOBJTYPE_PLAYER_INFO)
		{
			const NETOBJ_PLAYER_INFO *info = (const NETOBJ_PLAYER_INFO *)data;
			if(info->team == -1)
			{
				if(count)
					strcat(buffer, ", ");
				strcat(buffer, gameclient.clients[info->cid].name);
				count++;
			}
		}
	}
	if (config.gfx_custom_hud_colors)
	{
		gfx_text_color(config.gfx_spectator_hud_color_r * 1.0f / 255.0f, config.gfx_spectator_hud_color_g * 1.0f / 255.0f, config.gfx_spectator_hud_color_b * 1.0f / 255.0f, 1.0f);
	}
	gfx_text(0, x+10, y, 32, buffer, (int)w-20);
}

void SCOREBOARD::render_scoreboard(float x, float y, float w, int team, const char *title)
{
	//float ystart = y;
	float h = 750.0f;

	gfx_blend_normal();
	gfx_texture_set(-1);
	gfx_quads_begin();
	gfx_setcolor(0,0,0,0.5f);
	draw_round_rect(x-10.f, y-10.f, w, h, 17.0f);
	gfx_quads_end();

	// render title
	if(!title)
	{
		if(gameclient.snap.gameobj->game_over)
			title = localize("Game over");
		else
			title = localize("Score board");
	}

	float tw = gfx_text_width(0, 48, title, -1);

	if(team == -1)
	{
		if (config.gfx_custom_hud_colors)
		{
			gfx_text_color(config.gfx_player_hud_color_r * 1.0f / 255.0f, config.gfx_player_hud_color_g * 1.0f / 255.0f, config.gfx_player_hud_color_b * 1.0f / 255.0f, 1.0f);
		}
		gfx_text(0, x+w/2-tw/2, y, 48, title, -1);
	}
	else
	{
		gfx_text(0, x+10, y, 48, title, -1);

		if(gameclient.snap.gameobj)
		{
			char buf[128];
			int score = team ? gameclient.snap.gameobj->teamscore_blue : gameclient.snap.gameobj->teamscore_red;
			str_format(buf, sizeof(buf), "%d", score);
			tw = gfx_text_width(0, 48, buf, -1);
			if (config.gfx_custom_hud_colors)
			{
				if (team == 0)
					gfx_text_color(config.gfx_red_hud_color_r * 1.0f / 255.0f, config.gfx_red_hud_color_g * 1.0f / 255.0f, config.gfx_red_hud_color_b * 1.0f / 255.0f, 1.0f);
				else
					gfx_text_color(config.gfx_blue_hud_color_r * 1.0f / 255.0f, config.gfx_blue_hud_color_g * 1.0f / 255.0f, config.gfx_blue_hud_color_b * 1.0f / 255.0f, 1.0f);
			}
			gfx_text(0, x+w-tw-30, y, 48, buf, -1);
		}
	}

	y += 54.0f;

	// find players
	const NETOBJ_PLAYER_INFO *players[MAX_CLIENTS] = {0};
	int num_players = 0;
	for(int i = 0; i < snap_num_items(SNAP_CURRENT); i++)
	{
		SNAP_ITEM item;
		const void *data = snap_get_item(SNAP_CURRENT, i, &item);

		if(item.type == NETOBJTYPE_PLAYER_INFO)
		{
			const NETOBJ_PLAYER_INFO *info = (const NETOBJ_PLAYER_INFO *)data;
			if(info->team == team)
			{
				players[num_players] = info;
				num_players++;
			}
		}
	}

	// sort players
	for(int k = 0; k < num_players; k++) // ffs, bubblesort
	{
		for(int i = 0; i < num_players-k-1; i++)
		{
			if(players[i]->score < players[i+1]->score)
			{
				const NETOBJ_PLAYER_INFO *tmp = players[i];
				players[i] = players[i+1];
				players[i+1] = tmp;
			}
		}
	}

	// render headlines
	gfx_text(0, x+10, y, 24.0f, localize("Score"), -1);
	gfx_text(0, x+125, y, 24.0f, localize("Name"), -1);
	float tw_ = gfx_text_width(0, 24.0f, localize("Ping"), -1);
	gfx_text(0, x+w-50-tw_/2, y, 24.0f, localize("Ping"), -1);
	y += 29.0f;

	float font_size = 35.0f;
	float line_height = 50.0f;
	float tee_sizemod = 1.0f;
	float tee_offset = 0.0f;

	if(num_players > 13)
	{
		font_size = 30.0f;
		line_height = 40.0f;
		tee_sizemod = 0.8f;
		tee_offset = -5.0f;
	}

	// render player scores
	for(int i = 0; i < num_players; i++)
	{
		const NETOBJ_PLAYER_INFO *info = players[i];

		// make sure that we render the correct team

		char buf[128];
		if(info->local)
		{
			// background so it's easy to find the local player
			gfx_texture_set(-1);
			gfx_quads_begin();
			gfx_setcolor(1,1,1,0.25f);
			draw_round_rect(x, y, w-20, line_height*0.95f, 17.0f);
			gfx_quads_end();
		}

		if (config.gfx_custom_hud_colors)
		{
			if (gameclient.snap.gameobj->flags&&GAMEFLAG_TEAMS && team >= 0)
			{
				if (team == 0)
					gfx_text_color(config.gfx_red_hud_color_r * 1.0f / 255.0f, config.gfx_red_hud_color_g * 1.0f / 255.0f, config.gfx_red_hud_color_b * 1.0f / 255.0f, 1.0f);
				else
					gfx_text_color(config.gfx_blue_hud_color_r * 1.0f / 255.0f, config.gfx_blue_hud_color_g * 1.0f / 255.0f, config.gfx_blue_hud_color_b * 1.0f / 255.0f, 1.0f);
			} else {
				gfx_text_color(config.gfx_player_hud_color_r * 1.0f / 255.0f, config.gfx_player_hud_color_g * 1.0f / 255.0f, config.gfx_player_hud_color_b * 1.0f / 255.0f, 1.0f);
			}
		}
		gfx_text_color(1,1,1,1);

		str_format(buf, sizeof(buf), "%4d", info->score);
		gfx_text(0, x+60-gfx_text_width(0, font_size,buf,-1), y, font_size, buf, -1);

		gfx_text(0, x+128, y, font_size, gameclient.clients[info->cid].name, -1);

		str_format(buf, sizeof(buf), "%4d", info->latency);
		float tw = gfx_text_width(0, font_size, buf, -1);
		gfx_text(0, x+w-tw-35, y, font_size, buf, -1);

		// render avatar
		if((gameclient.snap.flags[0] && gameclient.snap.flags[0]->carried_by == info->cid) ||
			(gameclient.snap.flags[1] && gameclient.snap.flags[1]->carried_by == info->cid))
		{
			gfx_blend_normal();
			gfx_texture_set(data->images[IMAGE_GAME].id);
			gfx_quads_begin();

			if(info->team == 0) select_sprite(SPRITE_FLAG_BLUE, SPRITE_FLAG_FLIP_X);
			else select_sprite(SPRITE_FLAG_RED, SPRITE_FLAG_FLIP_X);

			float size = 64.0f;
			gfx_quads_drawTL(x+55, y-15, size/2, size);
			gfx_quads_end();
		}

		TEE_RENDER_INFO teeinfo = gameclient.clients[info->cid].render_info;
		teeinfo.size *= tee_sizemod;
		render_tee(ANIMSTATE::get_idle(), &teeinfo, EMOTE_NORMAL, vec2(1,0), vec2(x+90, y+28+tee_offset));


		y += line_height;
	}

	gfx_text_color(1, 1, 1, 1);
}

void SCOREBOARD::render_new()
{
	if (!data || !gameclient.snap.gameobj) return;

	float width = 400*3.0f*gfx_screenaspect();
	float height = 400*3.0f;
	float w = 800.0f;
    if(config.cl_new_scoreboard_full)
        w= w+600.0f;
	float h = 900.0f;
	float x = (width - w) / 2;
	float y = (height - h) / 2;

	gfx_mapscreen(0, 0, width, height);

	RECT main_view;
	main_view.x = x;
	main_view.y = y;
	main_view.w = w;
	main_view.h = h;

	ui_draw_rect(&main_view, vec4(0.0f, 0.0f, 0.0f, 0.5f), CORNER_ALL, 10.0f);

	RECT header, footer;

	ui_hsplit_t(&main_view, 40.0f, &header, &main_view);
	ui_draw_rect(&header, vec4(1.0f, 1.0f, 1.0f, 0.25f), CORNER_T, 10.0f);

	ui_hsplit_b(&main_view, 35.0f, &main_view, &footer);
	ui_draw_rect(&footer, vec4(1.0f, 1.0f, 1.0f, 0.25f), CORNER_B, 10.0f);

	ui_vsplit_l(&header, 15.0f, 0, &header);
	ui_vsplit_r(&header, 15.0f, &header, 0);

	ui_margin(&main_view, 10.0f, &main_view);

	ui_vsplit_l(&header, 15.0f, 0, &main_view);
	ui_vsplit_r(&header, 15.0f, &main_view, 0);

	ui_vsplit_l(&footer, 25.0f, 0, &footer);
	ui_vsplit_r(&footer, 25.0f, &footer, 0);

	main_view.w += 10.0f;

	if(gameclient.snap.gameobj && gameclient.snap.gameobj->score_limit)
	{
		char buf[64];
		str_format(buf, sizeof(buf),"%s: %d" ,localize("Score limit"), gameclient.snap.gameobj->score_limit);
		ui_do_label(&footer, buf, footer.h * 0.8f, -1);
	}

	ui_vsplit_l(&footer, 400.0f, 0, &footer);

	if(gameclient.snap.gameobj && gameclient.snap.gameobj->time_limit)
	{
		char buf[64];
		str_format(buf, sizeof(buf), "%s: %d" ,localize("Time limit"), gameclient.snap.gameobj->time_limit);
		ui_do_label(&footer, buf, footer.h * 0.8f, -1);
	}

	ui_vsplit_l(&footer, 800.0f, 0, &footer);

	if(gameclient.snap.gameobj && gameclient.snap.gameobj->round_num && gameclient.snap.gameobj->round_current)
	{
		char buf[64];
		str_format(buf, sizeof(buf), "%s: %d" ,localize("Round"), gameclient.snap.gameobj->round_current, gameclient.snap.gameobj->round_num);
		ui_do_label(&footer, buf, footer.h * 0.8f, -1);
	}

	float header_width = header.w;

	ui_vsplit_l(&header, 50.0f, 0, &header);
	ui_do_label(&header, localize("Name"), header.h * 0.8f, -1);


	ui_vsplit_l(&header, 350.0f, 0, &header);

	{
		RECT line_t = header;
		line_t.x += abs(125.0f - gfx_text_width(0, header.h * 0.8f, localize("Score"), -1)) / 2.0f;
		ui_do_label(&line_t, localize("Score"), header.h * 0.8f, -1);
	}

	ui_vsplit_l(&header, 125.0f, 0, &header);

	{
		RECT line_t = header;
		line_t.x += abs(75.0f - gfx_text_width(0, header.h * 0.8f, localize("Ping"), -1)) / 2.0f;
		ui_do_label(&line_t, localize("Ping"), header.h * 0.8f, -1);
	}

	ui_vsplit_l(&header, 75.0f, 0, &header);
	
	if(config.cl_new_scoreboard_full)
    {
		RECT line_t = header;
		line_t.x += abs(-45.0f - gfx_text_width(0, header.h * 0.8f, localize("Deaths"), -1)) / 2.0f;
		ui_do_label(&line_t, localize("Deaths"), header.h * 0.8f, -1);
	}

	float sprite_size = header.h * 0.8f;
	float spacing;
    if(config.cl_new_scoreboard_full)
            {
        	if (gameclient.snap.gameobj->flags&GAMEFLAG_FLAGS)
	    	    spacing = (header_width - 50.0f - 350.0f - 125.0f - 75.0f) / (NUM_WEAPONS + 3);
	         else
		        spacing = (header_width - 50.0f - 350.0f - 125.0f - 75.0f) / (NUM_WEAPONS + 2);
            }
    if(config.cl_new_scoreboard_full)
    {
	gfx_texture_set(data->images[IMAGE_GAME].id);
	gfx_quads_begin();

	gfx_quads_setrotation(0);

	{
		select_sprite(&data->sprites[SPRITE_STAR1]);
		gfx_quads_draw(header.x + spacing * 0.5f, header.y + sprite_size / 2.0f + header.h * 0.1f, sprite_size, sprite_size);
		ui_vsplit_l(&header, spacing, 0, &header);
	}
	{
//		select_sprite(&data->sprites[SPRITE_RED_MINUS]);
//		select_sprite(&data->sprites[SPRITE_STAR1]);
//		gfx_quads_draw(header.x + spacing * 0.5f, header.y + sprite_size / 2.0f + header.h * 0.1f, sprite_size, sprite_size);
		ui_vsplit_l(&header, spacing, 0, &header);
	}

	for (int i = 0; i < NUM_WEAPONS; i++)
	{
		select_sprite((i == WEAPON_HAMMER || i == WEAPON_NINJA) ? data->weapons.id[i].sprite_body : data->weapons.id[i].sprite_proj);

		float sw = i != WEAPON_NINJA ? sprite_size : sprite_size * 2.0f;

		gfx_quads_draw(header.x + spacing * 0.5f, header.y + sprite_size / 2.0f + header.h * 0.1f, sw, sprite_size);

		ui_vsplit_l(&header, spacing, 0, &header);
	}

	if (gameclient.snap.gameobj->flags&GAMEFLAG_FLAGS)
	{
		select_sprite(&data->sprites[SPRITE_FLAG_RED]);
		gfx_quads_draw(header.x + spacing * 0.5f, header.y + sprite_size / 2.0f + header.h * 0.1f, sprite_size * 0.5f, sprite_size);
		ui_vsplit_l(&header, ((header_width / 2) * 4.25f / 5) / (NUM_WEAPONS + 3), 0, &header);
	}

	gfx_quads_end();
    }

	main_view.y = y + 35.0f;
	main_view.h = h - 70.0f - 70.0f * 3 - 15.0f;

	float line_height = main_view.h / (float)MAX_CLIENTS;

	for (int team = 0; team <= 2; team++)
	{
		if (!(gameclient.snap.gameobj->flags&GAMEFLAG_TEAMS) && team == 1) continue;

		const NETOBJ_PLAYER_INFO *players[MAX_CLIENTS] = {0};
		int num_players = 0;
		for(int i = 0; i < snap_num_items(SNAP_CURRENT); i++)
		{
			SNAP_ITEM item;
			const void *data = snap_get_item(SNAP_CURRENT, i, &item);

			if(item.type == NETOBJTYPE_PLAYER_INFO)
			{
				const NETOBJ_PLAYER_INFO *info = (const NETOBJ_PLAYER_INFO *)data;
				if(info->team == (team == 2 ? -1 : team))
				{
					players[num_players] = info;
					num_players++;
				}
			}
		}

		if (team == 2 && num_players == 0) continue;

		// sort players
		for(int k = 0; k < num_players; k++) // ffs, bubblesort
		{
			for(int i = 0; i < num_players-k-1; i++)
			{
				if(players[i]->score < players[i+1]->score)
				{
					const NETOBJ_PLAYER_INFO *tmp = players[i];
					players[i] = players[i+1];
					players[i+1] = tmp;
				}
			}
		}

		if (gameclient.snap.gameobj->flags&GAMEFLAG_TEAMS)
		{
			if (team == 0 || team == 1)
			{
				RECT line = main_view;

				ui_vsplit_l(&line, (header_width / 2) / 5, 0, &line);

				if (config.gfx_custom_hud_colors)
				{
					if (team == 0)
						gfx_text_color(config.gfx_red_hud_color_r * 1.0f / 255.0f, config.gfx_red_hud_color_g * 1.0f / 255.0f, config.gfx_red_hud_color_b * 1.0f / 255.0f, 1.0f);
					else
						gfx_text_color(config.gfx_blue_hud_color_r * 1.0f / 255.0f, config.gfx_blue_hud_color_g * 1.0f / 255.0f, config.gfx_blue_hud_color_b * 1.0f / 255.0f, 1.0f);
				}

				if (team == 0)
					ui_do_label(&line, localize("Red team"), line_height * 0.8f * 2.0f, -1);
				else if (team == 1)
					ui_do_label(&line, localize("Blue team"), line_height * 0.8f * 2.0f, -1);

				line.x = x + w - 25.0f;
				line.w = 500.0f;
				line.x -= line.w;

				RECT line2 = line;

				char buf[128];
				int score = team ? gameclient.snap.gameobj->teamscore_blue : gameclient.snap.gameobj->teamscore_red;
				str_format(buf, sizeof(buf), "%d", score);

                if(config.cl_new_scoreboard_full)
				line2.x += abs(350.0f - gfx_text_width(0, line_height * 0.8f * 2.0f, buf, -1)) / 2.0f;
                else
			    line2.x += abs(w - gfx_text_width(0, line_height * 0.8f * 1.0f, buf, -1)) / 2.0f;
				line2.w = line.w - line2.x + line.x;
				ui_do_label(&line2, buf, line_height * 0.8f * 2.0f, -1);

				line.x -= line.w * 0.5f;

				line2 = line;
				str_format(buf, sizeof(buf), "%d", num_players);
                if(config.cl_new_scoreboard_full)
				line2.x += abs(350.0f - gfx_text_width(0, line_height * 0.8f * 1.0f, buf, -1)) / 2.0f;
                else
			    line2.x += abs(gfx_text_width(0, line_height * 0.8f * 1.0f, buf, -1)) / 2.0f;
				line2.w = line.w - line2.x + line.x;
				line2.y += line_height * 0.5f;
				line2.h = line.h - line2.y + line.y;
				ui_do_label(&line2, buf, line_height * 0.8f * 1.00f, -1);

				ui_hsplit_t(&main_view, line_height * 2.0f, 0, &main_view);
			}
		} else {
			if (team == 0)
			{
				RECT line = main_view;
				ui_vsplit_l(&line, (header_width / 2) / 5, 0, &line);

				if (config.gfx_custom_hud_colors)
				{
					gfx_text_color(config.gfx_player_hud_color_r * 1.0f / 255.0f, config.gfx_player_hud_color_g * 1.0f / 255.0f, config.gfx_player_hud_color_b * 1.0f / 255.0f, 1.0f);
				}

				ui_do_label(&line, localize("Players"), line_height * 0.8f * 2.0f, -1);

				line.x = x + w - 25.0f;
				line.w = 500.0f;
				line.x -= line.w * 1.5f;

				RECT line2 = line;

				line2 = line;
				char buf[128];
				str_format(buf, sizeof(buf), "%d", num_players);
				line2.x += abs(400.0f - gfx_text_width(0, line_height * 0.8f * 1.0f, buf, -1)) / 2.0f;
				line2.w = line.w - line2.x + line.x;
				line2.y += line_height * 0.5f;
				line2.h = line.h - line2.y + line.y;
				ui_do_label(&line2, buf, line_height * 0.8f * 1.00f, -1);

				ui_hsplit_t(&main_view, line_height * 2.0f, 0, &main_view);
			}
		}

		if (team == 2)
		{
			main_view.y = y + h - 35.0f - 15.0f;
			main_view.h = line_height * 2.0f + line_height * num_players;
			main_view.y -= main_view.h;

			RECT line = main_view;
			ui_vsplit_l(&line, (header_width / 2) / 5, 0, &line);

			if (config.gfx_custom_hud_colors)
			{
				gfx_text_color(config.gfx_spectator_hud_color_r * 1.0f / 255.0f, config.gfx_spectator_hud_color_g * 1.0f / 255.0f, config.gfx_spectator_hud_color_b * 1.0f / 255.0f, 1.0f);
			}

			ui_do_label(&line, localize("Spectators"), line_height * 0.8f * 2.0f, -1);

			line.x = x + w - 25.0f;
			line.w = 500.0f;
			line.x -= line.w * 1.5f;

			RECT line2 = line;

			line2 = line;
			char buf[128];
			str_format(buf, sizeof(buf), "%d", num_players);
            if(config.cl_new_scoreboard_full)
			line2.x += abs(400.0f - gfx_text_width(0, line_height * 0.8f * 1.0f, buf, -1)) / 2.0f;
            else
			line2.x += abs(gfx_text_width(0, line_height * 0.8f * 1.0f, buf, -1)) / 2.0f;
			line2.w = line.w - line2.x + line.x;
			line2.y += line_height * 0.5f;
			line2.h = line.h - line2.y + line.y;
			ui_do_label(&line2, buf, line_height * 0.8f * 1.00f, -1);

			ui_hsplit_t(&main_view, line_height * 2.0f, 0, &main_view);
		}

		for (int i = 0; i < num_players; i++)
		{
			char buf[128];

			const NETOBJ_PLAYER_INFO *info = players[i];

			RECT line = main_view;
			line.h = line_height;

			if (info->local)
				ui_draw_rect(&line, vec4(1.0f, 1.0f, 1.0f, 0.25f), CORNER_ALL, 5.0f);

			TEE_RENDER_INFO teeinfo = gameclient.clients[info->cid].render_info;
			teeinfo.size *= line_height / 64.0f;

			if((gameclient.snap.flags[0] && gameclient.snap.flags[0]->carried_by == info->cid) ||
				(gameclient.snap.flags[1] && gameclient.snap.flags[1]->carried_by == info->cid))
			{
				gfx_blend_normal();
				gfx_texture_set(data->images[IMAGE_GAME].id);
				gfx_quads_begin();

				if(info->team == 0) select_sprite(SPRITE_FLAG_BLUE, SPRITE_FLAG_FLIP_X);
				else select_sprite(SPRITE_FLAG_RED, SPRITE_FLAG_FLIP_X);

				float size = line_height;

				gfx_quads_drawTL(line.x + abs(75.0f - teeinfo.size) / 2.0f - 35.0f * (line_height / 64.0f), line.y + teeinfo.size / 2.0f - 40.0f * (line_height / 64.0f), size/2, size);
				gfx_quads_end();
			}

			render_tee(ANIMSTATE::get_idle(), &teeinfo, EMOTE_NORMAL, vec2(1,0), vec2(line.x + abs(75.0f - teeinfo.size) / 2.0f, line.y + teeinfo.size / 2.0f));

			ui_vsplit_l(&line, 50.0f, 0, &line);
			ui_do_label(&line, gameclient.clients[info->cid].name, line_height * 0.8f, -1);
            char buf2[128];
            if(config.cl_new_scoreboard_id)
            {
            str_format(buf2,sizeof(buf2),"id: %d",info->cid);
			ui_vsplit_l(&line, -120.0f, 0, &line);
            ui_do_label(&line, buf2 , line_height * 0.8, -1);
			ui_vsplit_l(&line, 120.0f, 0, &line);
            }
			ui_vsplit_l(&line, 350.0f, 0, &line);

			if (team != 2)
			{
				RECT line_t = line;
				str_format(buf, sizeof(buf), "%d", info->score);
				line_t.x += abs(125.0f - gfx_text_width(0, line_height * 0.8f, buf, -1)) / 2.0f;
				ui_do_label(&line_t, buf, line_height * 0.8f, -1);
			}

			ui_vsplit_l(&line, 125.0f, 0, &line);

			{
				RECT line_t = line;
				str_format(buf, sizeof(buf), "%d", info->latency);
				line_t.x += abs(75.0f - gfx_text_width(0, line_height * 0.8f, buf, -1)) / 2.0f;
				ui_do_label(&line_t, buf, line_height * 0.8f, -1);
			}
        if(config.cl_new_scoreboard_full)
        {
			if (team != 2)
			{
				ui_vsplit_l(&line, 75.0f, 0, &line);

				{
					RECT line_t = line;
					if (gameclient.clients[info->cid].stats.total_kills - gameclient.clients[info->cid].stats.total_killed != 0 || gameclient.clients[info->cid].stats.total_killed != 0)
						str_format(buf, sizeof(buf), "%d/%.1f", gameclient.clients[info->cid].stats.total_kills - gameclient.clients[info->cid].stats.total_killed, gameclient.clients[info->cid].stats.total_killed == 0 ? 0.0f : (float)((float)gameclient.clients[info->cid].stats.total_kills / (float)gameclient.clients[info->cid].stats.total_killed));
					else
						str_format(buf, sizeof(buf),"---");
					line_t.x += abs(spacing - gfx_text_width(0, line_height * 0.8f, buf, -1)) / 2.0f;
					ui_do_label(&line_t, buf, line_height * 0.8f, -1);
				}

				ui_vsplit_l(&line, spacing, 0, &line);
				{
					RECT line_t = line;
					if (gameclient.clients[info->cid].stats.total_kills != 0 || gameclient.clients[info->cid].stats.total_killed != 0)
						str_format(buf, sizeof(buf), "%d/%d", gameclient.clients[info->cid].stats.total_kills, gameclient.clients[info->cid].stats.total_killed);
					else
						str_format(buf, sizeof(buf), "---");
					line_t.x += abs(spacing - gfx_text_width(0, line_height * 0.8f, buf, -1)) / 2.0f;
					ui_do_label(&line_t, buf, line_height * 0.8f, -1);
				}

				ui_vsplit_l(&line, spacing, 0, &line);

				for (int i = 0; i < NUM_WEAPONS; i++)
				{
					RECT line_t = line;
					if (gameclient.clients[info->cid].stats.kills[i] != 0 || gameclient.clients[info->cid].stats.killed[i] != 0)
						str_format(buf, sizeof(buf), "%d/%d", gameclient.clients[info->cid].stats.kills[i], gameclient.clients[info->cid].stats.killed[i]);
					else
						str_format(buf, sizeof(buf), "---");
					line_t.x += abs(spacing - gfx_text_width(0, line_height * 0.8f, buf, -1)) / 2.0f;
					ui_do_label(&line_t, buf, line_height * 0.8f, -1);

					ui_vsplit_l(&line, spacing, 0, &line);
				}

				if (gameclient.snap.gameobj->flags&GAMEFLAG_FLAGS)
				{
					RECT line_t = line;
					if (gameclient.clients[info->cid].stats.flag_carried != 0 || gameclient.clients[info->cid].stats.flag_lost != 0)
						str_format(buf, sizeof(buf), "%d/%d", gameclient.clients[info->cid].stats.flag_carried, gameclient.clients[info->cid].stats.flag_lost);
					else
						str_format(buf, sizeof(buf), "---");
					line_t.x += abs(spacing - gfx_text_width(0, line_height * 0.8f, buf, -1)) / 2.0f;
					ui_do_label(&line_t, buf, line_height * 0.8f, -1);
				}
			}
        }
			ui_hsplit_t(&main_view, line_height, 0, &main_view);
		}
	}

	gfx_text_color(1, 1, 1, 1);
}

void SCOREBOARD::on_render()
{
	bool do_scoreboard = false;

	// if we activly wanna look on the scoreboard
	if(active)
		do_scoreboard = true;

	if(gameclient.snap.local_info && gameclient.snap.local_info->team != -1)
	{
		// we are not a spectator, check if we are ead
		if(!gameclient.snap.local_character || gameclient.snap.local_character->health < 0)
			do_scoreboard = true;
	}

	// if we the game is over
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->game_over)
		do_scoreboard = true;

	if(!do_scoreboard)
		return;

	// if the score board is active, then we should clear the motd message aswell
	if(active)
		gameclient.motd->clear();

	if (config.cl_new_scoreboard)
	{
		render_new();
		return;
	}

	float width = 400*3.0f*gfx_screenaspect();
	float height = 400*3.0f;

	gfx_mapscreen(0, 0, width, height);

	float w = 650.0f;

	if(gameclient.snap.gameobj && !(gameclient.snap.gameobj->flags&GAMEFLAG_TEAMS))
	{
		render_scoreboard(width/2-w/2, 150.0f, w, 0, 0);
		//render_scoreboard(gameobj, 0, 0, -1, 0);
	}
	else
	{

		if(gameclient.snap.gameobj && gameclient.snap.gameobj->game_over)
		{
			const char *text = localize("Draw");
			if(gameclient.snap.gameobj->teamscore_red > gameclient.snap.gameobj->teamscore_blue)
				text = localize("Red team wins");
			else if(gameclient.snap.gameobj->teamscore_blue > gameclient.snap.gameobj->teamscore_red)
				text = localize("Blue team wins");

			float w = gfx_text_width(0, 92.0f, text, -1);
			gfx_text(0, width/2-w/2, 45, 92.0f, text, -1);
		}

		render_scoreboard(width/2-w-20, 150.0f, w, 0, localize("Red team"));
		render_scoreboard(width/2 + 20, 150.0f, w, 1, localize("Blue team"));
	}

	render_goals(width/2-w/2, 150+750+25, w);
	render_spectators(width/2-w/2, 150+750+25+50+25, w);
}
