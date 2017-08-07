# This script generates a point-cloud input file for RIVET.
# The point-cloud is that of a "noisy circle" with a codensity function.

import random
import math
import sys
import os

total_pts = int(sys.argv[1])
proportion_circle = .9
radius = 2           #mean radius of the annulus
std_dev = 0.3        #standard deviation used for the radius of points selected from the annulus
ball_radius = 1      #radius of ball around each point for calculating values of the density function
rips_dist = 1000     #max distance for building the Rips complex (printed for the RIVET input file)
num_files = 2       #make several samples

#number of "data" points to be sampled roughly from the annulus
circle_pts = int (proportion_circle * total_pts)
#number of "noise" points to be sampled from a uniform distribution
noise_pts = total_pts - circle_pts     

for n in range(num_files):
	file_name = "noisy_circle_%i_pts_%i.txt" % (n, total_pts)
	f = open(file_name, "w")

	#print file header info
	f.write("# %s points from a noisy circle, %s points from a uniform distribution\n" %(circle_pts, noise_pts))
	f.write("# Parameters: mean radius %s, std. dev. radius %s, radius for codensity function %s\n" %(radius, std_dev, ball_radius))
	f.write("points\n")
	f.write("2\n")
	f.write("%s\n" %rips_dist)

	#arrays to hold points and density values
	points = []
	densities = []

	#generate points from the noisy circle
	for i in range (circle_pts):
		r = random.normalvariate(radius, std_dev)
		t = random.uniform(0, 360)
		x = r*math.cos(t)
		y = r*math.sin(t)
		points.append((x,y))

	#generate noise points from a uniform distribution
	max_val = 3*radius
	min_val = -3*radius
	for i in range (noise_pts):
		x = random.uniform(min_val, max_val)
		y = random.uniform(min_val, max_val)
		points.append((x,y))

	#compute density values for each point
	d2 = ball_radius**2
	for p in points:
		count = 0
		for q in points:
			d = (p[0] - q[0])**2 + (p[1] - q[1])**2
			if(d < d2):
				count += 1
		densities.append(count)

	#find max density value, from which we will subtract the individual densities to 
	#obtain positive values for codensity
	max_density = max(densities) + 1

	#print the points and codensities to a file
	for i, p in enumerate(points):
		codensity = max_density - densities[i]
		f.write("%s\t%s\t%s\n" %("{:.2f}".format(p[0]), "{:.2f}".format(p[1]), codensity) )

	print("Finished writing to %s" %file_name)
	f.close()
	os.system("mv %s ../betti_tests/%s" % (file_name, file_name))
