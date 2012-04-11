def addlimits(filename):
	f = open(filename, 'r')
	lines = f.read().splitlines()
	lines[lines.index('')] = '#include <climits>  //added by Zhou\n'
	f = open(filename, 'w+')
	for line in lines:
		f.write(line + '\n')
	f.close()

foo = open('limits.txt', 'r')
for line in foo:
	addlimits(line[:-1])

