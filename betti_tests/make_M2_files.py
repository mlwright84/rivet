# There are a bunch of .txt files in this folder.
# Make all the corresponding M2 files - bigraded, singly-graded w/o minimalBetti, and using minimalBetti
# Also, this will collect timing data from RIVET.

import os

for homology in range(2):
	for filename in os.listdir():
		if filename.endswith(".txt"):
			os.system("./../build/rivet_console %s --betti -H %d" % (filename, homology))

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