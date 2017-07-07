# M2 files for 0th homology computations contain some extra, unneeded lines.
# This script removes them.

import os

for r, d, files in os.walk("./"):
	for file in files:
		if file.endswith(".m2"):
			f1 = open(file, 'r')
			f2 = open("temp.txt", 'w')
			bool simplify = False
			for line in f1:
				if line == "C0=R^{}\n" or line == "d1=map(C0,C1,{});\n":
					simplify = True
					continue
				elif simplify:
					f2.write(line.replace("HM=homology(d1,d2)", "coker(d2)"))
				else
					f2.write(line)
			f1.close()
			f2.close()
			if file.endswith("bigraded.m2"):
				os.system("mv temp.txt bigraded/%s" % filename)
			elif file.endswith("singly_graded.m2"):
				os.system("mv temp.txt singly_graded/%s" % filename)
			elif file.endswith("minimal_betti.m2"):
				os.system("mv temp.txt minimal_betti/%s" % filename)
