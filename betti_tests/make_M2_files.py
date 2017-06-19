# There are a bunch of .txt files in this folder.
# Make all the corresponding M2 files - bigraded, singly-graded w/o minimalBetti, and using minimalBetti

import os

for filename in os.listdir():
	if filename.endswith(".txt"):
		os.system("./../build/rivet_console %s --betti -H 0 -V 2" %filename)

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