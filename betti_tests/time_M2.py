# Get timings for all the M2 files that are about noisy circles

import os

for r, d, files in os.walk("./"):
	for file in files:
		if file.startswith("noisy_circle_") and file.endswith(".m2"):
			# Write homology and num pts to the files now, since we have access to file names
			if file.endswith("bigraded.m2"):
				num_pts = int(file[13:16])
				homol = int(file[-13:-12])
				with open("bigraded_m2_times.txt", "a") as write_file:
					write_file.write("homol%d pts%d " % (homol, num_pts))
			elif file.endswith("singly_graded.m2"):
				num_pts = int(file[13:16])
				homol = int(file[-18:-17])
				with open("singly_graded_m2_times.txt", "a") as write_file:
					write_file.write("homol%d pts%d " % (homol, num_pts))
			elif file.endswith("minimal_betti.m2"):
				num_pts = int(file[13:16])
				homol = int(file[-18:-17])
				with open("minimal_betti_m2_times.txt", "a") as write_file:
					write_file.write("homol%d pts%d " % (homol, num_pts))
			# Get timing data
			os.system("M2 < %s" % os.path.join(r, file))

