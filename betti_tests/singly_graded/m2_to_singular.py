# Use a singly-graded M2 file to produce a Singular file.

import os
import string
import re

for file_name in os.listdir():
	if not file_name.endswith(".m2"):
		continue
	s_file = file_name[:-3] + ".txt"

	# Open f1 for writing and f2 for reading
	f1 = open(s_file, "w")
	f2 = open(file_name, "r")

	# Start writing to the new file
	f1.write('LIB "homolog.lib";\n')
	f1.write('ring R=2,(x,y),dp;\n')

	# Line 1 should be "R=ZZ/2[x,y,Degrees=>{{1},{1}}]"
	if f2.readline() != "R=ZZ/2[x,y,Degrees=>{{1},{1}}]\n":
		print("Warning: unexpected format of M2 file on line 1 of file: %s" % file_name)

	# We can discard the information given by C2.
	c2 = f2.readline()
	if c2[:7] != "C2=R^{{" or c2[-3:] != "}}\n":
		print("Warning: unexpected format of M2 file on line 2")

	# Collect the information given in C1
	c1 = f2.readline()
	if c1[:7] != "C1=R^{{" or c1[-3:] != "}}\n":
		print("Warning: unexpected format of M2 file on line 3")
	c1_strings = c1[7:-3].split("},{")
	f1.write("intvec single_grades_C1=")
	# Remove the minus sign if the number is not 0
	c1_strings = map(lambda x : x if x == "0" else x[1:], c1_strings)
	f1.write(",".join(c1_strings))
	f1.write("\n")

	# Lines 4 and 5 should be C0=R^{} and Id=promote(1,R), respectively.
	if f2.readline() != "C0=R^{}\n":
		print("Warning: unexpected format of M2 file on line 4")
	# WRONG - Line 5 isn't trivial for 1st homology
	if f2.readline() != "Id=promote(1,R)\n":
		print("Warning: unexpected format of M2 file on line 5")

	# Collect the data to make d2. We do this by extracting all the ordered pairs first, since 
	# the columns of d2 are identified one at a time.
	f1.write("module d2;\n")
	d2 = f2.readline()
	if d2[:14] != "d2=map(C1,C2,{" or d2[-4:] != "});\n":
		print("Warning: unexpected format of M2 file on line 6")
	# d2_pieces contains things of the form (a,b)=>c, where a and b are integers and c is a monomial
	d2_pieces = re.findall(r"\(.*?\)=>[^,]*", d2[14:-4])
	for term in d2_pieces:
		comma_pos = term.find(",")
		if comma_pos == -1:
			print("Warning: string matching failed for line 6")
		x_val = int(term[1 : comma_pos])
		arrow_pos = term.find(")=>")
		if arrow_pos == -1:
			print("Warning: string matching failed for line 6")
		y_val = int(term[comma_pos + 1 : arrow_pos])
		monomial = term[arrow_pos + 3:]



	# TODO: Figure out how to do d2 and d1
	f1.write("ERROR: d2 and d1 unimplemented\n")

	# Almost done! We don't care about the rest of the M2 file (which doesn't contain information we need).
	f2.close()
	f1.write('attrib(d2,"isHomog",single_grades_C1);\n')
	f1.write("module triv=0;\n")
	f1.write("def H=homology(d2,d1,triv,triv);\n")
	f1.write("resolution resH=lres(H,0);")
	f1.write('print(betti(resH), "betti");')
	f1.close()

