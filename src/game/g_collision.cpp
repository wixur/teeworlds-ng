/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#include <engine/e_system.h>
#include <game/g_vmath.h>
#include <math.h>
#include <engine/e_interface.h>
#include <game/g_math.h>
#include <game/g_mapitems.h>
#include <game/g_layers.h>

static TILE *tiles;
static int width = 0;
static int height = 0;

int col_width() { return width; }
int col_height() { return height; }

int col_init()
{
	width = layers_game()->width;
	height = layers_game()->height;
	tiles = (TILE *)map_get_data(layers_game()->data);
	return 1;
}


int col_is_solid(int x, int y)
{
	int nx = x/32;
	int ny = y/32;
	if(nx < 0 || nx >= width || ny >= height)
		return 1;
	
	if(y < 0)
		return 0; // up == sky == free
	
	return tiles[ny*width+nx].index == TILE_SOLID;
}

// TODO: rewrite this smarter!
bool col_intersect_line(vec2 pos0, vec2 pos1, vec2 *out)
{
	float d = distance(pos0, pos1);
	
	for(float f = 0; f < d; f++)
	{
		float a = f/d;
		vec2 pos = mix(pos0, pos1, a);
		if(col_is_solid((int)pos.x, (int)pos.y))
		{
			if(out)
				*out = pos;
			return true;
		}
	}
	if(out)
		*out = pos1;
	return false;
}

/*
	Simple collision rutines!
*/
/*
struct collision
{
	int w, h;
	unsigned char *data;
};

static collision col;
static int global_dividor;

int col_width()
{
	return col.w;
}

int col_height()
{
	return col.h;	
}

int col_init(int dividor)
{
	mapres_collision *c = (mapres_collision*)map_find_item(MAPRES_COLLISIONMAP,0);
	if(!c)
	{
		dbg_msg("mapres_col", "failed!");
		return 0;
	}
	col.w = c->width;
	col.h = c->height;
	global_dividor = dividor;
	col.data = (unsigned char *)map_get_data(c->data_index);
	return col.data ? 1 : 0;
}

*/
