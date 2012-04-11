def addassert(filename):
	f = open(filename, 'r')
	lines = f.read().splitlines()
	for l in lines:
		if l == '#include <cassert>':
			return
	lines[lines.index('')] = '#include <cassert>  //added by Zhou\n'
	f = open(filename, 'w+')
	for line in lines:
		f.write(line + '\n')
	f.close()

foo = open('asserts.txt', 'r')
for line in foo:
	addassert(line[:-1])

