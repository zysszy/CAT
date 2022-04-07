import sys

with open(sys.argv[1], "r") as f:
    lines = f.readlines()

with open(sys.argv[2], "w") as f:
    for line in lines:
        index = [int(i) for i in line.strip().split()]
        f.write(str(index[0] * 2 + index[1]) + "\n")
