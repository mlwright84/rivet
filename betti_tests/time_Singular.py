# Get timings for all the noisy circle Singular files

import os

for r, d, files in os.walk("./"):
	for file in files:
		if not (file.startswith("noisy_circle_") and file.endswith(".sing")):
			continue
		words = file.split('_')
		num_pts = int(words[4])
		prime = int(words[6])
		homol = int(words[8])
		# Write homology, num pts, and prime field to the files now, since we have access to file names
		if file.endswith("_res.sing"):
			with open("singular_res_times.txt", "a") as write_file:
				write_file.write("homol%d pts%d mod%d " % (homol, num_pts, prime))

		elif file.endswith("_sres.sing"):
			with open("singular_sres_times.txt", "a") as write_file:
				write_file.write("homol%d pts%d mod%d " % (homol, num_pts, prime))

		# Get timing data
		os.system("Singular < %s" % os.path.join(r, file))