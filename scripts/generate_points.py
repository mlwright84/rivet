# This should be run only once -- to generate the various files of random noisy circle points.

import os

sizes = [25, 50, 100, 200, 400, 800]

for n in sizes:
	os.system("python3 points_noisy_circle.py %i", % n)