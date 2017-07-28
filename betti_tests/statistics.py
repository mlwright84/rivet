# Collect some statistics about the timing data.

import os
import sys
import csv

output = open("timings.csv", "w")
writer = csv.writer(output, delimiter = ",")
rivet_header = ["RIVET", "homology", "points", "bifiltration time", "betti time"]
macaulay_single_header = ["Macaulay2, singly graded", "homology", "points", "coker/homol time",
	"prune time", "betti time", "display elements time"]
macaulay_bi_header = ["Macaulay2, bigraded", "homology", "points", "coker/homol time",
	"prune time", "betti time", "display elements time"]
macaulay_minimal_header = ["Macaulay2, minimal_betti", "homology", "points", "coker/homol time",
	"display elements time"]
singular_res_header = ["Singular, res", "homology", "points", "homol time", "resolution time", "betti time"]
singular_sres_header = ["Singular, sres", "homology", "points", "homol time", "resolution time", "betti time"]


with open("rivet_timing.txt", "r") as read_file:
	writer.writerow(rivet_header)
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
		writer.writerow(["", key[0], key[1], a/(1000*n), b/(1000*n)])

for grade in ["singly_graded", "bigraded"]:
	with open("%s_m2_times.txt" % grade, "r") as read_file:
		if grade == "singly_graded":
			writer.writerow(macaulay_single_header)
		else:
			writer.writerow(macaulay_bi_header)
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
			writer.writerow([" ", key[0], key[1], a/n, b/n, c/n, d/n])

with open("minimal_betti_m2_times.txt", "r") as read_file:
	writer.writerow(macaulay_minimal_header)
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
		writer.writerow([" ", key[0], key[1], a/n, b/n, c/n])

for fun in ["res", "sres"]:
	with open("singular_%s_times.txt" % fun, "r") as read_file:
		if fun == "res":
			writer.writerow(singular_res_header)
		else:
			writer.writerow(singular_sres_header)
		times = {}
		for line in read_file:
			homol = int(line[5:6])
			pts = int(line.split()[1][3:])
			betti_time = float(line.split()[-1])		#These are picked up backwards
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
			if key[0] == 0:
				(a, b, n) = times[key]
				writer.writerow([" ", key[0], key[1], 0, b/(1000*n), a/(1000*n)])
			else:
				(a, b, c, n) = times[key]
				writer.writerow([" ", key[0], key[1], c/(1000*n), b/(1000*n), a/(1000*n)])

output.close()