ff = open("candidata.en", "w")


def run(source, target):
    f = open(source)
    enlines = f.readlines()
    f.close()

    enlines = [enline.strip() for enline in enlines]

    f = open("./LookUpTable.txt")
    lines = f.readlines()
    f.close()

    dic = {}
    for t in range(0, len(lines), 2):
        dic[lines[t].strip()] = lines[t + 1].strip()

    f = open(target, "w")
    for line in enlines:
        if line not in dic:
            ff.write(line + "\n")
            continue
        f.write(dic[line] + "\n")
    f.close()

run("ED_Mu.en", "ED_Mu.zh")
run("LCS_Mu.en", "LCS_Mu.zh")
run("TFIDF_Mu.en", "TFIDF_Mu.zh")
run("BLEU_Mu.en", "BLEU_Mu.zh")
ff.close()
