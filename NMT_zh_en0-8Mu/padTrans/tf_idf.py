import collections
import math

f = open("idf_dic.txt", "r")
line = f.readline()
f.close()

idf = eval(line)

def compute_tfidf(line):
    words = line.strip().split()
    counter = collections.Counter()
    for word in words:
        #if word in [',', '.', ':', '"', "'", '', ' ', '', '。', '：', '，', '）', '（', '！', '?', '”', '“', "’", "‘", "；"]:
        #    continue
        counter[word] += 1

    ret_dic = {}
    for item in counter:
        search_index = item
        if item not in idf:
            item = "<unk>"

        ret_dic[search_index] = (counter[search_index] / len(words)) * idf[item]

    return ret_dic


def compute_cos(dic_a, dic_b):
    dot = 0
    l_a = 0
    l_b = 0
    for item in dic_a:
        l_a += dic_a[item] * dic_a[item]

    for item in dic_b:
        l_b += dic_b[item] * dic_b[item]
        if item in dic_a:
            dot += dic_a[item] * dic_b[item]

    #print (dic_a)
    #print (dic_b)
    #print (dot / math.exp(((math.log(l_a) / 2) + (math.log(l_b) / 2))))
    
    try:
        assert dot / math.exp(((math.log(l_a) / 2) + (math.log(l_b) / 2))) <= 1
    except:
        return 1
    return dot / math.exp(((math.log(l_a) / 2) + (math.log(l_b) / 2)))#dot / (math.sqrt(l_a) * math.sqrt(l_b))

if __name__ == "__main__":
    # how to use this two functions 
    
    f = open("test.txt")
    lines = f.readlines()
    f.close()

    top = 10
    for i in range(0, len(lines), 2):
        dic_a = compute_tfidf(lines[i])
        dic_b = compute_tfidf(lines[i + 1])
        compute_cos(dic_a, dic_b)
        
