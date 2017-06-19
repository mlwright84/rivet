# mikestillman proposed a workaround to an apparent bug in minimalBetti
# If the bug is patched, this script will remove the workaround.

import os

for file in os.listdir():
	if file.endswith("minimal_betti.m2"):
		f1 = open(file, 'r')
		f2 = open("temp.txt", 'w')
		for line in f1:
			f2.write(line.replace("R=ZZ/2[x,y,Degrees=>{1,1},MonomialOrder=>{Position=>Up}]",
				"R=ZZ/2[x,y,Degrees=>{1,1}]"))
		f1.close()
		f2.close()
		os.system("mv temp.txt %s" % file)
