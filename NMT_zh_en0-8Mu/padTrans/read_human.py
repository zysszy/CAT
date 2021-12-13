import sys
import os
import math
import collections

def tobool(s):
    if s == "True":
        return True
    else:
        return False

filter_list = []
#with open("../../NMT_zh_en0/google/human_google/train.txt") as f:
#    lines = f.readlines()
#    f.close()
#    now_count_all = 0
#    for i in range(0, len(lines), 11):
#        now_count_all += 1
#        if now_count_all == 45:
#            continue
#        filter_list.append(lines[i + 6].strip())
#        if now_count_all >= 100:
#            break

t_out_list = [[] for t in range(4)]

ground_new = [{} for i in range(9)]

sum = [0] * 8

filter_count = 0
with open("Com_ALL.txt", "r") as f:
    lines = f.readlines()
    count_data = len(lines) // 14
    for i in range(0, len(lines), 14):
        vote = 0
        if lines[i + 9].strip() in filter_list:
            filter_count += 1
            continue
        for t in range(8):
            #ground_new[t][lines[i + 12].strip()] = (tobool(lines[i + t].strip().split()[-1]))
            length = 1#float(max(len(lines[i + 9].split()), len(lines[i + 11].split())))
            #length = math.sqrt(length)
            if t < 4:
                #continue
                delta = float(lines[i + t].strip().split()[1])
                sc1 = float(lines[i + t].strip().split()[2])
                sc2 = float(lines[i + t].strip().split()[3])
                if t != 3:
                    delta *= length
                    sc1 *= length
                    sc2 *= length
                #length = len()
                if delta >= max(sc1, sc2) * [0.05, 0.08, 0.06, 0.08][t]:
                    ground_new[t][lines[i + 9].strip()] = True#(tobool(lines[i + t].strip().split()[-1]))
                else:
                    ground_new[t][lines[i + 9].strip()] = False
                if ground_new[t][lines[i + 9].strip()] == False:
                    vote += 0.5
                sum[t] += float(delta)
            else:
                #sum[t] += 1 - float(lines[i + t].strip().split()[1])
                if t == 7:
                    score = float(lines[i + t].strip().split()[1])
                    if float(lines[i + t].strip().split()[1]) < 0.906:
                        ground_new[t][lines[i + 9].strip()] = True
                    else:
                        ground_new[t][lines[i + 9].strip()] = False
                        vote += 1
                else:
                    score = float(lines[i + t].strip().split()[1]) * length
                    if score < [0.963, 0.963, 0.999][t - 4]:
                        ground_new[t][lines[i + 9].strip()] = True
                    else:
                        ground_new[t][lines[i + 9].strip()] = False
                        vote += 1
                sum[t] += 1 - score

        if vote >= 3:
            ground_new[8][lines[i + 9].strip()] = False
        else:
            ground_new[8][lines[i + 9].strip()] = True

#count_data = count
#print (count_data)
#print (count)
#assert count_data == count
#assert len(ground_new[0]) == len(ground)

index = ["LCS-O", "ED-O", "TF-O", "BLEU-O", "LCS", "ED", "TF", "BLEU", "Vote"]
out_list = [[] for i in range(9)]
dic = {}
dic[True] = "P"
dic[False] = "N"
#dic[tuple([False])] = "TN"
#dic[tuple([False])] = "FN"
#dic[tuple([True])] = "FP"
#dic[tuple([True])] = "TP"

count = [collections.Counter() for i in range(9)]
#sum = [0] * 8

print (filter_count)
#assert filter_count == 99

count_all = 0
for i in ground_new[0]:
    for t in range(9):
        tp = ground_new[t][i]#tuple([ground[i], ground_new[t][i]])
        out_list[t].append(tp)
        count[t][tp] += 1
        if t == 0:
            count_all += 1
#print (count_data)
#print (count_all)
#assert count_all == count_data
for item in dic:
    print (dic[item])
f = open("out.answer", "w")
for i in range(9):
    f.write(index[i] + "")
    for item in dic:
        f.write(" & ")
        f.write(str(count[i][item]) + " (" + str(round(float(count[i][item]) / count_all * 100, 2)) + "\\%) ")
    f.write("\\\\\n")
f.close()

for s in sum:
    print (s / count_data)
