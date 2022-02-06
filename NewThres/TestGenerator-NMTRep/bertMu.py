import nltk
from tqdm import tqdm
import numpy as np
import string
import math
import torch
import sys
import time
import torch.nn.functional as F
# from stanfordcorenlp import StanfordCoreNLP
import os
import random
from copy import deepcopy
#from pytorch_pretrained_bert import BertTokenizer, BertModel, BertForMaskedLM
from transformers import BertConfig, BertTokenizer, BertModel, RobertaTokenizer, RobertaModel, BertForMaskedLM
# from nltk.tokenize.treebank import TreebankWordTokenizer, TreebankWordDetokenizer

os.environ["CUDA_VISIBLE_DEVICES"]="5"

K_Number = 100
Max_Mutants = 16

ft = time.time()
#tokenizer = TreebankWordTokenizer()
#detokenizer = TreebankWordDetokenizer()

#nlp = StanfordCoreNLP("stanford-corenlp-full-2018-02-27", port=34139, lang="en")

def check_tree (ori_tag, line):
    tag = line.strip()
    tag = nlp.pos_tag(tag)
    #print (tag)
    #print (ori_tag)
    #print ("-----------------")
    if len(tag) != len(ori_tag):
        return False
    for i in range(len(tag)):
        if tag[i][1] != ori_tag[i][1]:
            return False
    return True


def bertInit():
    #config = Ber
    berttokenizer = BertTokenizer.from_pretrained('bert-large-cased')
    bertmodel = BertForMaskedLM.from_pretrained("bert-large-cased")#'/data/szy/bertlarge')
    bertori = BertModel.from_pretrained("bert-large-cased")#'/data/szy/bertlarge')
    #berttokenizer = RobertaTokenizer.from_pretrained('bert-large-uncased')
    #bertmodel = RoBertaForMaskedLM.from_pretrained('/data/szy/bertlarge')
    bertmodel.eval().cuda()#.to(torch.device("cuda:0"))
    bertori.eval().cuda()#.to(torch.device("cuda:1"))
    
    return bertmodel, berttokenizer, bertori

# tokenizer = TreebankWordTokenizer()

lcache = []

def BertM (bert, berttoken, inpori, bertori):
    global lcache
    for k in lcache:
        if inpori == k[0]:
            return k[1], k[2]
    sentence = inpori
    #batchsize = 3000 // len(tokens)
#    oritokens = tokenizer.tokenize(sentence)
    tokens = berttoken.tokenize(sentence)
    batchsize = 4000 // len(tokens)
    #tag = nlp.pos_tag(" ".join(oritokens))
    gen = []
    ltokens = ["[CLS]"] + tokens + ["[SEP]"]
    #oriencoding = bertori(torch.tensor([berttoken.convert_tokens_to_ids(ltokens)]).cuda())[0][0].data.cpu().numpy()
    #print ([ltokens[0:i] + ["[MASK]"] + ltokens[i + 1:] for i in range(1, len(ltokens) - 1)])
    try:
        encoding = [berttoken.convert_tokens_to_ids(ltokens[0:i] + ["[MASK]"] + ltokens[i + 1:]) for i in range(1, len(ltokens) - 1)]#.cuda()
    except:
        return " ".join(tokens), gen
    p = []
    for i in range(0, len(encoding), batchsize):
        tensor = torch.tensor(encoding[i: min(len(encoding), i + batchsize)]).cuda()
        pre = F.softmax(bert(tensor)[0], dim=-1).data.cpu()
        p.append(pre)
    pre = torch.cat(p, 0)
    #print (len(pre), len())
    #topk = torch.topk(pre[i][i + 1], K_Number)#.tolist()
    tarl = [[tokens, -1]]
    for i in range(len(tokens)):
        #i = 1
        #if tag[i][1] not in ["NNS", "JJ", "NN", "NNP", "NNPS", "CD", "NNPS"]:i
        #    continue
        if tokens[i] in string.punctuation:
            continue
        #token = tokens[i]
        #print (tokens)
        #ltokens = [x.lower() for x in tokens]
        #ltokens[i] = '[MASK]'
        #ltokens = ['[CLS]'] + ltokens + ["[SEP]"]
        #print (token)
        #try:
        #    encoding = [berttoken.convert_tokens_to_ids(ltokens) for t ]#.cuda()
        #except:
        #    continue
        #tensor = torch.tensor([encoding]).cuda()
        #pre = bert(tensor)[0] #F.softmax(bert(tensor)[0], dim=-1)
        #print (pre)

        topk = torch.topk(pre[i][i + 1], K_Number)#.tolist()
        value = topk[0].numpy()
        topk = topk[1].numpy().tolist()
        
        #print (topk)
        topkTokens = berttoken.convert_ids_to_tokens(topk)
        #print (topkTokens)
        #DA = oriencoding[i]
        
       # tarl = []
        for index in range(len(topkTokens)):
            if value[index] < 0.05:
                break
            tt = topkTokens[index]
            #print (tt)
            if tt in string.punctuation:
                continue
            if tt.strip() == tokens[i].strip():
                continue

            l = deepcopy(tokens)
            l[i] = tt
            tarl.append([l, i, value[index]])
        
    if len(tarl) == 0:
        return " ".join(tokens), gen
        
    
    
    lDB = []
    #batchsize = 100

    #oriencoding = bertori(torch.tensor([berttoken.convert_tokens_to_ids(ltokens)]).cuda())[0][0].data.cpu().numpy()
    #oriencoding = bertori(torch.tensor([berttoken.convert_tokens_to_ids(ltokens)]).cuda())[0][0].data.cpu().numpy()
    for i in range(0, len(tarl), batchsize):
        #tarlist = tarl[i: min(len(tarl), i + 300]
        lDB.append(bertori(torch.tensor([berttoken.convert_tokens_to_ids(["[CLS]"] + l[0] + ["[SEP]"]) for l in tarl[i: min(i + batchsize, len(tarl))]]).cuda())[0].data.cpu().numpy())
    lDB = np.concatenate(lDB, axis=0)
            
    #print ("-----------------")
    #print (len(lDB))
    #print (len(tarl))
    lDA = lDB[0]
    assert len(lDB) == len(tarl)
    tarl = tarl[1:]
    lDB = lDB[1:]
    for t in range(len(lDB)):
        DB = lDB[t][tarl[t][1]]
        DA = lDA[tarl[t][1]]
#        assert np.shape(DA) == np.shape(DB)
        cossim = np.sum(DA * DB) / (np.sqrt(np.sum(DA * DA)) * np.sqrt(np.sum(DB * DB)))
#        print (cossim)
        #print ()
        if cossim < 0.85:
            continue
            #print ("------")
            #print (" ".join(oritokens))
            #print (" ".join(tokens))
            #print (" ".join(l))
        sen = " ".join(tarl[t][0])# + "\t!@#$%^& " + str(math.exp(value[index]))#.replace(" ##", "")
        
            #if check_tree(tag, sen):
        gen.append([cossim, sen])
    if len(lcache) > 4:
        lcache = lcache[1:]    

    lcache.append([inpori, " ".join(tokens), gen])
    return " ".join(tokens), gen#.replace(" ##", ""), gen

f = open(sys.argv[1])
lines = f.readlines()
f.close()

l = []
for i in range(len(lines)):
    l.append(lines[i].strip())

bertmodel, berttoken, bertori = bertInit()

f = open(sys.argv[2], "w")
fline = open(sys.argv[3], "w")
for i in tqdm(range(len(l))):
    line = l[i]
    #tag = nlp.pos_tag(line)
    tar, gen = BertM(bertmodel, berttoken, line, bertori)
    gen = sorted(gen)[::-1]
    count = 0
    for sen in gen:
        f.write(tar.strip() + "\n")
        f.write(sen[1].strip() + "\n")
        fline.write(str(i // 2) + " " + str(i % 2) + "\n")
        count += 1
        if count >= Max_Mutants:
            break
f.close()
fline.close()
print (time.time() - ft)
