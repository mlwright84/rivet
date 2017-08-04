# This script works assuming that the command:
#	~/rivet/scripts$ python3 generate_points.py
# has been run. This script contains the rest of the timing pipeline; running it will
# produce the necessary Macaulay2 and Singular files, and perform all the timing computations.

import os
primes = [2, 7, 13] # prime fields to use

# We now have a bunch of text files with RIVET data. The next step collects the timing data for 
# RIVET, as well as produce the corresponding Macaulay2 files.
for prime in primes:
	os.system("python3 make_M2_files.py %d" % prime)

# Some Macaulay2 files have trivial components when computed using 0th homology.
# This script simplifies the files where appropriate.
os.system("python3 reduce_0_homol.py")

# Produce the Singular files, using the contents of the Macaulay2 files
os.system("python3 singly_graded/m2_to_singular.py")

# This script runs all of the Macaulay2 files and collects the timing data for them.
os.system("python3 time_M2.py")

# This script runs all of the Singular files and collects the timing data for them.
os.system("python3 time_Singular.py")

# Assemble some statistics
os.system("python3 statistics.py")