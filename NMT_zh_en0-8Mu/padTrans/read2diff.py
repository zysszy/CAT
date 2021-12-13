f = open("./f_en_mu.zh.beam", "r")
zhlines = f.readlines()
f.close()

with open("./en_mu.txt", "r") as f:
    enlines = f.readlines()

f_Mut_en = open("Com_Original.en", "w")
f_Ori_zh = open("Com_Mutated.zh", "w")
f_Mut_zh = open("Com_Original.zh", "w")
f_Ori_en = open("Com_Mutated.en", "w")
f_Ori_O = open("./Com_oracle.zh", "w")


for i in range(len(enlines)):
    en = enlines[i].strip() + "\n"
    zh = " ".join(zhlines[i].strip().split("\t")[0].split()) + "\n"
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
    
