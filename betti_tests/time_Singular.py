# Run the Singular files

import os

for r, d, files in os.walk("./"):
	for file in files:
		if not file.endswith(".sing"):
			continue
		words = file.split('_')
		num_pts = int(words[2])
		prime = int(words[6])
		homol = int(words[8])
		if file.endswith("_res.sing"):
			with open("singular_res_times.txt", "a") as write_file:
				write_file.write("homol%d pts%d mod%d " % (homol, num_pts, prime))
		elif file.endswith("_sres.sing"):
			with open("singular_sres_times.txt", "a") as write_file:
				write_file.write("homol%d pts%d mod%d " % (homol, num_pts, prime))
		os.system("Singular < %s" % os.path.join(r, file))