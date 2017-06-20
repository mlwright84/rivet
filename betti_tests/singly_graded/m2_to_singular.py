# Use a singly-graded M2 file to produce a Singular file.

import os

for file_name in os.listdir(directory):
	if not file_name.endswith(".m2"):
		continue
	s_file = file_name[-3:]

	# Open f1 for writing and f2 for reading
	f1 = open(s_file, "w")
	f2 = open(file_name, "r")

	# Start writing to the new file
	f1.write('LIB "homolog.lib";\n')
	f1.write('ring R=2,(x,y),dp;\n')

	# Line 1 should be "R=ZZ/2[x,y,Degrees=>{{1},{1}}]"
	if f2.readline() != "R=ZZ/2[x,y,Degrees=>{{1},{1}}]\n":
		print("Warning: unexpected format of M2 file on line 1")

	# Need to ask Mike about how to deal with C2, since it doesn't appear in the sample Singular file.
	# For now, I'll treat it the same as C1.
	c2 = f2.readline()
	if c2[:7] != "C2=R^{{" || c2[-3:] != "}}\n":
		print("Warning: unexpected format of M2 file on line 2")
	c2_strings = split(c2[7:-3], "},{")
	for word in c2_strings:
		# remove the minus sign that M2 uses if the number is not 0
		f1.write(word if word == "0" else word[1:])
	f1.write("\n")

	# Do the same thing with C1.
	c1 = f2.readline()
	if c1[:7] != "C1=R^{{" || c1[-3:] != "}}\n":
		print("Warning: unexpected format of M2 file on line 3")
	c1_strings = split(c1[7:-3], "},{")
	for word in c1_strings:
		# remove the minus sign that M2 uses if the number is not 0
		f1.write(word if word == "0" else word[1:])
	f1.write("\n")

	# Lines 4 and 5 should be C0=R^{} and Id=promote(1,R), respectively.
	if f2.readline() != "C0=R^{}":
		print("Warning: unexpected format of M2 file on line 4")
	if f2.readline() != "Id=promote(1,R)":
		print("Warning: unexpected format of M2 file on line 5")

	# Collect the data to make d2. We do this by extracting all the ordered pairs first, since 
	# the columns of d2 are identified one at a time.
	f1.write("module d2;\n")
	d2 = f2.readline()
	if d2[:14] != "d2=map(C1,C2,{" || d2[-4:] != "});\n":
		print("Warning: unexpected format of M2 file on line 6")
	
	# TODO: Figure out how to do d2 and d1

	# Almost done! We don't care about the rest of the M2 file (which doesn't contain information we need).
	f2.close()
	f1.write("module triv=0;\n"
	f1.write("def H=homology(d2,d1,triv,triv);\n")
	f1.write("resolution resH=res(H,0);")
	f1.write('print(betti(resH), "betti");')
	f1.close()

