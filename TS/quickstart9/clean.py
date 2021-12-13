f = open("chinese.txt", "r")
zh_lines = f.readlines()
f.close()
f = open("english.txt", "r")
en_lines = f.readlines()
f.close()

f_zh = open("chinese_e.txt", "w")
f_en = open("english_e.txt", "w")

for i in range(len(zh_lines)):
    if 0 == len(zh_lines[i].strip().split()) or 0 == len(en_lines[i].strip().split()):
        continue
    f_zh.write(zh_lines[i])
    f_en.write(en_lines[i])

f_zh.close()
f_en.close()
