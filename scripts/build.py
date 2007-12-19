import os, sys, shutil, httplib, zipfile

version = sys.argv[1]

flag_download = True
flag_clean = True

if os.name == "nt":
	platform = "win32"
else:
	# get name
	name = os.popen("uname").readline().strip().lower()
	if name == "darwin":
		name = "osx"
		
	# get arch
	machine = os.popen("uname -m").readline().strip().lower()
	arch = "unknown"
	
	if machine[0] == "i" and machine[2:] == "86":
		arch = "x86"
	elif machine == "x86_64":
		arch = "x86_64"
	elif "power" in machine.lower():
		arch = "ppc"
		
	platform = name + "_" + arch

print "teewars-%s-%s" % (version, platform)

src_package = "teewars-%s-src.zip" % version

root_dir = os.getcwd() + "/work"
src_dir = ""

def fetch_file(server, url, local):
	try:
		conn = httplib.HTTPConnection(server)
		conn.request("GET", url)
		response = conn.getresponse()
		if response.status != 200:
			return False
		
		f = file(local, "wb")
		f.write(response.read())
		f.close()
		conn.close()
		return True
	except:
		pass
	return False

def unzip(filename, where):
	z = zipfile.ZipFile(filename, "r")
	for name in z.namelist():
		try: os.makedirs(where+"/"+os.path.dirname(name))
		except: pass
		f = file(where+"/"+name, "wb")
		f.write(z.read(name))
		f.close()
	z.close()


def path_exist(d):
	try: os.stat(d)
	except: return False
	return True

def bail(reason):
	print reason
	os.chdir(root_dir)
	sys.exit(-1)

# clean
if flag_clean:
	print "*** cleaning ***"
	try: shutil.rmtree("work")
	except: pass
	
# make dir
try: os.mkdir("work")
except: pass

# change dir
os.chdir(root_dir)

# download
if flag_download:
	print "*** downloading bam source package ***"
	if not fetch_file("www.teewars.com", "/files/bam.zip", "bam.zip"):
		if not fetch_file("www.teewars.com", "/files/beta/bam.zip", "bam.zip"):
			bail("couldn't find source package and couldn't download it")
		
	print "*** downloading teewars source package ***"
	if not fetch_file("www.teewars.com", "/files/%s" % src_package, src_package):
		if not fetch_file("www.teewars.com", "/files/beta/%s" % src_package, src_package):
			bail("couldn't find source package and couldn't download it")

# unpack
print "*** unpacking source ***"
unzip("bam.zip", ".")
unzip(src_package, "teewars")
src_dir = "teewars/"+ os.listdir("teewars/")[0]

# build bam
if 1:
	print "*** building bam ***"
	os.chdir("bam")
	output = "bam"
	bam_cmd = "./bam"
	if os.name == "nt":
		if os.system("make_win32_msvc2005.bat") != 0:
			bail("failed to build bam")
		output += ".exe"
		bam_cmd = "bam"
	else:
		if os.system("sh make_unix.sh") != 0:
			bail("failed to build bam")
	os.chdir(root_dir)
	shutil.copy("bam/src/"+output, src_dir+"/"+output)

# build teewars
if 1:
	print "*** building teewars ***"
	os.chdir(src_dir)
	if os.system("%s server_release client_release" % bam_cmd) != 0:
		bail("failed to build teewars")
	if name == 'osx':
		if os.system("%s TeeLaunch_release" % bam_cmd) != 0:
			bail("failed to build OSX Launcher")
	os.chdir(root_dir)

# make release
if 1:
	print "*** making release ***"
	os.chdir(src_dir)
	if os.system("python scripts/make_release.py %s %s" % (version, platform)) != 0:
		bail("failed to make a relase of teewars")
	final_output = "FAIL"
	for f in os.listdir("."):
		if version in f and platform in f and "teewars" in f and (".zip" in f or ".tar.gz" in f):
			final_output = f
	os.chdir(root_dir)
	shutil.copy("%s/%s" % (src_dir, final_output), "../"+final_output)

print "*** all done ***"

		
