# Delete all the timing files.

import os

timing_files = ["bigraded_m2_times.txt", "minimal_betti_m2_times.txt",
                "singly_graded_m2_times.txt", "singular_res_times.txt",
                "singular_sres_times.txt", "rivet_timing.txt"]

for file in os.listdir():
	if file in timing_files:
		os.system("rm %s" % file)