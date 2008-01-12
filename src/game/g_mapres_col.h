/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#ifndef GAME_MAPRES_COL_H
#define GAME_MAPRES_COL_H


#include <game/g_vmath.h>

struct mapres_collision
{
	int width;
	int height;
	int data_index;
};

int col_init(int dividor);
int col_check_point(int x, int y);
int col_width();
int col_height();
bool col_intersect_line(vec2 pos0, vec2 pos1, vec2 *out);

#endif
