# This script collects timing data from RIVET, and also generates three types of Macaulay2 files.
# Each Macaulay2 file is then moved to an appropriate folder.
# Takes in one command-line argument, the prime field to be used.
# The prime field defaults to 2 if no argument is given.

import os
import sys

prime = 2 if len(sys.argv) == 1 else int(sys.argv[1]) 

for homology in range(2):
	for filename in os.listdir():
		if filename.endswith(".txt"):
			os.system("./../build/rivet_console %s --betti -H %d -p %d" % (filename, homology, prime))

os.system("mkdir bigraded")
os.system("mkdir singly_graded")
os.system("mkdir minimal_betti")

for filename in os.listdir():
	if filename.endswith("bigraded.m2"):
		os.system("mv %s bigraded/%s" % (filename, filename))
	elif filename.endswith("singly_graded.m2"):
		os.system("mv %s singly_graded/%s" % (filename, filename))
	elif filename.endswith("minimal_betti.m2"):
		os.system("mv %s minimal_betti/%s" % (filename, filename))