# Run the Singular files

import os

for file_name in os.listdir():
	if file_name.endswith("_res.sing"):
		num_pts = int(file[13:16])
		homol = int(file[-24:-23])
		with open("singular_res_times.txt", "a") as write_file:
			write_file.write("homol%d pts%d " % (homol, num_pts))
	elif file_name.endswith("_sres.sing"):
		num_pts = int(file[13:16])
		homol = int(file[-25:-24])
		with open("singular_sres_times.txt", "a") as write_file:
			write_file.write("homol%d pts%d " % (homol, num_pts))
	os.system("Singular %s" % file_name)