f = open("./en_mu.txt")
enlines = f.readlines()
f.close()

enlines = [enline.strip() for enline in enlines]

f = open("./Mu.google.zh")
lines = f.readlines()
f.close()

dic = {}
for t in range(0, len(lines), 2):
    dic[lines[t].strip()] = lines[t + 1].strip()

f = open("./f_en_mu.zh.beam", "w")
for line in enlines:
    f.write(dic[line] + "\n")
f.close()
