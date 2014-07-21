# sleep.py
# Will count up to a passed argument, incrementing
# every second.

import time
import sys

if len(sys.argv) < 2:
	print ("Missing an argument")
else:
	for x in range(0, int(sys.argv[1])):
		print ("We're on time ", x)
		time.sleep(1)
