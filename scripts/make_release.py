import shutil, os, sys, zipfile

valid_platforms = ["win32", "linux86", "linux86_64", "src"]

if len(sys.argv) != 3:
	print "wrong number of arguments"
	print sys.argv[0], "VERSION PLATFORM"
	sys.exit(-1)

name = "teewars"
version = sys.argv[1]
platform = sys.argv[2]
exe_ext = ""
use_zip = 0
use_gz = 1
include_data = True
include_exe = True
include_src = False

if platform == "src":
	include_data = False
	include_exe = False
	include_src = True
	use_zip = 1
	use_gz = 1

if not platform in valid_platforms:
	print "not a valid platform"
	print valid_platforms
	sys.exit(-1)

if platform == 'win32':
	exe_ext = ".exe"
	use_zip = 1
	use_gz = 0

def copydir(src, dst, excl=[]):
	for root, dirs, files in os.walk(src, topdown=True):
		if "/." in root or "\\." in root:
			continue
		for name in dirs:
			if name[0] != '.':
				os.mkdir(os.path.join(dst, root, name))
		for name in files:
			if name[0] != '.':
				shutil.copy(os.path.join(root, name), os.path.join(dst, root, name))
				
package = "%s-%s-%s" %(name, version, platform)
package_dir = package

print "cleaning target"
shutil.rmtree(package_dir, True)
os.mkdir(package_dir)

print "adding files"
shutil.copy("readme.txt", package_dir)
shutil.copy("license.txt", package_dir)

if include_data:
	os.mkdir(os.path.join(package_dir, "data"))
	copydir("data", package_dir)

if include_exe:
	shutil.copy("teewars"+exe_ext, package_dir)
	shutil.copy("teewars_srv"+exe_ext, package_dir)
	
if include_src:
	for p in ["src", "scripts", "datasrc"] :
		os.mkdir(os.path.join(package_dir, p))
		copydir(p, package_dir)
	shutil.copy("default.bam", package_dir)

if use_zip:
	print "making zip archive"
	zf = zipfile.ZipFile("%s.zip" % package, 'w', zipfile.ZIP_DEFLATED)
	
	for root, dirs, files in os.walk(package_dir, topdown=True):
		for name in files:
			n = os.path.join(root, name)
			zf.write(n, n)
	#zf.printdir()
	zf.close()
	
if use_gz:
	print "making tar.gz archive"
	os.system("tar czf %s.tar.gz %s" % (package, package_dir))
	
print "done"
