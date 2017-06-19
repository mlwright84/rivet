# This script converts the output of elements(Betti_Nums) from Macaulay2 to a RIVET-style list of \xi support points

import sys

#check arguments
if(len(sys.argv) < 2):
	print("  usage: convert_output.py [name of file containing elements(Betti_Nums) output]")
	sys.exit()

#read the entire file
file = open(sys.argv[1], 'r')
input = file.read()

#convert the Macaulay2 output to a single line, and remove hyphens, indentation, and curly braces
input = input.translate(None, '- {}\n')

#the string input now contains a comma-separated list of numbers; every four numbers give a \xi support point
#convert the string to a list of 4-tuples (which will still be strings)
input = input[1:-1]
tuples = input.split('),(')

#declare lists that will keep track of the support points
xi0 = []
xi1 = []
xi2 = []

#process the list of numbers
for tup in tuples:
	nums = tup.split(',')
	i = int(nums[0])
	x = int(nums[1])
	y = int(nums[2])
	if(i == 0):
		xi0.append((x,y))
	elif(i == 1):
		xi1.append((x,y))
	elif(i == 2):
		xi2.append((x,y))

#sort lists lexicographically
xi0.sort()
xi1.sort()
xi2.sort()

#define a function to print these lists in the format we want
def print_xi( xi_list ):
	x = -1
	y = -1
	m = 0
	for point in xi_list:
		if(point[0] == x and point[1] == y):	#increment the multiplicity counter
			m += 1
		else:					#print previous xi point and remember new coordinates
			if(x != -1):
				print "  (", x, ",", y, ")x", m
			x = point[0]
			y = point[1]
			m = 1
	#print last xi point
	print "  (", x, ",", y, ")x", m

#output
print "xi_0 support points:"
print_xi(xi0)
print "xi_1 support points:"
print_xi(xi1)
print "xi_2 support points:"
print_xi(xi2)

