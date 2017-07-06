# Use a singly-graded M2 file to produce a Singular file.
# RUN THIS SCRIPT BEFORE THE M2 FILES ARE SIMPLIFIED
# TODO: Fix indexing

import os
import string
import re

for file_name in os.listdir():
	if not file_name.endswith(".m2"):
		continue
	s_file = file_name[:-3] + ".txt"
	print("Writing the file %s" %s_file)

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
	if c2[:6] != "C2=R^{" or c2[-2:] != "}\n":
		print("Warning: unexpected format of M2 file on line 2")

	# Collect the information given in C1
	c1 = f2.readline()
	if c1[:6] != "C1=R^{" or c1[-2:] != "}\n":
		print("Warning: unexpected format of M2 file on line 3")
	if len(c1) != 8:
		c1_strings = c1[7:-3].split("},{")
		# Remove the minus sign if the number is not 0
		c1_strings = map(lambda x : x if x == "0" else x[1:], c1_strings)
		f1.write("intvec single_grades_C1=")
		f1.write(",".join(c1_strings))
		f1.write(";\n")
	else:
		f1.write("intvec single_grades_C1;\n")

	# Also collect C0
	c0 = f2.readline()
	if c0[:6] != "C0=R^{" or c1[-2:] != "}\n":
		print("Warning: unexpected format of M2 file on line 4")
	if len(c0) != 8:
		c0_strings = c0[7:-3].split("},{")
		# Remove the minus sign if the number is not 0
		c0_strings = map(lambda x : x if x == "0" else x[1:], c0_strings)
		f1.write("intvec single_grades_C0=")
		f1.write(",".join(c1_strings))
		f1.write("\n")
	else:
		f1.write("intvec single_grades_C0;\n")

	# Line5 should be Id=promote(1,R).
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
	d2_dict = dict()
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
		# need to put the monomial (monomial) in the correct position (x_val, y_val).
		# In general, we use d[y_val] = stuff with x_val.
		# We store the items in a dictionary: keys are y coords, items are lists of (x, monomial) tuples
		if y_val in d2_dict:
			d2_dict.get(y_val).append((x_val, monomial))
		else:
			d2_dict[y_val] = [(x_val, monomial)]
	# Data collected; now, output it to the file.
	for key in d2_dict:
		f1.write("d2[%d]=" %key)
		pairs = d2_dict[key]
		terms = map(lambda x : "%s*gen(%d)" % (x[1], x[0]), pairs)
		f1.write("%s;\n" % "+".join(terms))

	# Repeat for d1
	f1.write("module d1;\n")
	d1 = f2.readline()
	if d1[:14] != "d1=map(C0,C1,{" or d1[-4:] != "});\n":
		print("Warning: unexpected format of M2 file on line 7")
	# d2_pieces contains things of the form (a,b)=>c, where a and b are integers and c is a monomial
	d1_pieces = re.findall(r"\(.*?\)=>[^,]*", d1[14:-4])
	d1_dict = dict()
	for term in d1_pieces:
		comma_pos = term.find(",")
		if comma_pos == -1:
			print("Warning: string matching failed for line 7")
		x_val = int(term[1 : comma_pos])
		arrow_pos = term.find(")=>")
		if arrow_pos == -1:
			print("Warning: string matching failed for line 7")
		y_val = int(term[comma_pos + 1 : arrow_pos])
		monomial = term[arrow_pos + 3:]
		# need to put the monomial (monomial) in the correct position (x_val, y_val).
		# In general, we use d[y_val] = stuff with x_val.
		# We store the items in a dictionary: keys are y coords, items are lists of (x, monomial) tuples
		if y_val in d1_dict:
			d1_dict.get(y_val).append((x_val, monomial))
		else:
			d1_dict[y_val] = [(x_val, monomial)]
	# Data collected; now, output it to the file.
	for key in d1_dict:
		f1.write("d1[%d]=" % key)
		pairs = d1_dict[key]
		terms = map(lambda x : "%s*gen(%d)" % (x[1], x[0]), pairs)
		f1.write("+".join(terms))


	# Almost done! We don't care about the rest of the M2 file (which doesn't contain information we need).
	f2.close()
	f1.write('attrib(d2,"isHomog",single_grades_C1);\n')
	f1.write('attrib(d1,"isHomog",single_grades_C0);\n')
	f1.write("module triv=0;\n")
	f1.write("def H=homology(d2,d1,triv,triv);\n")
	f1.write("resolution resH=lres(H,0);\n")
	f1.write('print(betti(resH), "betti");\n')
	f1.close()

#TODO: redo using sres() from Shcreyer.lib