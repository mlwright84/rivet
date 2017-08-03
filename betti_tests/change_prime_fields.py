# Make copies of the M2 and Singular files, but using prime fields other than 2.

import os

for r, d, files in os.walk("./"):
	for file in files:
		if "mod_2" in file and file.endswith(".m2"):
			f1 = open(os.path.join(r, file), 'r')
			f2 = open(os.path.join(r, file).replace("mod_2", "mod_13"))
			for line in f1:
				f2.write(line.replace("R=ZZ/2", "R = ZZ/13"))
			f1.close()
			f2.close()
		elif "mod_2" in file and file.endswith(".sing"):
			f1 = open(os.path.join(r, file), 'r')
			f2 = open(os.path.join(r, file).replace("mod_2", "mod_13"))
			for line in f1:
				f2.write(line.replace("ring R=2", "ring R=13"))
			f1.close()
			f2.close()