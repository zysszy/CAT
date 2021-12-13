import os
import threading

done = [0] * 11
def sayhi(i):
    os.system(f"cp quickstart0/low.py quickstart{i}")#&& sh do.sh && python3 read.py")
    os.system(f"cp quickstart0/do.sh quickstart{i}")#&& sh do.sh && python3 read.py")
    os.system(f"cd quickstart{i} && sh do.sh && python3 read.py")
    done[i] = 1

with open("./quickstart0/english_e.txt") as f:
    enlines = f.readlines()

with open("./quickstart0/chinese_e.txt") as f:
    zhlines = f.readlines()

assert len(enlines) == len(zhlines)

depth = len(enlines) // 10 + 1

for i in range(1, 11):
    with open(f"./quickstart{i}/chinese_e.txt", "w") as f:
        for t in range(depth * (i - 1), min(len(zhlines), depth * i)):
            f.write(zhlines[t])
    
    with open(f"./quickstart{i}/english_e.txt", "w") as f:
        for t in range(depth * (i - 1), min(len(enlines), depth * i)):
            f.write(enlines[t])

    t=threading.Thread(target=sayhi,args=(i, ))
    t.start()

last = -1
while True:
    sum = 0 
    for t in done:
        sum += t
    if sum > last:
        last = sum
        if sum >= 10:
            break
        print ("now processing(done):" + str(sum))

ffinal = open("./quickstart0/alignment_e.txt", "w")

for i in range(1, 11):
    with open(f"./quickstart{i}/alignment_e.txt") as f:
        lines = f.readlines()
    for t in range(len(lines)):
        ffinal.write(lines[t])

ffinal.close()
