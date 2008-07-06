/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */

const char *engine_savepath(const char *filename, char *buffer, int max);
void engine_init(const char *appname);
void engine_parse_arguments(int argc, char **argv);

int engine_config_write_start();
void engine_config_write_line(const char *line);
void engine_config_write_stop();

int engine_stress(float probability);


enum
{
	MAX_MASTERSERVERS=16
};

void mastersrv_default();
int mastersrv_load();
int mastersrv_save();

int mastersrv_refresh_addresses();
void mastersrv_update();
int mastersrv_refreshing();
void mastersrv_dump_servers();
NETADDR mastersrv_get(int index);
const char *mastersrv_name(int index);
