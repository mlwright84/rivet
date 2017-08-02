# Deletes all of the Macaulay2, Singular, and RIVET data files in this folder and its subdirectories.
# You probably don't want to use this one too often, unless you're sure you don't want the data anymore.

import os
import sys

# Make sure we're not runnign this on accident
if not input("Are you sure you want to delete all the data files? y/n ") == 'y':
	sys.exit()
if not input("Are you absolutely sure? This will delete all .txt files as well! y/n ") == 'y':
	sys.exit()
if not input("Type 'hopscotch' to delete all the data files. ") == 'hopscotch':
	sys.exit()


for r, d, files in os.walk("./"):
	for file in files:
		if file.endswith(".m2") or file.endswith(".sing") or file.endswith(".txt"):
			os.system("rm %s" % os.path.join(r, file))