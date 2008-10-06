/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <base/system.h>

#include <engine/e_server_interface.h>
/*#include <engine/e_client_interface.h>*/
#include <engine/e_config.h>
#include <engine/e_console.h>
#include <engine/e_engine.h>
#include <engine/e_network.h>
#include "e_linereader.h"

/* compiled-in data-dir path */
#define DATA_DIR "data"

static JOBPOOL hostlookuppool;

static void con_dbg_dumpmem(void *result, void *user_data)
{
	mem_debug_dump();
}


static char application_save_path[512] = {0};
static char *datadir_override = 0;

const char *engine_savepath(const char *filename, char *buffer, int max)
{
	str_format(buffer, max, "%s/%s", application_save_path, filename);
	return buffer;
}

void engine_init(const char *appname)
{
	dbg_logger_stdout();
	dbg_logger_debugger();
	
	/* */
	dbg_msg("engine", "running on %s-%s-%s", CONF_FAMILY_STRING, CONF_PLATFORM_STRING, CONF_ARCH_STRING);
#ifdef CONF_ARCH_ENDIAN_LITTLE
	dbg_msg("engine", "arch is little endian");
#elif defined(CONF_ARCH_ENDIAN_BIG)
	dbg_msg("engine", "arch is big endian");
#else
	dbg_msg("engine", "unknown endian");
#endif

	/* init the network */
	net_init();
	netcommon_init();
	
	/* create storage location */
	{
		char path[1024] = {0};
		fs_storage_path(appname, application_save_path, sizeof(application_save_path));
		if(fs_makedir(application_save_path) == 0)
		{		
			str_format(path, sizeof(path), "%s/screenshots", application_save_path);
			fs_makedir(path);

			str_format(path, sizeof(path), "%s/maps", application_save_path);
			fs_makedir(path);

			str_format(path, sizeof(path), "%s/demos", application_save_path);
			fs_makedir(path);
		}
	}

	/* init console and add the console logger */
	console_init();
	dbg_logger(console_print);
	
	jobs_initpool(&hostlookuppool, 1);

	MACRO_REGISTER_COMMAND("dbg_dumpmem", "", con_dbg_dumpmem, 0x0);
	
	/* reset the config */
	config_reset();
}

void engine_parse_arguments(int argc, char **argv)
{
	/* load the configuration */
	int i;
	int abs = 0;
	const char *config_filename = "settings.cfg";
	char buf[1024];
	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-' && argv[i][1] == 'f' && argv[i][2] == 0 && argc - i > 1)
		{
			config_filename = argv[i+1];
			abs = 1;
			i++;
		}
		else if(argv[i][0] == '-' && argv[i][1] == 'd' && argv[i][2] == 0 && argc - i > 1)
		{
			datadir_override = argv[i+1];
			i++;
		}
	}

	if(abs)
		console_execute_file(config_filename);
	else
		console_execute_file(engine_savepath(config_filename, buf, sizeof(buf)));
	
	/* search arguments for overrides */
	{
		int i;
		for(i = 1; i < argc; i++)
			console_execute_line(argv[i]);
	}

	console_execute_file(engine_savepath("autoexec.cfg", buf, sizeof(buf)));

	/* open logfile if needed */
	if(config.logfile[0])
		dbg_logger_file(config.logfile);

	/* set default servers and load from disk*/
	mastersrv_default();
	mastersrv_load();
}


static IOHANDLE config_file = 0;

int engine_config_write_start()
{
	char filename[1024];
	config_save(engine_savepath("settings.cfg", filename, sizeof(filename)));
	
	config_file = io_open(filename, IOFLAG_WRITE);
	if(config_file == 0)
		return -1;
	return 0;
}

void engine_config_write_line(const char *line)
{
	if(config_file)
	{
#if defined(CONF_FAMILY_WINDOWS)
		static const char newline[] = "\r\n";
#else
		static const char newline[] = "\n";
#endif
		io_write(config_file, line, strlen(line));
		io_write(config_file, newline, sizeof(newline)-1);
	}
}

void engine_config_write_stop()
{
	io_close(config_file);
	config_file = 0;
}
/*
void engine_writeconfig()
{
}*/

static int perf_tick = 1;
static PERFORMACE_INFO *current = 0;

void perf_init()
{
}

void perf_next()
{
	perf_tick++;
	current = 0;
}

void perf_start(PERFORMACE_INFO *info)
{
	if(info->tick != perf_tick)
	{
		info->parent = current;
		info->first_child = 0;
		info->next_child = 0;
		
		if(info->parent)
		{
			info->next_child = info->parent->first_child;
			info->parent->first_child = info;
		}
		
		info->tick = perf_tick;
		info->biggest = 0;
		info->total = 0;
	}
	
	current = info;
	current->start = time_get();
}

void perf_end()
{
	if(!current)
		return;
		
	current->last_delta = time_get()-current->start;
	current->total += current->last_delta;
	
	if(current->last_delta > current->biggest)
		current->biggest = current->last_delta;
	
	current = current->parent;
}

static void perf_dump_imp(PERFORMACE_INFO *info, int indent)
{
	char buf[512] = {0};
	int64 freq = time_freq();
	int i;
	
	for(i = 0; i < indent; i++)
		buf[i] = ' ';
	
	str_format(&buf[indent], sizeof(buf)-indent, "%-20s %8.2f %8.2f", info->name, info->total*1000/(float)freq, info->biggest*1000/(float)freq);
	dbg_msg("perf", "%s", buf);
	
	info = info->first_child;
	while(info)
	{
		perf_dump_imp(info, indent+2);
		info = info->next_child;
	}
}

void perf_dump(PERFORMACE_INFO *top)
{
	perf_dump_imp(top, 0);
}

/* master server functions */
typedef struct
{
	char hostname[128];
	NETADDR addr;
	
	HOSTLOOKUP lookup;
} MASTER_INFO;

static MASTER_INFO master_servers[MAX_MASTERSERVERS] = {{{0}}};
static int needs_update = -1;

int mastersrv_refresh_addresses()
{
	int i;
	
	if(needs_update != -1)
		return 0;
	
	dbg_msg("engine/mastersrv", "refreshing master server addresses");

	/* add lookup jobs */
	for(i = 0; i < MAX_MASTERSERVERS; i++)	
		engine_hostlookup(&master_servers[i].lookup, master_servers[i].hostname);
	
	needs_update = 1;
	return 0;
}

void mastersrv_update()
{
	int i;
	
	/* check if we need to update */
	if(needs_update != 1)
		return;
	needs_update = 0;
	
	for(i = 0; i < MAX_MASTERSERVERS; i++)
	{
		if(jobs_status(&master_servers[i].lookup.job) != JOBSTATUS_DONE)
			needs_update = 1;
		else
		{
			master_servers[i].addr = master_servers[i].lookup.addr;
			master_servers[i].addr.port = 8300;
		}
	}
	
	if(!needs_update)
	{
		dbg_msg("engine/mastersrv", "saving addresses");
		mastersrv_save();
	}
}

int mastersrv_refreshing()
{
	return needs_update;
}

NETADDR mastersrv_get(int index) 
{
	return master_servers[index].addr;
}

const char *mastersrv_name(int index) 
{
	return master_servers[index].hostname;
}

void mastersrv_dump_servers()
{
	int i;
	for(i = 0; i < MAX_MASTERSERVERS; i++)
	{
		dbg_msg("mastersrv", "#%d = %d.%d.%d.%d", i,
			master_servers[i].addr.ip[0], master_servers[i].addr.ip[1],
			master_servers[i].addr.ip[2], master_servers[i].addr.ip[3]);
	}
}

void mastersrv_default()
{
	int i;
	mem_zero(master_servers, sizeof(master_servers));
	for(i = 0; i < MAX_MASTERSERVERS; i++)
		sprintf(master_servers[i].hostname, "master%d.teeworlds.com", i+1);
}

int mastersrv_load()
{
	LINEREADER lr;
	IOHANDLE file;
	int count = 0;
	char filename[1024];
	
	engine_savepath("masters.cfg", filename, sizeof(filename));
	
	/* try to open file */
	file = io_open(filename, IOFLAG_READ);
	if(!file)
		return -1;
	
	linereader_init(&lr, file);
	while(1)
	{
		MASTER_INFO info = {{0}};
		int ip[4];
		const char *line = linereader_get(&lr);
		if(!line)
			break;

		/* parse line */		
		if(sscanf(line, "%s %d.%d.%d.%d", info.hostname, &ip[0], &ip[1], &ip[2], &ip[3]) == 5)
		{
			info.addr.ip[0] = (unsigned char)ip[0];
			info.addr.ip[1] = (unsigned char)ip[1];
			info.addr.ip[2] = (unsigned char)ip[2];
			info.addr.ip[3] = (unsigned char)ip[3];
			info.addr.port = 8300;
			if(count != MAX_MASTERSERVERS)
			{
				master_servers[count] = info;
				count++;
			}
			else
				dbg_msg("engine/mastersrv", "warning: skipped master server '%s' due to limit of %d", line, MAX_MASTERSERVERS);
		}
		else
			dbg_msg("engine/mastersrv", "warning: couldn't parse master server '%s'", line);
	}
	
	io_close(file);
	return 0;
}

int mastersrv_save()
{
	IOHANDLE file;
	int i;
	char filename[1024];

	engine_savepath("masters.cfg", filename, sizeof(filename));
	
	/* try to open file */
	file = io_open(filename, IOFLAG_WRITE);
	if(!file)
		return -1;

	for(i = 0; i < MAX_MASTERSERVERS; i++)
	{
		char buf[1024];
		str_format(buf, sizeof(buf), "%s %d.%d.%d.%d\n", master_servers[i].hostname,
			master_servers[i].addr.ip[0], master_servers[i].addr.ip[1],
			master_servers[i].addr.ip[2], master_servers[i].addr.ip[3]);
			
		io_write(file, buf, strlen(buf));
	}
	
	io_close(file);
	return 0;
}


int hostlookup_thread(void *user)
{
	HOSTLOOKUP *lookup = (HOSTLOOKUP *)user;
	net_host_lookup(lookup->hostname, &lookup->addr, NETTYPE_IPV4);
	return 0;
}

void engine_hostlookup(HOSTLOOKUP *lookup, const char *hostname)
{
	str_copy(lookup->hostname, hostname, sizeof(lookup->hostname));
	jobs_add(&hostlookuppool, &lookup->job, hostlookup_thread, lookup);
}

int engine_chdir_datadir(char *argv0)
{
	int found = 0;
	char data_dir[1024*2];
	
	/* 1) use provided data-dir override */
	if (datadir_override)
	{
		if (fs_is_dir(datadir_override))
		{
			str_copy(data_dir, datadir_override, sizeof(data_dir));
			dbg_msg("engine/datadir", "using override '%s'", data_dir);
			found = 1;
		}
		else
		{
			dbg_msg("engine/datadir",
				"specified data-dir '%s' does not exist",
				datadir_override);
			return 0;
		}
	}
	
	/* 2) use data-dir in PWD if present */
	if (!found && fs_is_dir("data"))
	{
		strcpy(data_dir, "data");
		found = 1;
	}
	
	/* 3) use compiled-in data-dir if present */
	if (!found && fs_is_dir(DATA_DIR))
	{
		strcpy(data_dir, DATA_DIR);
		found = 1;
	}
	
	/* 4) check for usable path in argv[0] */
	if (!found)
	{
		unsigned int pos = strrchr(argv0, '/') - argv0;
		
		if (pos < sizeof(data_dir))
		{
			char basedir[sizeof(data_dir)];
			strncpy(basedir, argv0, pos);
			basedir[pos] = '\0';
			str_format(data_dir, sizeof(data_dir),
				"%s/data", basedir);
			
			if (fs_is_dir(data_dir))
				found = 1;
		}
	}
	
#if defined(CONF_FAMILY_UNIX)
	/* 5) check for all default locations */
	if (!found)
	{
		const char *sdirs[] = {
			"/usr/share/teeworlds",
			"/usr/local/share/teeworlds"
		};
		const int sdirs_count = sizeof(sdirs) / sizeof(sdirs[0]);
		
		int i;
		for (i = 0; i < sdirs_count; i++)
		{
			if (fs_is_dir(sdirs[i]))
			{
				strcpy(data_dir, sdirs[i]);
				found = 1;
				break;
			}
		}
	}
#endif
	
	/* data-dir exists */
	if (found)
	{
		dbg_msg("engine/datadir", "using '%s'", data_dir);
		
		/* change working directory to data-dir */
		fs_chdir(data_dir);
	}
	
	return found;
}
