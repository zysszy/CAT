import sys

f = open(sys.argv[1])
lines = f.readlines()
f.close()

f = open(sys.argv[2], "w")
for line in lines:
    f.write(line.replace(" ##", ""))
f.close()
