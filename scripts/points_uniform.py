# This script generates a point-cloud input file for RIVET.
# Points are chosen IID uniformly in a rectangle [0,max_x)x[0,max_y).
# A real-valued function on each point is also chosen IID uniformly in an interval [0,max_f).

import random

n = 50      #number of points
max_x = 10   #x-coordinates uniform between 0 and max_x
max_y = 10   #y-coordinates uniform between 0 and max_y
max_f = 20   #function values uniform between 0 and max_f
dist = 1000     #max distance for building the Rips complex (printd for the RIVET input file)
num_files = 10       #make several samples


for i in range(num_files):
	file_name = "uniform_%i.txt" % i
	file = open(file_name, "w")

	#print file header info
	file.write("# %s points chosen IID uniformly\n" %(n))
	file.write("points\n")
	file.write("2\n")
	file.write("%s\n" %dist)

	#now generate and print the points
	for i in range (n):
		x = random.uniform(0, max_x)
		y = random.uniform(0, max_y)
		f = random.uniform(0, max_f)
		file.write( "%s\t%s\t%s\n" %("{0:.2f}".format(x), "{0:.2f}".format(y), "{0:.2f}".format(f)) )

	print("Finished writing to %s" %file_name)
	file.close()
