#include <stdio.h>
#include <math.h>
#include <string.h>

#include <baselib/system.h>
#include <baselib/keys.h>
#include <baselib/mouse.h>
#include <baselib/network.h>

#include <engine/interface.h>
#include <engine/versions.h>
#include "../mapres.h"

#include <engine/client/ui.h>
#include "mapres_image.h"
#include "mapres_tilemap.h"

using namespace baselib;

/********************************************************
 MENU                                                  
*********************************************************/

enum gui_tileset_enum
{
	tileset_regular,
	tileset_hot,
	tileset_active,
	tileset_inactive
};

enum gui_parts_enum
{
	invalid_part = 0,
    button_big_topleft, 
    button_big_topmid, 
    button_big_topright, 
    button_big_midleft, 
    button_big_midmid, 
    button_big_midright, 
    button_big_btmleft, 
    button_big_btmmid, 
    button_big_btmright, 
    slider_big_horiz_begin, 
    slider_big_horiz_mid, 
    slider_big_horiz_end, 
    slider_small_horiz_begin, 
    slider_small_horiz_mid, 
    slider_small_horiz_end, 
    slider_small_vert_begin, 
    radio_unchecked, 
    radio_checked, 
    slider_small_vert_mid, 
    slider_small_vert_end, 
    slider_big_vert_begin, 
    slider_big_handle_vert, 
    slider_big_handle_horiz, 
    slider_small_handle_horiz, 
    slider_big_arrow_left, 
    slider_big_arrow_up, 
    slider_big_arrow_right, 
    slider_big_arrow_down, 
    slider_small_arrow_left, 
    slider_small_arrow_up, 
    slider_small_arrow_right, 
    slider_small_arrow_down, 
    slider_small_handle_vert, 
    slider_big_vert_mid, 
    slider_big_vert_end, 
    screen_thick_topleft, 
    screen_thick_topmid, 
    screen_thick_topright, 
    screen_thick_midleft, 
    screen_thick_midmid, 
    screen_thick_midright, 
    screen_thick_btmleft, 
    screen_thick_btmmid, 
    screen_thick_btmright, 
    screen_transparent_topleft, 
    screen_transparent_topmid, 
    screen_transparent_topright, 
    screen_transparent_midleft, 
    screen_transparent_midmid, 
    screen_transparent_midright, 
    screen_transparent_btmleft, 
    screen_transparent_btmmid, 
    screen_transparent_btmright, 
    screen_info_topleft, 
    screen_info_topmid, 
    screen_info_topright, 
    screen_info_midleft, 
    screen_info_midmid, 
    screen_info_midright, 
    screen_info_btmleft, 
    screen_info_btmmid, 
    screen_info_btmright, 
    screen_textbox_topleft, 
    screen_textbox_topmid, 
    screen_textbox_topright, 
    screen_textbox_midleft, 
    screen_textbox_midmid, 
    screen_textbox_midright, 
    screen_textbox_btmleft, 
    screen_textbox_btmmid, 
    screen_textbox_btmright, 
    screen_list_topleft, 
    screen_list_topmid, 
    screen_list_topright, 
    screen_list_midleft, 
    screen_list_midmid, 
    screen_list_midright, 
    screen_list_btmleft, 
    screen_list_btmmid, 
    screen_list_btmright
};

struct gui_part
{
	int x, y;
	int w, h;
	bool stretch_x, stretch_y;
};

gui_part parts[] =
{
	{ 0, 0, 0, 0 }, // invalid_part
{ 0, 0, 16, 16 }, // button_big_topleft 
{ 16, 0, 16, 16 }, // button_big_topmid 
{ 32, 0, 16, 16 }, // button_big_topright 
{ 0, 16, 16, 16 }, // button_big_midleft 
{ 16, 16, 16, 16 }, // button_big_midmid 
{ 32, 16, 16, 16 }, // button_big_midright 
{ 0, 32, 16, 16 }, // button_big_btmleft 
{ 16, 32, 16, 16 }, // button_big_btmmid 
{ 32, 32, 16, 16 }, // button_big_btmright 
{ 0, 48, 16, 16 }, // slider_big_horiz_begin 
{ 16, 48, 16, 16 }, // slider_big_horiz_mid 
{ 32, 48, 16, 16 }, // slider_big_horiz_end 
{ 0, 96, 16, 16 }, // slider_small_horiz_begin 
{ 16, 96, 16, 16 }, // slider_small_horiz_mid 
{ 32, 96, 16, 16 }, // slider_small_horiz_end 
{ 48, 96, 16, 16 }, // slider_small_vert_begin 
{ 64, 112, 32, 32 }, // radio_unchecked 
{ 96, 112, 32, 32 }, // radio_checked 
{ 48, 112, 16, 16 }, // slider_small_vert_mid 
{ 48, 128, 16, 16 }, // slider_small_vert_end 
{ 48, 48, 16, 16 }, // slider_big_vert_begin 
{ 64, 48, 16, 32 }, // slider_big_handle_vert 
{ 80, 64, 32, 16 }, // slider_big_handle_horiz 
{ 80, 48, 16, 16 }, // slider_small_handle_horiz 
{ 0, 64, 16, 16 }, // slider_big_arrow_left 
{ 16, 64, 16, 16 }, // slider_big_arrow_up 
{ 32, 64, 16, 16 }, // slider_big_arrow_right 
{ 16, 80, 16, 16 }, // slider_big_arrow_down 
{ 0, 112, 16, 16 }, // slider_small_arrow_left 
{ 16, 112, 16, 16 }, // slider_small_arrow_up 
{ 32, 112, 16, 16 }, // slider_small_arrow_right 
{ 16, 128, 16, 16 }, // slider_small_arrow_down 
{ 96, 48, 16, 16 }, // slider_small_handle_vert 
{ 48, 64, 16, 16 }, // slider_big_vert_mid 
{ 48, 80, 16, 16 }, // slider_big_vert_end 
{ 0, 384, 16, 16 }, // screen_thick_topleft 
{ 16, 384, 16, 16 }, // screen_thick_topmid 
{ 32, 384, 16, 16 }, // screen_thick_topright 
{ 0, 400, 16, 16 }, // screen_thick_midleft 
{ 16, 400, 16, 16 }, // screen_thick_midmid 
{ 32, 400, 16, 16 }, // screen_thick_midright 
{ 0, 416, 16, 16 }, // screen_thick_btmleft 
{ 16, 416, 16, 16 }, // screen_thick_btmmid 
{ 32, 416, 16, 16 }, // screen_thick_btmright 
{ 48, 384, 16, 16 }, // screen_transparent_topleft 
{ 64, 384, 16, 16 }, // screen_transparent_topmid 
{ 80, 384, 16, 16 }, // screen_transparent_topright 
{ 48, 400, 16, 16 }, // screen_transparent_midleft 
{ 64, 400, 16, 16 }, // screen_transparent_midmid 
{ 80, 400, 16, 16 }, // screen_transparent_midright 
{ 48, 416, 16, 16 }, // screen_transparent_btmleft 
{ 64, 416, 16, 16 }, // screen_transparent_btmmid 
{ 80, 416, 16, 16 }, // screen_transparent_btmright 
{ 96, 384, 16, 16 }, // screen_info_topleft 
{ 112, 384, 16, 16 }, // screen_info_topmid 
{ 128, 384, 16, 16 }, // screen_info_topright 
{ 96, 400, 16, 16 }, // screen_info_midleft 
{ 112, 400, 16, 16 }, // screen_info_midmid 
{ 128, 400, 16, 16 }, // screen_info_midright 
{ 96, 416, 16, 16 }, // screen_info_btmleft 
{ 112, 416, 16, 16 }, // screen_info_btmmid 
{ 128, 416, 16, 16 }, // screen_info_btmright 
{ 144, 384, 16, 16 }, // screen_textbox_topleft 
{ 160, 384, 16, 16 }, // screen_textbox_topmid 
{ 176, 384, 16, 16 }, // screen_textbox_topright 
{ 144, 400, 16, 16 }, // screen_textbox_midleft 
{ 160, 400, 16, 16 }, // screen_textbox_midmid 
{ 176, 400, 16, 16 }, // screen_textbox_midright 
{ 144, 416, 16, 16 }, // screen_textbox_btmleft 
{ 160, 416, 16, 16 }, // screen_textbox_btmmid 
{ 176, 416, 16, 16 }, // screen_textbox_btmright 
{ 192, 384, 16, 16 }, // screen_list_topleft 
{ 208, 384, 16, 16 }, // screen_list_topmid 
{ 224, 384, 16, 16 }, // screen_list_topright 
{ 192, 400, 16, 16 }, // screen_list_midleft 
{ 208, 400, 16, 16 }, // screen_list_midmid 
{ 224, 400, 16, 16 }, // screen_list_midright 
{ 192, 416, 16, 16 }, // screen_list_btmleft 
{ 208, 416, 16, 16 }, // screen_list_btmmid 
{ 224, 416, 16, 16 } // screen_list_btmright
};

#define GUI_COMPOSITE_BOX(name) { name ## _topleft, name ## _topmid, name ## _topright, name ## _midleft, name ## _midmid, name ## _midright, name ## _btmleft, name ## _btmmid, name ## _btmright }

enum gui_composite_box_enum
{
	button_big_box,
	screen_thick_box,
	screen_transparent_box,
	screen_info_box,
	screen_textbox_box,
	screen_list_box,
};

struct gui_composite_box
{
	gui_parts_enum part_topleft, part_topmid, part_topright;
	gui_parts_enum part_midleft, part_midmid, part_midright;
	gui_parts_enum part_btmleft, part_btmmid, part_btmright;
};

gui_composite_box boxes[] =
{
	GUI_COMPOSITE_BOX(button_big),
	GUI_COMPOSITE_BOX(screen_thick),
	GUI_COMPOSITE_BOX(screen_transparent),
	GUI_COMPOSITE_BOX(screen_info),
	GUI_COMPOSITE_BOX(screen_textbox),
	GUI_COMPOSITE_BOX(screen_list),
};

int gui_tileset_texture;

void draw_part(gui_part part, gui_tileset_enum tileset, float x, float y, float w, float h)
{
	const float tex_w = 512.0, tex_h = 512.0;

	const float start_x = part.x/tex_w, start_y = part.y/tex_h, f_w = part.w/tex_w, f_h = part.h/tex_h;

	float ts_x, ts_y;

	switch (tileset)
	{
		case tileset_regular:
			ts_x = 0.0f; ts_y = 0.0f; break;
		case tileset_hot:
			ts_x = 0.375f; ts_y = 0.375f; break;
		case tileset_active:
			ts_x = 0.0f; ts_y = 0.375f; break;
		case tileset_inactive:
			ts_x = 0.375f; ts_y = 0.0f; break;
		default:
			dbg_msg("menu", "invalid tileset given to draw_part");
	}
	
    gfx_blend_normal();
    gfx_texture_set(gui_tileset_texture);
    gfx_quads_begin();
    gfx_quads_setcolor(1,1,1,1);
	gfx_quads_setsubset(
		ts_x+start_x, // startx
		ts_y+start_y, // starty
		ts_x+start_x+f_w, // endx
		ts_y+start_y+f_h); // endy								
    gfx_quads_drawTL(x,y,w,h);
    gfx_quads_end();
}


void draw_part(gui_parts_enum part, gui_tileset_enum tileset, float x, float y, float w, float h)
{
	draw_part(parts[part], tileset, x, y, w, h);
}

void draw_part(gui_parts_enum part, gui_tileset_enum tileset, float x, float y)
{
	draw_part(part, tileset, x, y, parts[part].w, parts[part].h);
}

void draw_box(gui_composite_box_enum box, gui_tileset_enum tileset, float x, float y, float w, float h)
{
	gui_composite_box b = boxes[box];

	gui_part topleft_p = parts[b.part_topleft];
	gui_part topmid_p = parts[b.part_topmid];
	gui_part topright_p = parts[b.part_topright];
	gui_part midleft_p = parts[b.part_midleft];
	gui_part btmleft_p = parts[b.part_btmleft];

	float scale = h / (topleft_p.h + midleft_p.h + btmleft_p.h);
	scale = 1.0;

	float topleft_w = scale * topleft_p.w;
	float topright_w = scale * topright_p.w;
	float topmid_w = w - topright_w - topleft_w;
	float topleft_h = scale * topleft_p.h;
	float btmleft_h = scale * btmleft_p.h;
	float midleft_h = h - topleft_h - btmleft_h;

	draw_part(b.part_topleft, tileset, x, y);
	draw_part(b.part_topmid, tileset, x + topleft_w, y, topmid_w, topleft_h);
	draw_part(b.part_topright, tileset, x + topleft_w + topmid_w, y);
	
	draw_part(b.part_midleft, tileset, x, y + topleft_h, topright_w, midleft_h);
	draw_part(b.part_midmid, tileset, x + topleft_w, y + topleft_h, topmid_w, midleft_h);
	draw_part(b.part_midright, tileset, x + topleft_w + topmid_w, y + topleft_h, topright_w, midleft_h);

	draw_part(b.part_btmleft, tileset, x, y + topleft_h + midleft_h);
	draw_part(b.part_btmmid, tileset, x + topleft_w, y + topleft_h + midleft_h, topmid_w, btmleft_h);
	draw_part(b.part_btmright, tileset, x + topleft_w + topmid_w, y + topleft_h + midleft_h);
}

struct pretty_font
{
    char m_CharStartTable[256];
    char m_CharEndTable[256];
    int font_texture;
};  

extern pretty_font *current_font;
void gfx_pretty_text(float x, float y, float size, const char *text);
float gfx_pretty_text_width(float size, const char *text);

void draw_scrolling_background(int id, float w, float h, float t)
{
	float tx = w/256.0f;
	float ty = h/256.0f;

	float start_x = fmod(t, 1.0f);
	float start_y = 1.0f - fmod(t*0.8f, 1.0f);

    gfx_blend_normal();
    gfx_texture_set(id);
    gfx_quads_begin();
    gfx_quads_setcolor(1,1,1,1);
	gfx_quads_setsubset(
		start_x, // startx
		start_y, // starty
		start_x+tx, // endx
		start_y+ty); // endy								
    gfx_quads_drawTL(0.0f,0.0f,w,h);
    gfx_quads_end();
}

int background_texture;
int not_empty_item_texture;
int empty_item_texture;
int active_item_texture;
int selected_item_texture;
int teewars_banner_texture;
int connect_localhost_texture;

int music_menu;
int music_menu_id = -1;

struct button_textures
{
	int *normal_texture;
	int *hot_texture;
	int *active_texture;
};

button_textures connect_localhost_button = { &connect_localhost_texture, &connect_localhost_texture, &connect_localhost_texture };
button_textures list_item_button = { &not_empty_item_texture, &active_item_texture, &active_item_texture };
button_textures selected_item_button = { &selected_item_texture, &selected_item_texture, &selected_item_texture };

void draw_menu_button(void *id, const char *text, int checked, float x, float y, float w, float h, void *extra)
{
	button_textures *tx = (button_textures *)extra;

    gfx_blend_normal();
	
	if (ui_active_item() == id && ui_hot_item() == id)
		gfx_texture_set(*tx->active_texture);
	else if (ui_hot_item() == id)
		gfx_texture_set(*tx->hot_texture);
	else
		gfx_texture_set(*tx->normal_texture);
	
    gfx_quads_begin();

    gfx_quads_setcolor(1,1,1,1);

    gfx_quads_drawTL(x,y,w,h);
    gfx_quads_end();

    gfx_texture_set(current_font->font_texture);
    gfx_pretty_text(x + 8.f, y - 7.f, 36.f, text);
}

void draw_image_button(void *id, const char *text, int checked, float x, float y, float w, float h, void *extra)
{
	ui_do_image(*(int *)id, x, y, w, h);
}

void draw_single_part_button(void *id, const char *text, int checked, float x, float y, float w, float h, void *extra)
{
	gui_tileset_enum tileset;

	if (ui_active_item() == id && ui_hot_item() == id)
		tileset = tileset_active;
	else if (ui_hot_item() == id)
		tileset = tileset_hot;
	else
		tileset = tileset_regular;

	draw_part((gui_parts_enum)(int)(int *)extra, tileset, x, y, w, h);
}

void draw_teewars_button(void *id, const char *text, int checked, float x, float y, float w, float h, void *extra)
{
	float text_width = gfx_pretty_text_width(46.f, text);
	gui_tileset_enum tileset;

	if (ui_active_item() == id && ui_hot_item() == id)
		tileset = tileset_active;
	else if (ui_hot_item() == id)
		tileset = tileset_hot;
	else
		tileset = tileset_regular;

	if ((int)(int *)extra == 1)
		tileset = tileset_inactive;

	draw_box(button_big_box, tileset, x, y, w, h);

	gfx_texture_set(current_font->font_texture);

	gfx_pretty_text(x + w/2 - text_width/2 , y, 46.f, text);
}

struct server_info
{
	int version;
    int players;
	int max_players;
	netaddr4 address;
	char name[129];
	char map[65];
};

struct server_list
{   
    server_info infos[10];
	int active_count, info_count;
	int scroll_index;
	int selected_index;
};

int ui_do_edit_box(void *id, float x, float y, float w, float h, char *str, int str_size)
{
    int inside = ui_mouse_inside(x, y, w, h);
	int r = 0;

	if(inside)
	{
		ui_set_hot_item(id);

		if(ui_mouse_button(0))
			ui_set_active_item(id);
	}

	if (ui_active_item() == id)
	{
		int c = keys::last_char();
		int k = keys::last_key();
		int len = strlen(str);
	
		if (c >= 32 && c < 128)
		{
			if (len < str_size - 1)
			{
				str[len] = c;
				str[len+1] = 0;
			}
		}

		if (k == keys::backspace)
		{
			if (len > 0)
				str[len-1] = 0;
		}
		r = 1;
	}

	draw_box(screen_textbox_box, tileset_regular, x, y, w, h);

	ui_do_label(x + 8.f, y - 7.f, str);

	if (ui_active_item() == id)
	{
		float w = gfx_pretty_text_width(36.0f, str);
		ui_do_label(x + 8.f + w, y - 7.f, "_");
	}

	return r;
}

int do_scroll_bar(void *id, float x, float y, float height, int steps, int last_index)
{
	int r = last_index;

	static int up_button;
	static int down_button;

    if (ui_do_button(&up_button, "", 0, x + 8, y, 16, 16, draw_single_part_button, (void *)slider_big_arrow_up))
	{
		if (r > 0)
			--r;
	}
    else if (ui_do_button(&down_button, "", 0, x + 8, y + height - 16, 16, 16, draw_single_part_button, (void *)slider_big_arrow_down))
	{
		if (r < steps)
			++r;
	}
	else if (steps > 0) // only if there's actually stuff to scroll through
	{
		int inside = ui_mouse_inside(x, y + 16, 16, height - 32);
        if (inside && (!ui_active_item() || ui_active_item() == id))
			ui_set_hot_item(id);

		if(ui_active_item() == id)
		{
			if (ui_mouse_button(0))
			{
				float pos = ui_mouse_y() - y - 16;
				float perc = pos / (height - 32);

				r = (int)((steps + 1) * perc);
				if (r < 0)
					r = 0;
				else if (r > steps)
					r = steps;
			}
			else
				ui_set_active_item(0);
		}
		else if (ui_hot_item() == id && ui_mouse_button(0))
			ui_set_active_item(id);
		else if (inside && (!ui_active_item() || ui_active_item() == id))
			ui_set_hot_item(id);
	}

	draw_part(slider_big_vert_begin, tileset_regular, x + 8, y + 16, 16, 16);
	draw_part(slider_big_vert_mid, tileset_regular, x + 8, y + 32, 16, height - 32 - 32);
	draw_part(slider_big_vert_end, tileset_regular, x + 8, y + height - 32, 16, 16);

	draw_part(slider_big_handle_horiz, tileset_regular, x, y + 16 + r * ((height - 64) / steps), 32, 16);

	return r;
}

int do_server_list(server_list *list, float x, float y, int visible_items)
{
	const float spacing = 3.f;
	const float item_height = 28;
	const float item_width = 728;
	const float real_width = item_width + 20;
	const float real_height = item_height * visible_items + spacing * (visible_items - 1);

	int r = -1;

	for (int i = 0; i < visible_items; i++)
	{
		int item_index = i + list->scroll_index;
		if (item_index >= list->active_count)
			ui_do_image(empty_item_texture, x, y + i * item_height + i * spacing, item_width, item_height);
		else
		{
			server_info *item = &list->infos[item_index];

			bool clicked = false;
			if (list->selected_index == item_index)
				clicked = ui_do_button(item, item->name, 0, x, y + i * item_height + i * spacing, item_width, item_height, draw_menu_button, &selected_item_button);
			else
				clicked = ui_do_button(item, item->name, 0, x, y + i * item_height + i * spacing, item_width, item_height, draw_menu_button, &list_item_button);

			char temp[64]; // plenty of extra room so we don't get sad :o
			sprintf(temp, "%i/%i", item->players, item->max_players);

			gfx_texture_set(current_font->font_texture);
			gfx_pretty_text(x + 600, y + i * item_height + i * spacing - 7.f, 36.f, temp);
            gfx_pretty_text(x + 360, y + i * item_height + i * spacing - 7.f, 36.f, item->map);

			if (clicked)
			{
				r = item_index;
				list->selected_index = item_index;
			}
		}
	}

	list->scroll_index = do_scroll_bar(&list->scroll_index, x + real_width - 16, y, real_height, list->active_count - visible_items, list->scroll_index);
	
	return r;
}

char *read_int(char *buffer, int *value)
{
    *value = buffer[0] << 24;
    *value |= buffer[1] << 16;
    *value |= buffer[2] << 8;
    *value |= buffer[3];

	return buffer + 4;
}

char *read_netaddr(char *buffer, netaddr4 *addr)
{
	addr->ip[0] = *buffer++;
	addr->ip[1] = *buffer++;
	addr->ip[2] = *buffer++;
	addr->ip[3] = *buffer++;

	int port;
	buffer = read_int(buffer, &port);

	addr->port = port;

	return buffer;
}

void refresh_list(server_list *list)
{
	netaddr4 addr;
	netaddr4 me(0, 0, 0, 0, 0);

	list->selected_index = -1;
	
	if (net_host_lookup(MASTER_SERVER_ADDRESS, MASTER_SERVER_PORT, &addr) == 0)
    {
        socket_tcp4 sock;
        sock.open(&me);

		//sock.set_non_blocking();

		// try and connect with a timeout of 1 second
        if (sock.connect_non_blocking(&addr))
        {
            char data[256];
            int total_received = 0;
            int received;

            int master_server_version = -1;
            int server_count = -1;

            // read header
            while (total_received < 12 && (received = sock.recv(data + total_received, 12 - total_received)) > 0)
                total_received += received;

            // see if we have the header
            if (total_received == 12)
            {
                int signature;
                read_int(data, &signature);
    
                // check signature
                if(signature == 'TWSL')
                {
                    read_int(data + 4, &master_server_version);
                    read_int(data + 8, &server_count);

                    // TODO: handle master server version O.o
                        
                    const int server_info_size = 212;

                    list->active_count = 0;
    
                    for (int i = 0; i < server_count; i++)
                    { 
                         total_received = 0;

                        // read data for a server
                        while (sock.is_connected() && total_received < server_info_size && (received = sock.recv(data + total_received, server_info_size - total_received)) > 0)
                            total_received += received;

                        // check if we got enough data
                        if (total_received == server_info_size)
                        {
                            char *d = data;

                            server_info *info = &list->infos[i];

                            d = read_int(d, &info->version);
                            d = read_netaddr(d, &info->address);

							//dbg_msg("menu/got_serverinfo", "IP: %i.%i.%i.%i:%i", (int)info->address.ip[0], (int)info->address.ip[1], (int)info->address.ip[2], (int)info->address.ip[3], info->address.port);

                            d = read_int(d, &info->players);
                            d = read_int(d, &info->max_players);
                            memcpy(info->name, d, 128);
                            d += 128;
							memcpy(info->map, d, 64);

                            // let's be safe.
                            info->name[128] = 0;
                            info->map[64] = 0;

                            ++list->active_count;
                        }
                        else
                            break;
                    }

                    if (list->scroll_index >= list->active_count)
                        list->scroll_index = list->active_count - 1;

                    if (list->scroll_index < 0)
                        list->scroll_index = 0;
                }
            }

            sock.close();
        }
	}
}

static int menu_render(netaddr4 *server_address, char *str, int max_len)
{
	// background color
	gfx_clear(89/255.f,122/255.f,0.0);

	// GUI coordsys
	gfx_mapscreen(0,0,800.0f,600.0f);

	static server_list list;
	static bool inited = false;

	if (!inited)
	{
		list.info_count = 256;

		list.scroll_index = 0;
		list.selected_index = -1;

		inited = true;

		refresh_list(&list);
	}

	static int64 start = time_get();

	float t = double(time_get() - start) / double(time_freq());
	draw_scrolling_background(background_texture, 800, 600, t * 0.01);

	ui_do_image(teewars_banner_texture, 140, 20, 512, 128);

	do_server_list(&list, 20, 160, 8);

	/*
    if (ui_do_button(&connect_localhost_button, "", 0, 15, 250, 64, 24, draw_menu_button, &connect_localhost_button))
    {
        *server_address = netaddr4(127, 0, 0, 1, 8303);
        return 1;
    }*/	

	static int refresh_button, join_button, quit_button;

	if (ui_do_button(&refresh_button, "Refresh", 0, 440, 420, 170, 48, draw_teewars_button))
	{
		refresh_list(&list);
	} 

	if (list.selected_index == -1)
	{
		ui_do_button(&join_button, "Join", 0, 620, 420, 128, 48, draw_teewars_button, (void *)1);
	}
	else if (ui_do_button(&join_button, "Join", 0, 620, 420, 128, 48, draw_teewars_button))
	{
		*server_address = list.infos[list.selected_index].address;

		dbg_msg("menu/join_button", "IP: %i.%i.%i.%i:%i", (int)server_address->ip[0], (int)server_address->ip[1], (int)server_address->ip[2], (int)server_address->ip[3], server_address->port);

		return 1;
	}

	const float name_x = 20, name_y = 430;

	ui_do_label(name_x + 8.f, name_y - 7.f, "Name:");
	//ui_do_image(input_box_texture, name_x + 50 - 5, name_y - 5, 150 + 10, 14 + 10);
	ui_do_edit_box(str, name_x + 100, name_y, 300, 28, str, max_len);

	if (ui_do_button(&quit_button, "Quit", 0, 620, 490, 128, 48, draw_teewars_button))
		return -1;

	ui_do_label(20.0f, 600.0f-40.0f, "Version: " TEEWARS_VERSION);
	
	return 0;
}

void modmenu_init()
{
	keys::enable_char_cache();
	keys::enable_key_cache();

    current_font->font_texture = gfx_load_texture("data/big_font.png");
	background_texture = gfx_load_texture("data/gui_bg.png");
    not_empty_item_texture = gfx_load_texture("data/gui/game_list_item_not_empty.png");
    empty_item_texture = gfx_load_texture("data/gui/game_list_item_empty.png");
    active_item_texture = gfx_load_texture("data/gui/game_list_item_active.png");
	selected_item_texture = gfx_load_texture("data/gui/game_list_item_selected.png");

	gui_tileset_texture = gfx_load_texture("data/gui/gui_widgets.png");

    teewars_banner_texture = gfx_load_texture("data/gui_logo.png");
	connect_localhost_texture = gfx_load_texture("data/gui/game_list_connect_localhost.png");

	music_menu = snd_load_wav("data/audio/Music_Menu.wav");
}

void modmenu_shutdown()
{
}

int modmenu_render(void *ptr, char *str, int max_len)
{
	static int mouse_x = 0;
	static int mouse_y = 0;

	if (music_menu_id == -1)
	{
		dbg_msg("menu", "no music is playing, so let's play some tunes!");
		music_menu_id = snd_play(music_menu, SND_LOOP);
	}

	netaddr4 *server_address = (netaddr4 *)ptr;	

    // handle mouse movement
    float mx, my, mwx, mwy;
    {
        int rx, ry;
        inp_mouse_relative(&rx, &ry);
        mouse_x += rx;
        mouse_y += ry;
        if(mouse_x < 0) mouse_x = 0;
        if(mouse_y < 0) mouse_y = 0;
        if(mouse_x > gfx_screenwidth()) mouse_x = gfx_screenwidth();
        if(mouse_y > gfx_screenheight()) mouse_y = gfx_screenheight();
            
        // update the ui
        mx = (mouse_x/(float)gfx_screenwidth())*800.0f;
        my = (mouse_y/(float)gfx_screenheight())*600.0f;
        mwx = mx*3.0f; // adjust to zoom and offset
        mwy = mx*3.0f; // adjust to zoom and offset
            
        int buttons = 0;
        if(inp_mouse_button_pressed(0)) buttons |= 1;
        if(inp_mouse_button_pressed(1)) buttons |= 2;
        if(inp_mouse_button_pressed(2)) buttons |= 4;
            
        ui_update(mx,my,mx*3.0f,my*3.0f,buttons);
    }

    int r = menu_render(server_address, str, max_len);

    // render butt ugly mouse cursor
    gfx_texture_set(-1);
    gfx_quads_begin();
    gfx_quads_setcolor(0,0,0,1);
    gfx_quads_draw_freeform(mx,my,mx,my,
                                mx+14,my,
                                mx,my+14);
    gfx_quads_setcolor(1,1,1,1);
    gfx_quads_draw_freeform(mx+1,my+1,mx+1,my+1,
                                mx+10,my+1,
                                mx+1,my+10);
    gfx_quads_end();

	if (r)
	{
		snd_stop(music_menu_id);
		music_menu_id = -1;
	}

	keys::clear_char();
	keys::clear_key();

	return r;
}