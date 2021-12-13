import re

f = open("english.txt", "r")
lines = f.readlines()
f.close()

def token(line):
    #code = re.sub(r'([^A-Za-z0-9_.\-%])', r' \1 ', code)
    return re.sub(r"([\"$])", r" \1 ", re.sub("([[\s+\.\!\/_,%^*(+\"\']+|[+——！，。？、~@#￥%……&*（）]+])", r" \1",line))

f = open("english_e.txt", "w")
for line in lines:
    site = token(line)
    print (token(line))
    f.write(site.replace("  ", " ").strip().lower() + "\n")
f.close()
