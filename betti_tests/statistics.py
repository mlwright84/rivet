# Collect some statistics about the timing data.

import os
import sys

sys.stdout = open("preliminary_times.txt", "w")

with open("rivet_timing.txt", "r") as read_file:
	print("RIVET")
	times = {}
	for line in read_file:
		homol = int(line[5:6])
		pts = int(line[10:13])
		bifilt_time = float(line.split()[2])
		betti_time = float(line.split()[3])
		key = (homol, pts)
		if key in times:
			(a, b, n) = times[key]
			times[key] = (a + bifilt_time, b + betti_time, n + 1)
		else:
			times[key] = (bifilt_time, betti_time, 1)
	for key in times:
		(a, b, n) = times[key]
		print("For homology %d with %d points: " % key)
		print("\taverage bifiltration took %f seconds, betti computation took %f seconds." 
			% (a/(1000*n), b/(1000*n)))
	print()

for grade in ["singly_graded", "bigraded"]:
	with open("%s_m2_times.txt" % grade, "r") as read_file:
		print("Macaulay2, %s" % grade)
		times = {}
		for line in read_file:
			homol = int(line[5:6])
			pts = int(line.split()[1][3:])
			coker_or_homol_time = float(line.split()[2])
			prune_time = float(line.split()[3])
			betti_time = float(line.split()[4])
			elements_time = float(line.split()[5])
			key = (homol, pts)
			if key in times:
				(a, b, c, d, n) = times[key]
				times[key] = (a + coker_or_homol_time, b + prune_time, c + betti_time, d + elements_time, n + 1)
			else:
				times[key] = (coker_or_homol_time, prune_time, betti_time, elements_time, 1)
		for key in times:
			(a, b, c, d, n) = times[key]
			print("For homology %d with %d points: " % key)
			if key[0] == 0:
				print("\t average cokernel computation took %f seconds, prune computation took %f seconds;"
					% (a/n, b/n))
			else:
				print("\t average homology computation took %f seconds, prune computation took %f seconds;"
					% (a/n, b/n))
			print("\t average betti computation took %f seconds, elements computation took %f seconds."
				% (c/n, d/n))
	print()

with open("minimal_betti_m2_times.txt", "r") as read_file:
	print("Macaulay2, minimal_betti")
	times = {}
	for line in read_file:
		homol = int(line[5:6])
		pts = int(line[10:13])
		coker_or_homol_time = float(line.split()[2])
		minimal_betti_time = float(line.split()[3])
		elements_time = float(line.split()[4])
		key = (homol, pts)
		if key in times:
			(a, b, c, n) = times[key]
			times[key] = (a + coker_or_homol_time, b + minimal_betti_time, c + elements_time, n + 1)
		else:
			times[key] = (coker_or_homol_time, minimal_betti_time, elements_time, 1)
	for key in times:
		(a, b, c, n) = times[key]
		print("For homology %d with %d points: " % key)
		if key[0] == 0:
			print("\t average cokernel computation took %f seconds, minimal_betti computation took %f seconds;"
				% (a/n, b/n))
		else:
			print("\t average homology computation took %f seconds, minimal_betti computation took %f seconds;"
				% (a/n, b/n))
		print("\t average elements computation took %f seconds."
			% (c/n))



for fun in ["res", "sres"]:
	with open("singular_%s_times.txt" % fun, "r") as read_file:
		print("Singular, %s" % fun)
		times = {}
		for line in read_file:
			homol = int(line[5:6])
			pts = int(line.split()[1][3:])
			betti_time = float(line.split()[-1])
			resolution_time = float(line.split()[-2])
			key = (homol, pts)
			if key in times:
				if homol == 0:
					(a, b, n) = times[key]
					times[key] = (a + betti_time, b + resolution_time, n + 1)
				else:
					(a, b, c, n) = times[key]
					# line.split()[-3] gives the homology time
					times[key] = (a + betti_time, b + resolution_time, c + float(line.split()[-3]), n + 1)
			else:
				if homol == 0:
					times[key] = (betti_time, resolution_time, 1)
				else:
					times[key] = (betti_time, resolution_time, float(line.split()[-3]), 1)
		for key in times:
			print("For homology %d with %d points: " % key)
			if key[0] == 0:
				(a, b, n) = times[key]
				print("\t average resolution computation took %f seconds, betti computation took %f seconds."
					% (b/(1000*n), a/(1000*n)))
			else:
				(a, b, c, n) = times[key]
				print("\t average homology computation took %f seconds;" % (c/(1000*n)))
				print("\t average resolution computation took %f seconds, betti computation took %f seconds."
					% (b/(1000*n), a/(1000*n)))

sys.stdout.close()