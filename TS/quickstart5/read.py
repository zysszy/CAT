f = open("chinese_e.txt")
zh = f.readlines()
f.close()

f = open("english_e.txt")
en = f.readlines()
f.close()

assert len(zh) == len(en)

f = open("alignment_e.txt")
align = f. readlines()
f.close()

f = open("word_alignment.txt", "w")

for i in range(len(zh)):
    print (i)
    dic = {}
    zh_words = zh[i].strip().split()
    en_words = en[i].strip().split()
    align_words = align[i].strip().split()
    for a in align_words:
        try:
            se = a.split("-")
            # print (zh_words[int(se[0])], en_words[int(se[1])])
            start = int(se[0])
            end = int(se[1])
            if en_words[end] not in dic:
                dic[en_words[end]] = [start]
            else:
                dic[en_words[end]].append(start)
        except:
            continue

    f.write(str(dic) + "\n")

f.close()
