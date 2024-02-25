#!/usr/bin/python3

import sys
import subprocess
import json

failures=0
srcdir=''

# JSON format:
# {
#   "prog": "./myprogram",
#   "tests": [
#     [ "test name",
#       [ "arg1", "arg2", ... ],
#       "input data",		  <-- default: empty
#       "expected stdout output", <-- default: empty
#       "expected stderr output", <-- default: empty
#       0,			  <-- expected return code (default: zero)
#	{ "in": "y", "out": "y" } <-- flags and further options
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

	test_opts = {}
	if len(testparams) > 6:
		test_opts = testparams[6]

	if 'in' in test_opts:
		with open(srcdir + '/' + in_data, 'r') as content_file:
			in_data = content_file.read()
	if 'out' in test_opts:
		with open(srcdir + '/' + wanted_out, 'r') as content_file:
			wanted_out = content_file.read()

	p = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	(out_data, err_data) = p.communicate(in_data.encode('utf-8'))

	ok=True

	if out_data.decode('utf-8') != wanted_out:
		print("FAIL " + test_name + " stdout-invalid", file=sys.stderr)
		failures = failures + 1
		ok=False
	if err_data.decode('utf-8') != wanted_err:
		print("FAIL " + test_name + " stderr-invalid", file=sys.stderr)
		failures = failures + 1
		ok=False
	if p.returncode != wanted_returncode:
		print("FAIL " + test_name + " retcode-invalid", file=sys.stderr)
		failures = failures + 1
		ok=False

	if ok:
		print("OK " + test_name, file=sys.stderr)

def runtests(d):
	prog = d['prog']
	for testparams in d['tests']:
		runtest(prog, testparams)

if __name__ == '__main__':
	if len(sys.argv) != 3:
		print("Usage: testrunner.py TEST-DATA-DIR JSON-CONFIG-FILE")
		sys.exit(1)

	srcdir=sys.argv[1]
	cfgfn=sys.argv[2]
	fn = srcdir + '/' + cfgfn

	with open(fn) as json_data:
	    d = json.load(json_data, strict=False)

	    runtests(d)

	if failures > 0:
		sys.exit(1)

	sys.exit(0)

