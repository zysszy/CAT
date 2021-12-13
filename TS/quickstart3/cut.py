import jieba

f = open("chinese_e.txt", "r")
lines = f.readlines()
f.close()
 
f = open("chinese_e.txt", "w")
for line in lines:
    print ("".join(line.split()).strip())
    seg_list = jieba.cut(line.strip(), cut_all=False)
    f.write(" ".join(seg_list) + "\n")
    # print("Default Mode: " + "/ ".join(seg_list))  # 精确模式
    # exit()

f.close()
