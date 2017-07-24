# Use a singly-graded M2 file to produce a Singular file.
# This script should be run AFTER the 0th homology files are simplified.

import os
import string
import re

for file_name in os.listdir("singly_graded/"):
	if not file_name.endswith("singly_graded.m2"):
		continue
	is_0th_homol = True if file_name[-18:-17] == '0' else False
	res_file = file_name[:-3] + "_res.sing"
	print("Writing the file %s" % res_file)

	# Open f1 for writing and f2 for reading
	f1 = open(res_file, "w")
	f2 = open("singly_graded/" + file_name, "r")

	# Start writing to the new file
	f1.write('LIB "homolog.lib";\n')
	f1.write('ring R=2,(x,y),dp;\n')

	# Line 1 should be "R=ZZ/2[x,y,Degrees=>{{1},{1}}]"
	if f2.readline() != "R=ZZ/2[x,y,Degrees=>{{1},{1}}]\n":
		print("Warning: unexpected format of M2 file on line 1 of file: %s" % file_name)

	# We can discard the information given by C2.
	c2 = f2.readline()
	if c2[:6] != "C2=R^{" or c2[-2:] != "}\n":
		print("Warning: unexpected format of M2 file for c2")

	# Collect the information given in C1.
	c1 = f2.readline()
	if c1[:6] != "C1=R^{" or c1[-2:] != "}\n":
		print("Warning: unexpected format of M2 file for c1")
	if len(c1) != 8:
		c1_strings = c1[7:-3].split("},{")
		# Remove the minus sign if the number is not 0
		c1_strings = map(lambda x : x if x == "0" else x[1:], c1_strings)
		f1.write("intvec single_grades_C1=")
		f1.write(",".join(c1_strings))
		f1.write(";\n")
	else:
		print("Warning: Nothing contained in c1 of M2 file")
		f1.write("intvec single_grades_C1;\n")

	# Collect C0 only if working in non-0th homology.
	if not is_0th_homol:
		c0 = f2.readline()
		if c0[:6] != "C0=R^{" or c0[-2:] != "}\n":
			print("Warning: unexpected format of M2 file for c0")
		c0_strings = c0[7:-3].split("},{")
		# Remove the minus sign if the number is not 0
		c0_strings = map(lambda x : x if x == "0" else x[1:], c0_strings)
		f1.write("intvec single_grades_C0=")
		f1.write(",".join(c0_strings))
		f1.write(";\n")
	else:
		f1.write("//No C0 for 0th homology\n")

	# Next line should be Id=promote(1,R).
	if f2.readline() != "Id=promote(1,R)\n":
		print("Warning: expected Id=promote(1,R) but got something else")

	# Collect the data to make d2. We do this by extracting all the ordered pairs first, since 
	# the columns of d2 are identified one at a time.
	d2 = f2.readline()
	if d2[:14] != "d2=map(C1,C2,{" or d2[-4:] != "});\n":
		print("Warning: unexpected format of M2 file for d2")
	f1.write("module d2;\n")
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
		f1.write("d2[%d]=" % (key + 1))
		pairs = d2_dict[key]
		terms = map(lambda x : "%sgen(%d)" % ("" if x[1] == "Id" else (x[1] + "*"), x[0] + 1), pairs)
		f1.write("%s;\n" % "+".join(terms))

	# Repeat for d1. Once again, this is irrelevant for 0th homology.
	if not is_0th_homol:
		d1 = f2.readline()
		if d1[:14] != "d1=map(C0,C1,{" or d1[-4:] != "});\n":
			print("Warning: unexpected format of M2 file on line 7")
		# d2_pieces contains things of the form (a,b)=>c, where a and b are integers and c is a monomial
		f1.write("module d1;\n")
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
			f1.write("d1[%d]=" % (key + 1))
			pairs = d1_dict[key]
			terms = map(lambda x : "%sgen(%d)" % ("" if x[1] == "Id" else (x[1] + "*"), x[0] + 1), pairs)
			f1.write("%s;\n" % "+".join(terms))
	else:
		f1.write("//No d1 for 0th homology\n")


	# Almost done! We don't care about the rest of the M2 file (which doesn't contain information we need).
	f2.close()
	if is_0th_homol:
		f1.write('attrib(d2,"isHomog",single_grades_C1);\n')
		f1.write('timer=0;\n')
		f1.write('system("--ticks-per-sec",1000); //timer resolution in ms\n')
		f1.write('int t0=timer;\n')
		f1.write('resolution resH=res(d2,0);\n')	
		f1.write('int t1 = timer - t0;\n')
		f1.write('print(betti(resH), "betti");\n')
		f1.write('int t2 = timer - t1 - t0;\n')
		f1.write('write(":a singular_res_times.txt", string(t1, " ", t2));\n')
	else:
		f1.write('attrib(d2,"isHomog",single_grades_C1);\n')
		f1.write('attrib(d1,"isHomog",single_grades_C0);\n')
		f1.write('timer=0;\n')
		f1.write('system("--ticks-per-sec",1000); //timer resolution in ms\n')
		f1.write('int t0=timer;\n')
		f1.write('module triv=0;\n')
		f1.write('def H=homology(d2,d1,triv,triv);\n')
		f1.write('int t1 = timer - t0;\n')
		f1.write('resolution resH=res(H,0);\n')	
		f1.write('int t2 = timer - t1 - t0;\n')
		f1.write('print(betti(resH), "betti");\n')
		f1.write('int t3 = timer - t2 - t1 - t0;\n')
		f1.write('write(":a singular_res_times.txt", string(t1, " ", t2, " ", t3));\n')
	f1.close()

	# Copy the lines into another file, with a couple of tweaks -- repeat with sres()
	sres_file = file_name[:-3] + "_sres.sing"
	f1 = open(sres_file, "w")
	f2 = open(res_file, "r")

	f1.write('LIB "schreyer.lib";\n')
	for line in f2:
		f1.write(line.replace("resolution resH=res(", "resolution resH=s_res(")
			.replace("singular_res_times.txt", "singular_sres_times.txt"))
	f1.close()
	f2.close()

	os.system("mv %s singly_graded/%s" % (sres_file, sres_file))
	os.system("mv %s singly_graded/%s" % (res_file, res_file))