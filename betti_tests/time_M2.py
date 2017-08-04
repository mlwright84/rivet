# Get timings for all the M2 files that are about noisy circles

import os

for r, d, files in os.walk("./"):
	for file in files:
		if file.startswith("noisy_circle_") and file.endswith(".m2"):
			words = file.split('_')
			num_pts = int(words[2])
			prime = int(words[6])
			homol = int(words[8])
			# Write homology, num pts, and prime field to the files now, since we have access to file names
			if file.endswith("bigraded.m2"):
				with open("bigraded_m2_times.txt", "a") as write_file:
					write_file.write("homol%d pts%d mod%d " % (homol, num_pts, prime))

			elif file.endswith("singly_graded.m2"):
				with open("singly_graded_m2_times.txt", "a") as write_file:
					write_file.write("homol%d pts%d mod%d " % (homol, num_pts, prime))

			elif file.endswith("minimal_betti.m2"):
				with open("minimal_betti_m2_times.txt", "a") as write_file:
					write_file.write("homol%d pts%d mod%d " % (homol, num_pts, prime))

			# Get timing data
			os.system("M2 < %s" % os.path.join(r, file))

