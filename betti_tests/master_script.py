# Assuming that random data using the other scripts has already been generated.

import os

# Make the M2 files and collect the timing data for RIVET
os.system("python3 make_M2_files.py")

# Produce the Singular files
os.system("python3 singly_graded/m2_to_singular.py")

# Simplify the applicable M2 files
os.system("python3 reduce_0_homol.py")

# Collect the timing data for M2
for r, d, files in os.walk("./"):
	for file in files:
		if file.startswith("noisy_circle_") and file.endswith(".m2"):
			os.system("M2 << %s" % file)

