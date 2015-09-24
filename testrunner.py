#!/usr/bin/python

import sys
import subprocess
import json

failures=0

# JSON format:
# {
#   "prog": "./myprogram",
#   "tests": [
#     [ "test name",
#       [ "arg1", "arg2", ... ],
#       "input data",		  <-- default: empty
#       "expected stdout output", <-- default: empty
#       "expected stderr output", <-- default: empty
#       0			  <-- expected return code (default: zero)
#     ]
#   ]
# }
#

def runtest(prog, testparams):
	global failures

	test_name = testparams[0]
	args = [ prog ]
	args = args + testparams[1]

	in_data = ""
	if len(testparams) > 2:
		in_data = testparams[2]
	wanted_out = ""
	if len(testparams) > 3:
		wanted_out = testparams[3]
	wanted_err = ""
	if len(testparams) > 4:
		wanted_err = testparams[4]
	wanted_returncode = 0
	if len(testparams) > 5:
		wanted_returncode = testparams[5]

	p = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	(out_data, err_data) = p.communicate(in_data)

	if out_data != wanted_out:
		print >>sys.stderr, "FAIL " + test_name + " stdout-invalid"
		failures = failures + 1
	if err_data != wanted_err:
		print >>sys.stderr, "FAIL " + test_name + " stderr-invalid"
		failures = failures + 1
	if p.returncode != wanted_returncode:
		print >>sys.stderr, "FAIL " + test_name + " retcode-invalid"
		failures = failures + 1

def runtests(d):
	prog = d['prog']
	for testparams in d['tests']:
		runtest(prog, testparams)

if __name__ == '__main__':
	if len(sys.argv) != 2:
		print "Usage: testrunner.py JSON-CONFIG-FILE"
		sys.exit(1)

	with open(sys.argv[1]) as json_data:
	    d = json.load(json_data, strict=False)

	    runtests(d)

	if failures > 0:
		sys.exit(1)

	sys.exit(0)

