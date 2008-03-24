# coding: utf-8
from socket import *
import struct
import sys

def tw_get_server_info(address, port):
	try:
		sock = socket(AF_INET, SOCK_DGRAM) 
		sock.settimeout(1.5); 
		sock.sendto("\xff\xff\xff\xff\xff\xff\xff\xff\xff\xffgief", (address, port)) 
		data, addr = sock.recvfrom(1024) 
		sock.close() 
	 
		data = data[14:] # skip header
	 	
		slots = data.split("\x00")
		server_info = {}
		server_info["version"] = slots[0]
		server_info["name"] = slots[1]
		server_info["map"] = slots[2]
		server_info["gametype_id"] = int(slots[3])
		server_info["flags"] = int(slots[4])
		server_info["progression"] = int(slots[5])
		server_info["num_players"] = int(slots[6])
		server_info["max_players"] = int(slots[7])
		server_info["players"] = []
		
		for i in xrange(0, server_info["num_players"]):
			player = {}
			player["name"] = slots[8+i*2+1]
			player["score"] = slots[8+i*2]
			server_info["players"] += [player]
			
		gametypes = ["dm", "tdm", "ctf"]
		try: server_info["gametype_name"] = gametypes[server_info["gametype_id"]]
		except: server_info["gametype_name"] = "unknown"
		
		return server_info
	except:
		return None
		
def tw_get_server_count(address, port):
	try:
		sock = socket(AF_INET, SOCK_DGRAM) 
		sock.settimeout(1.5); 
		sock.sendto("\xff\xff\xff\xff\xff\xff\xff\xff\xff\xffcoun", (address, port)) 
		data, addr = sock.recvfrom(1024) 
		sock.close() 
	 
		data = data[14:] # skip header
		return struct.unpack(">H", data)[0]
	except:
		return -1

def tw_get_servers(address):
	counter = 0
	master_port = 8300
	servers = []
 
	try:
		sock = socket(AF_INET, SOCK_DGRAM) 
		sock.settimeout(1.5) 
		sock.sendto("\x20\x00\x00\x00\x00\x00\xff\xff\xff\xffreqt", (address, master_port)) 
 
		data, addr = sock.recvfrom(1024) 
		sock.close() 
		data = data[14:] 
		num_servers = len(data) / 6 
		num_players = 0 

		for n in range(0, num_servers): 
			ip = ".".join(map(str, map(ord, data[n*6:n*6+4]))) 
			port = ord(data[n*6+5]) * 256 + ord(data[n*6+4]) 
			servers += [[ip, port]]

		return servers
	except:
		return None		


#tw_get_server_info("10.0.0.2", 8303)

#sys.exit(-1)

servers = []
for i in range(1, 16):
	addr = "master%d.teeworlds.com"%i
	list = tw_get_servers(addr)
	if list:
		print "%s had %d servers" % (addr, len(list))
		servers += list

total_players = 0
for server in servers:
	print "checking server", server[0], server[1]
	info = tw_get_server_info(server[0], server[1])
	if info:
		total_players += len(info["players"])

print total_players
