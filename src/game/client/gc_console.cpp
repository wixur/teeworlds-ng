#include "gc_console.h"
#include "../generated/gc_data.h"

extern "C" {
	#include <engine/e_system.h>
	#include <engine/e_client_interface.h>
	#include <engine/e_config.h>
	#include <engine/e_console.h>
	#include <engine/e_ringbuffer.h>
	#include <engine/client/ec_font.h>
}

#include <cstring>
#include <cstdio>

#include "gc_ui.h"
#include "gc_client.h"

#include "../g_version.h"

enum
{
	CONSOLE_CLOSED,
	CONSOLE_OPENING,
	CONSOLE_OPEN,
	CONSOLE_CLOSING,
};

static char console_history_data[65536];
static RINGBUFFER *console_history;

static char console_backlog_data[65536];
static RINGBUFFER *console_backlog;

static unsigned int console_input_len = 0;
static char console_input[256] = {0};
static int console_state = CONSOLE_CLOSED;
static float state_change_end = 0.0f;
static const float state_change_duration = 0.1f;

static float time_now()
{
	static long long time_start = time_get();
	return float(time_get()-time_start)/float(time_freq());
}

static void client_console_print(const char *str)
{
	int len = strlen(str);

	if (len > 255)
		len = 255;

	char *entry = (char *)ringbuf_allocate(console_backlog, len+1);
	memcpy(entry, str, len+1);
}


static void con_team(void *result, void *user_data)
{
	int new_team;
	console_result_int(result, 1, &new_team);
	send_switch_team(new_team);
}

static void command_history(void *result, void *user_data)
{
	char *entry = (char *)ringbuf_first(console_history);

	while (entry)
	{
		dbg_msg("console/history", entry);

		entry = (char *)ringbuf_next(console_history, entry);
	}
}

void send_kill(int client_id);

static void command_kill(void *result, void *user_data)
{
	send_kill(-1);
}

void client_console_init()
{
	console_history = ringbuf_init(console_history_data, sizeof(console_history_data));
	console_backlog = ringbuf_init(console_backlog_data, sizeof(console_backlog_data));

	console_register_print_callback(client_console_print);
	MACRO_REGISTER_COMMAND("team", "i", con_team, 0x0);
	MACRO_REGISTER_COMMAND("history", "", command_history, 0x0);
	MACRO_REGISTER_COMMAND("kill", "", command_kill, 0x0);
}

static char *console_history_entry = 0x0;

void console_handle_input()
{
	int was_active = console_active();

	for(int i = 0; i < inp_num_events(); i++)
	{
		INPUT_EVENT e = inp_get_event(i);

		if (e.key == KEY_F3)
		{
			console_toggle();
		}

		if (console_active())
		{
			if (!(e.ch >= 0 && e.ch < 32))
			{
				if (console_input_len < sizeof(console_input) - 1)
				{
					console_input[console_input_len] = e.ch;
					console_input[console_input_len+1] = 0;
					console_input_len++;

					console_history_entry = 0x0;
				}
			}

			if(e.key == KEY_BACKSPACE)
			{
				if(console_input_len > 0)
				{
					console_input[console_input_len-1] = 0;
					console_input_len--;

					console_history_entry = 0x0;
				}
			}
			else if(e.key == KEY_ENTER || e.key == KEY_KP_ENTER)
			{
				if (console_input_len)
				{
					char *entry = (char *)ringbuf_allocate(console_history, console_input_len+1);
					memcpy(entry, console_input, console_input_len+1);
					
					console_execute(console_input);
					console_input[0] = 0;
					console_input_len = 0;

					console_history_entry = 0x0;
				}
			}
			else if (e.key == KEY_UP)
			{
				if (console_history_entry)
				{
					char *test = (char *)ringbuf_prev(console_history, console_history_entry);

					if (test)
						console_history_entry = test;
				}
				else
					console_history_entry = (char *)ringbuf_last(console_history);

				if (console_history_entry)
				{
					unsigned int len = strlen(console_history_entry);
					if (len < sizeof(console_input) - 1)
					{
						memcpy(console_input, console_history_entry, len+1);

						console_input_len = len;
					}
				}

			}
			else if (e.key == KEY_DOWN)
			{
				if (console_history_entry)
					console_history_entry = (char *)ringbuf_next(console_history, console_history_entry);

				if (console_history_entry)
				{
					unsigned int len = strlen(console_history_entry);
					if (len < sizeof(console_input) - 1)
					{
						memcpy(console_input, console_history_entry, len+1);

						console_input_len = len;
					}
				}
				else
				{
					console_input[0] = 0;
					console_input_len = 0;
				}
			}
		}
	}

	if (was_active || console_active())
	{
		inp_clear_events();
		inp_clear_key_states();
	}
}

void console_toggle()
{
	if (console_state == CONSOLE_CLOSED || console_state == CONSOLE_OPEN)
	{
		state_change_end = time_now()+state_change_duration;
	}
	else
	{
		float progress = state_change_end-time_now();
		float reversed_progress = state_change_duration-progress;

		state_change_end = time_now()+reversed_progress;
	}

	if (console_state == CONSOLE_CLOSED || console_state == CONSOLE_CLOSING)
		console_state = CONSOLE_OPENING;
	else
		console_state = CONSOLE_CLOSING;
}

// only defined for 0<=t<=1
static float console_scale_func(float t)
{
	//return t;
	return sinf(acosf(1.0f-t));
}

void console_render()
{
    RECT screen = *ui_screen();
	float console_max_height = screen.h*3/5.0f;
	float console_height;

	float progress = (time_now()-(state_change_end-state_change_duration))/float(state_change_duration);

	if (progress >= 1.0f)
	{
		if (console_state == CONSOLE_CLOSING)
			console_state = CONSOLE_CLOSED;
		else if (console_state == CONSOLE_OPENING)
			console_state = CONSOLE_OPEN;

		progress = 1.0f;
	}
	
	if (console_state == CONSOLE_CLOSED)
		return;

	float console_height_scale;

	if (console_state == CONSOLE_OPENING)
		console_height_scale = console_scale_func(progress);
	else if (console_state == CONSOLE_CLOSING)
		console_height_scale = console_scale_func(1.0f-progress);
	else //if (console_state == CONSOLE_OPEN)
		console_height_scale = console_scale_func(1.0f);

	console_height = console_height_scale*console_max_height;

	gfx_mapscreen(screen.x, screen.y, screen.w, screen.h);


	gfx_texture_set(data->images[IMAGE_CONSOLE_BG].id);
    gfx_quads_begin();
    gfx_setcolor(0.2f, 0.2f, 0.2f,0.9f);
    gfx_quads_setsubset(0,-console_height*0.075f,screen.w*0.075f*0.5f,0);
    gfx_quads_drawTL(0,0,screen.w,console_height);
    gfx_quads_end();

	gfx_texture_set(data->images[IMAGE_CONSOLE_BAR].id);
    gfx_quads_begin();
    gfx_setcolor(1.0f, 1.0f, 1.0f, 0.9f);
    gfx_quads_setsubset(0,0.1f,screen.w*0.015f,1-0.1f);
    gfx_quads_drawTL(0,console_height-10.0f,screen.w,10.0f);
    gfx_quads_end();
    
    console_height -= 10.0f;

	{
		float font_size = 12.0f;
		float row_height = font_size*1.4f;
		float width = gfx_text_width(0, font_size, console_input, -1);
		float x = 3, y = console_height - row_height - 2;
		float prompt_width = gfx_text_width(0, font_size, ">", -1)+2;

		gfx_text(0, x, y, font_size, ">", -1);
		gfx_text(0, x+prompt_width, y, font_size, console_input, -1);
		gfx_text(0, x+prompt_width+width+1, y, font_size, "_", -1);

		char buf[64];
		sprintf(buf, "Teewars v%s", TEEWARS_VERSION);
		float version_width = gfx_text_width(0, font_size, buf, -1);
		gfx_text(0, screen.w-version_width-5, y, font_size, buf, -1);

		y -= row_height;

		char *entry = (char *)ringbuf_last(console_backlog);
		while (y > 0.0f && entry)
		{
			gfx_text(0, x, y, font_size, entry, -1);
			y -= row_height;

			entry = (char *)ringbuf_prev(console_backlog, entry);
		}
	}
}

int console_active()
{
	return console_state != CONSOLE_CLOSED;
}

