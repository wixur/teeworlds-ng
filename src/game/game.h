#include <engine/system.h>
#include <game/math.h>
#include <math.h>
#include "../engine/interface.h"
#include "mapres_col.h"

#include "game_protocol.h"

inline vec2 get_direction(int angle)
{
	float a = angle/256.0f;
	return vec2(cosf(a), sinf(a));
}

inline vec2 get_dir(float a)
{
	return vec2(cosf(a), sinf(a));
}

inline float get_angle(vec2 dir)
{
	float a = atan(dir.y/dir.x);
	if(dir.x < 0)
		a = a+pi;
	return a;
}


template<typename T>
inline T saturated_add(T min, T max, T current, T modifier)
{
	if(modifier < 0)
	{
		if(current < min)
			return current;
		current += modifier;
		if(current < min)
			current = min;
		return current;
	}
	else
	{
		if(current > max)
			return current;
		current += modifier;
		if(current > max)
			current = max;
		return current;
	}
}

void move_point(vec2 *inout_pos, vec2 *inout_vel, float elasticity, int *bounces);
void move_box(vec2 *inout_pos, vec2 *inout_vel, vec2 size, float elasticity);


// hooking stuff
enum
{
	HOOK_RETRACTED=-1,
	HOOK_IDLE=0,
	HOOK_FLYING,
	HOOK_GRABBED
};

class world_core
{
public:
	world_core()
	{
		mem_zero(players, sizeof(players));
	}
		
	class player_core *players[MAX_CLIENTS];
};

class player_core
{
public:
	world_core *world;
	
	vec2 pos;
	vec2 vel;
	
	vec2 hook_pos;
	vec2 hook_dir;
	int hook_tick;
	int hook_state;
	int hooked_player;
	
	int jumped;
	player_input input;
	
	void tick();
	void move();
	
	void read(const obj_player_core *obj_core);
	void write(obj_player_core *obj_core);
	void quantize();
};


#define LERP(a,b,t) (a + (b-a) * t)
#define min(a, b) ( a > b ? b : a)
#define max(a, b) ( a > b ? a : b)

inline bool col_check_point(float x, float y) { return col_check_point((int)x, (int)y) != 0; }
inline bool col_check_point(vec2 p) { return col_check_point(p.x, p.y); }

struct mapres_entity
{
	int x, y;
	int data[];
};

struct mapres_spawnpoint
{
	int x, y;
};

struct mapres_item
{
	int x, y;
	int type;
};

struct mapres_flagstand
{
	int x, y;
};

enum
{
	MAPRES_ENTS_START=1,
	MAPRES_SPAWNPOINT=1,
	MAPRES_ITEM=2,
	MAPRES_SPAWNPOINT_RED=3,
	MAPRES_SPAWNPOINT_BLUE=4,
	MAPRES_FLAGSTAND_RED=5,
	MAPRES_FLAGSTAND_BLUE=6,
	MAPRES_ENTS_END,
	
	ITEM_NULL=0,
	ITEM_WEAPON_GUN=0x00010001,
	ITEM_WEAPON_SHOTGUN=0x00010002,
	ITEM_WEAPON_ROCKET=0x00010003,
	ITEM_WEAPON_SNIPER=0x00010004,
	ITEM_WEAPON_HAMMER=0x00010005,
	ITEM_HEALTH =0x00020001,
	ITEM_ARMOR=0x00030001,
	ITEM_NINJA=0x00040001,
};
