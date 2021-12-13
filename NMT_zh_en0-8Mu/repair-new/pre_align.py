from copy import deepcopy
import sys

f = open(sys.argv[1], "r")
zh_lines = f.readlines()
f.close()

f = open(sys.argv[2], "w")
for i in range(len(zh_lines)):
    line = zh_lines[i].strip().split("\t")
    zh = line[0]
    f.write(zh + "\n")# + str(line[1]) + "\n")
f.close()

