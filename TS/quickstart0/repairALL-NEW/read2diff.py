
def do(met, prob=""):
    f = open(f"./{met}_Repaired.{prob}zh", "r")
    zhlines = f.readlines()
    f.close()

    with open(f"./bugs_{met}.txt", "r") as f:
        enlines = f.readlines()

    f_Mut_en = open(f"Com_Original_{met}.{prob}en", "w")
    f_Ori_zh = open(f"Com_Mutated_{met}.{prob}zh", "w")
    f_Mut_zh = open(f"Com_Original_{met}.{prob}zh", "w")
    f_Ori_en = open(f"Com_Mutated_{met}.{prob}en", "w")
    f_Ori_O = open(f"./Com_oracle_{met}.{prob}zh", "w")


    for i in range(len(enlines)):
        en = enlines[i].strip() + "\n"
        zh = zhlines[i].strip() + "\n" #" ".join(zhlines[i].strip().split("\t")[0].split()[:-1]) + "\n"
        if i % 2 == 0:
            f_Ori_en.write(en)
            f_Ori_zh.write(zh)
        elif i % 2 == 1:
            f_Mut_en.write(en)
            f_Mut_zh.write(zh)
            f_Ori_O.write(zh)

    f_Mut_en.close()
    f_Mut_zh.close()
    f_Ori_zh.close()
    f_Ori_en.close()
    f_Ori_O.close()

met = ["LCS", "ED", "TFIDF", "BLEU"]
prob = ["", "prob."]

[do(m, p) for m in met for p in prob]
        
