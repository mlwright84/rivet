# Assuming that random data using the other scripts has already been generated.

import os

# We work assuming that the command:
#	~/rivet/scripts$ python3 generate_points.py
# has been run. This generates 10 noisy circle files, with sizes specified in that file.

# We now have a bunch of RIVET files. We want to collect the timing data for the RIVET files,
# as well as produce the corresponding Macaulay2 files.
os.system("python3 make_M2_files.py")

# Produce the Singular files, using the contents of the Macaulay2 files
os.system("python3 singly_graded/m2_to_singular.py")

# Simplify the applicable M2 files -- for 0th homology, there are some tweaks that can shorten the code
# (and probably optimize it as well).
os.system("python3 reduce_0_homol.py")

# Collect the timing data for M2.
os.system("python3 time_M2.py")

# TODO: Collect the timing data for Singular.
os.system("python3 time_Singular.py")

# TODO: Assemble some statistics