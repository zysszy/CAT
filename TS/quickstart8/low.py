f = open("english_e.txt", "r")
lines = f.readlines()
f.close()

f = open("english_e.txt", "w")
for line in lines:
    f.write(line.lower())

f.close()
