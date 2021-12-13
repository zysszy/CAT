f = open("./en_mu.txt")
enlines = f.readlines()
f.close()

enlines = [enline.strip() for enline in enlines]

f = open("./LookUpTable.txt")
lines = f.readlines()
f.close()

dic = {}
for t in range(0, len(lines), 2):
    dic[lines[t].strip()] = lines[t + 1].strip()

f = open("./f_en_mu.zh.beam", "w")
ff = open("./candidata.en", "w")
for line in enlines:
    if line not in dic:
        ff.write(line + "\n")
        continue
    f.write(dic[line] + "\n")
f.close()
ff.close()
