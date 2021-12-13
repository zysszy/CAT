import os
from sklearn.feature_extraction.text import TfidfVectorizer
from tqdm import tqdm
import nltk, string
from tf_idf import *
import re
import sys 
import six 
import nltk
import math
import numpy as np
import collections
import unicodedata
import time
from multiprocessing import Process

metrics = ["LCS", "TFIDF", "ED", "BLEU"]
thresholds = {"LCS": 0.963, "ED":0.963, "BLEU":0.906, "TFIDF":0.999}
processNum = 70

class UnicodeRegex(object):
  """Ad-hoc hack to recognize all punctuation and symbols."""

  def __init__(self):
    punctuation = self.property_chars("P")
    self.nondigit_punct_re = re.compile(r"([^\d])([" + punctuation + r"])")
    self.punct_nondigit_re = re.compile(r"([" + punctuation + r"])([^\d])")
    self.symbol_re = re.compile("([" + self.property_chars("S") + "])")

  def property_chars(self, prefix):
    return "".join(six.unichr(x) for x in range(sys.maxunicode)
                   if unicodedata.category(six.unichr(x)).startswith(prefix))

uregex = UnicodeRegex()

def bleu_tokenize(string):
  r"""Tokenize a string following the official BLEU implementation.
  See https://github.com/moses-smt/mosesdecoder/"
           "blob/master/scripts/generic/mteval-v14.pl#L954-L983
  In our case, the input string is expected to be just one line
  and no HTML entities de-escaping is needed.
  So we just tokenize on punctuation and symbols,
  except when a punctuation is preceded and followed by a digit
  (e.g. a comma/dot as a thousand/decimal separator).
  Note that a number (e.g. a year) followed by a dot at the end of sentence
  is NOT tokenized,
  i.e. the dot stays with the number because `s/(\p{P})(\P{N})/ $1 $2/g`
  does not match this case (unless we add a space after each sentence).
  However, this error is already in the original mteval-v14.pl
  and we want to be consistent with it.
  Args:
    string: the input string
  Returns:
    a list of tokens
  """
  string = uregex.nondigit_punct_re.sub(r"\1 \2 ", string)
  string = uregex.punct_nondigit_re.sub(r" \1 \2", string)
  string = uregex.symbol_re.sub(r" \1 ", string)
  return string.split()

def _get_ngrams(segment, max_order):
  """Extracts all n-grams up to a given maximum order from an input segment.
  Args:
    segment: text segment from which n-grams will be extracted.
    max_order: maximum length in tokens of the n-grams returned by this
        methods.
  Returns:
    The Counter containing all n-grams up to max_order in segment
    with a count of how many times each n-gram occurred.
  """
  ngram_with_padding = collections.Counter()
  ngram_counts = collections.Counter()
  for order in range(1, max_order + 1): 
    for i in range(0, len(segment) - order + 1): 
      ngram = tuple(segment[i:i + order])
      #print (segment[i: i + order])
      if "<padding>" in segment[i:i + order]:
        #print ("1")
        ngram_with_padding[ngram] += 1
      else:
        ngram_counts[ngram] += 1

  if len(ngram_with_padding) > 0:
      return ngram_counts, ngram_with_padding
  return ngram_counts, ngram_with_padding

def overlap_with_padding(ngram, collect, count_pad):
    _sum = 0
    p_ngram = []
    for f_index in range(count_pad + len(collect)):
        s_index = f_index - count_pad + 1
        l = []
        from copy import deepcopy
        n_ngram = []
        for t in ngram:
            n_ngram.append(deepcopy(t))
        #n_ngram = deepcopy(ngram)

        for index in range(max(0, s_index), min(f_index + 1, len(ngram))):
            n_ngram[index] = "<padding>"

        if "<padding>" not in n_ngram:
            continue
        n_ngram = tuple(n_ngram)
        #print (n_ngram)
        if n_ngram not in p_ngram:
            p_ngram.append(n_ngram)
            _sum += collect[n_ngram]
    #print (_sum)
    return _sum

def get_div_tokens(tokens, s, e):
    old_tokens = []
    new_tokens = []

    in_old = 0
    in_new = 0
    for i in range(s):
        if in_old > 0:
            old_tokens.append(tokens[i])
        elif in_new > 0:
            new_tokens.append(tokens[i])
        else:
            old_tokens.append(tokens[i])
            new_tokens.append(tokens[i])

        if tokens[i] == "{+":
            in_new += 1
        elif tokens[i] == "+}":
            in_new -= 1

        if tokens[i] == "[-":
            in_old += 1
        elif tokens[i] == "-]":
            in_old -= 1

        assert in_new < 2 and in_new > -1 and in_old < 2 and in_old > -1 and in_new + in_old < 2

    for i in range(e + 1, len(tokens)):
        if in_old > 0:
            old_tokens.append(tokens[i])
        elif in_new > 0:
            new_tokens.append(tokens[i])
        else:
            old_tokens.append(tokens[i])
            new_tokens.append(tokens[i])

        if tokens[i] == "{+":
            in_new += 1
        elif tokens[i] == "+}":
            in_new -= 1

        if tokens[i] == "[-":
            in_old += 1
        elif tokens[i] == "-]":
            in_old -= 1

        assert in_new < 2 and in_new > -1 and in_old < 2 and in_old > -1 and in_new + in_old < 2

    assert "[-" not in old_tokens and "{+" not in old_tokens
    assert "[-" not in new_tokens and "{+" not in new_tokens
    return old_tokens, new_tokens

def get_sub_sentence(tokens, index): # index denotes where the "[-" / "{+" is.
    s = index
    e = -1
    for i in range(index, len(tokens)):
        if i < len(tokens) - 1 and tokens[i + 1] == "{+" and tokens[i] == "-]":
            pass
        elif tokens[i] in ["+}", "-]"]:
            e = i
            break

    assert index == len(tokens) or e != -1
    return get_div_tokens(tokens, s, e)

def sentences_from_wdiff(wdiff_line):
    count_o = []
    count_n = []
    bf = ""
    #for line in lines:
    now = wdiff_line
    now = bf + " " + now
    now = re.sub(r"([\[])([\-])", r" \1\2 ", now)
    now = re.sub(r"([\-])([\]])", r" \1\2 ", now)
    now = re.sub(r"([\{])([\+])", r" \1\2 ", now)
    now = re.sub(r"([\+])([\}])", r" \1\2 ", now)
    # print (now)

    # stable
    o = []
    n = []
    #print ("-----------------")
    words = now.strip().split()
    old_tokens = []
    new_tokens = []

    in_old = 0
    in_new = 0
    tokens = words
    for i in range(len(words)):
        if in_old > 0 and tokens[i] not in ["{+", "+}", "[-", "-]"]:
            old_tokens.append(tokens[i])
        elif in_new > 0 and tokens[i] not in ["{+", "+}", "[-", "-]"]:
            new_tokens.append(tokens[i])
        elif tokens[i] not in ["{+", "+}", "[-", "-]"]:
            old_tokens.append(tokens[i])
            new_tokens.append(tokens[i])

        if tokens[i] == "{+":
            in_new += 1
        elif tokens[i] == "+}":
            in_new -= 1

        if tokens[i] == "[-":
            in_old += 1
        elif tokens[i] == "-]":
            in_old -= 1

        assert in_new < 2 and in_new > -1 and in_old < 2 and in_old > -1 and in_new + in_old < 2
    o.append(old_tokens)
    n.append(new_tokens)
    #o.append(words)
    #n.append(words)
    #print (" ".join(words))
    for i in range(len(words)):
        if words[i] == "[-":
            for t in range(i + 1, len(words)):
                if words[t] == "-]":
                    sentence = []
                    in_new = 0
                    if t - i >= 5:
                        break
                    for k in range(len(words)):
                        if k >= i and k <=t:
                            continue
                        elif words[k] == "{+":
                            in_new += 1
                        elif words[k] == "+}":
                            in_new -= 1
                            assert in_new >= 0
                            continue
                        if in_new > 0:
                            continue
                        if words[k] in ["[-", "-]"]:
                            continue
                        else:
                            sentence.append(words[k])
                    #print (" ".join(sentence))
                    o.append(sentence)
                    break

        if words[i] == "{+":
            for t in range(i + 1, len(words)):
                if words[t] == "+}":
                    sentence = []
                    in_new = 0
                    if t - i >= 5:
                        break
                    for k in range(len(words)):
                        if k >= i and k <=t:
                            continue
                        elif words[k] == "[-":
                            in_new += 1
                        elif words[k] == "-]":
                            in_new -= 1
                            assert in_new >= 0
                            continue
                        if in_new > 0:
                            continue
                        if words[k] in ["{+", "+}"]:
                            continue
                        else:
                            sentence.append(words[k])
                    #print (" ".join(sentence))
                    n.append(sentence)
                    break

    for i in o:
        for t in n:
            count_o.append(i)
            count_n.append(t)

    return count_o, count_n

def read_from_wdiff(old_sentence, new_sentence, term):
    #print (old_sentence)
    f = open(f"{term}memory_2.txt", "w")
    f.write(old_sentence.strip())
    f.close()
    f = open(f"{term}memory_1.txt", "w")
    f.write(new_sentence.strip())
    f.close()

    diff = os.popen(f"wdiff {term}memory_1.txt {term}memory_2.txt")
    lines = diff.readlines()
    #print (lines)
    assert len(lines) == 1
    diff.close()

    return lines[0]

def compute_bleu(reference_corpus,
                 translation_corpus,
                 max_order=4,
                 use_bp=True):
  """Computes BLEU score of translated segments against one or more references.
  Args:
    reference_corpus: list of references for each translation. Each
        reference should be tokenized into a list of tokens.
    translation_corpus: list of translations to score. Each translation
        should be tokenized into a list of tokens.
    max_order: Maximum n-gram order to use when computing BLEU score.
    use_bp: boolean, whether to apply brevity penalty.
  Returns:
    BLEU score.
  """
  reference_length = 0
  translation_length = 0
  bp = 1.0
  geo_mean = 0

  matches_by_order = [0] * max_order
  possible_matches_by_order = [0] * max_order
  precisions = []

  for (references, translations) in zip(reference_corpus, translation_corpus):
    reference_length += len(references)
    count_pad = 0
    for i in references:
        if i == "<padding>":
            count_pad += 1

    translation_length += len(translations)
    #print (references)
    #print (max_order)
    ref_ngram_counts, ngram_with_padding = _get_ngrams(references, max_order)
    translation_ngram_counts, _ = _get_ngrams(translations, max_order)

    overlap = dict((ngram,
                    min(count, translation_ngram_counts[ngram]))
                   for ngram, count in ref_ngram_counts.items())

    for ngram, count in translation_ngram_counts.items():
        if ngram in overlap:
            overlap[ngram] += overlap_with_padding(ngram, ngram_with_padding, count_pad)
        else:
            overlap[ngram] = overlap_with_padding(ngram, ngram_with_padding, count_pad)

    for ngram in overlap:
      matches_by_order[len(ngram) - 1] += overlap[ngram]
      possible_matches_by_order[len(ngram) - 1] += overlap_with_padding(ngram, ngram_with_padding, count_pad)
    for ngram in translation_ngram_counts:
      possible_matches_by_order[len(ngram)-1] += translation_ngram_counts[ngram]

    #for ngram in ngram_with_padding:
    #    possible_matches_by_order[len(ngram) - 1] += ngram_with_padding[ngram]

  precisions = [0] * max_order
  smooth = 1.0
  for i in range(0, max_order):
    if possible_matches_by_order[i] > 0:
      precisions[i] = matches_by_order[i] / possible_matches_by_order[i]
      if matches_by_order[i] > 0:
        precisions[i] = matches_by_order[i] / possible_matches_by_order[i]
      else:
        smooth *= 2
        precisions[i] = 1.0 / (smooth * possible_matches_by_order[i])
    else:
      precisions[i] = 0.0

  if max(precisions) > 0:
    p_log_sum = sum(math.log(p) for p in precisions if p)
    geo_mean = math.exp(p_log_sum/max_order)

  if use_bp:
    if not reference_length:
      bp = 1.0
    else:
      ratio = translation_length / reference_length
      if ratio <= 0.0:
        bp = 0.0
      elif ratio >= 1.0:
        bp = 1.0
      else:
        bp = math.exp(1 - 1. / ratio)
  bleu = geo_mean * bp
  return np.float32(bleu)

def com_bleu(en, new, ora, en_old, en_new):
    sum_ = 0
    or_lines = []
    en_lines = []
    f = open("ans.bleu", "w")
    l = []
    for i in range(len(en)):
        en_lines = []
        or_lines = []
        en_new_lines = []
        en_new_line = bleu_tokenize(new[i].replace("<padding>", "padding"))
        for t in range(len(en_new_line)):
            if en_new_line[t] == "padding":
                en_new_line[t] = "<padding>"
        en_new_lines.append(en_new_line)

        en_line = bleu_tokenize(en[i].replace("<padding>", "padding"))
        for t in range(len(en_line)):
            if en_line[t] == "padding":
                en_line[t] = "<padding>"

        en_lines.append(en_line)
        or_line = bleu_tokenize(ora[i].replace("<padding>", "padding"))

        for t in range(len(or_line)):
            if or_line[t] == "padding":
                or_line[t] = "<padding>"

        or_lines.append(or_line)
        score = compute_bleu(or_lines, en_lines)#nltk.translate.bleu_score.sentence_bleu([or_line], en_line, weights=(0.25, 0.25, 0.25, 0.25), smoothing_function=nltk.translate.bleu_score.SmoothingFunction.method4)
        score_new = compute_bleu(or_lines, en_new_lines)#nltk.translate.bleu_score.sentence_bleu([or_line], en_line, weights=(0.25, 0.25, 0.25, 0.25), smoothing_function=nltk.translate.bleu_score.SmoothingFunction.method4)
        #print (en_line)
        delta = abs(score - score_new)
        l.append([delta, score, score_new, en_old[i], en[i], en_new[i], new[i], ora[i]])
        f.write(str(score))
        f.write("\n")
        #print (or_line)
        #print (score)
        sum_ += score
    f_com = open("Com_BLEU.txt", "w")
    l = sorted(l)[::-1]

    for t in range(len(l)):
        f_com.write(str(l[t][0]) + " " + str(l[t][1]) +  " " + str(l[t][2]) + "\n")
        for k in range(3, 8):
            f_com.write(l[t][k])
        f_com.write("\n")

    f_com.close()
    return sum_ / len(en)

def com_bleu_2 (old, new):
    en_lines = []
    or_lines = []
    en_new_line = bleu_tokenize(new)
    en_line = bleu_tokenize(old)
    or_lines.append(en_line)
    en_lines.append(en_new_line)
    score = compute_bleu(or_lines, en_lines)#nltk.tran
    #l.append([score, score, score, en_old[i], en[i], en_new[i], new[i], ora[i]])
    return score#sum_ / len(en)

def bleu_sim_d(x, y):
    assert len(x) == len(y)
    ma = -1000
    #print (x)
    assert len(x) > 0
    for i in range(len(x)):
        lcs_vari = float(com_bleu_2(" ".join(x[i]), " ".join(y[i])))
        lcs_score = (lcs_vari) #/ max(len(x[i]), len(y[i]))
        ma = max(ma, lcs_score)
        lcs_vari = float(com_bleu_2(" ".join(y[i]), " ".join(x[i])))
        lcs_score = (lcs_vari) #/ max(len(x[i]), len(y[i]))
        ma = max(ma, lcs_score)

    return ma

#f = open("full.en")
#en_lines = f.readlines()
#f.close()

#f = open("full.oracle.en")
#or_lines = f.readlines()
#f.close()

#print (com_bleu(en_lines, or_lines))




#nltk.download('punkt') # if necessary...


stemmer = nltk.stem.porter.PorterStemmer()
remove_punctuation_map = dict((ord(char), None) for char in string.punctuation)

def stem_tokens(tokens):
    return [stemmer.stem(item) for item in tokens]

'''remove punctuation, lowercase, stem'''
def normalize(text):
    #print (text.strip())
    return stem_tokens(text.strip().split())#nltk.word_tokenize(text.lower().translate(remove_punctuation_map)))

vectorizer = TfidfVectorizer(tokenizer=normalize)

def tfidf_cos_sim(text1, text2):
    tfidf = vectorizer.fit_transform([text1, text2])
    return ((tfidf * tfidf.T).A)[0,1]

def tf_cos_sim(x, y):
    count = 0
    new_oracle = []
    #print (y) 
    y = y.strip().split()
    x = x.strip().split()
    for i in y:
        if i == "<padding>":
            count += 1
        else:
            new_oracle.append(i)

    min_ed = -2
    for i in range(len(x) - count):
        new_x = []
        for t in range(len(x)):
            if t - i >= 0 and t - i < count:
                continue
            else:
                new_x.append(x[t])
        #print (" ".join(new_x))

        dic_a = compute_tfidf(" ".join(new_x))
        dic_b = compute_tfidf(" ".join(new_oracle))
        ed = compute_cos(dic_a, dic_b)
        min_ed = max(ed, min_ed)

    if min_ed == -2:
        min_ed = 1

    return min_ed

def tf_cos_d_sim(x, y):
    ma = -100000
    for i in range(len(x)):
        x_now = x[i]
        y_now = y[i]
        dic_a = compute_tfidf(" ".join(x_now))
        dic_b = compute_tfidf(" ".join(y_now))
        ma = max(ma, compute_cos(dic_a, dic_b))
    return ma

def lcs(X, Y):
    # find the length of the strings 
    m = len(X)
    n = len(Y)

    # declaring the array for storing the dp values 
    L = [[None]*(n + 1) for i in range(m + 1)]

    """Following steps build L[m + 1][n + 1] in bottom up fashion 
    Note: L[i][j] contains length of LCS of X[0..i-1] 
    and Y[0..j-1]"""
    for i in range(m + 1):
        for j in range(n + 1):
            if i == 0 or j == 0 :
                L[i][j] = 0
            elif X[i-1] == Y[j-1]:
                L[i][j] = L[i-1][j-1]+1
            else:
                L[i][j] = max(L[i-1][j], L[i][j-1])

    # L[m][n] contains the length of LCS of X[0..n-1] & Y[0..m-1] 
    return L[m][n]
# end of function lcs 

def lcs_sim(x, y): #y denotes the oracle sentence
    count = 0

    new_oracle = []
    for i in y:
        if i == "<padding>":
            count += 1
        else:
            new_oracle.append(i)

    lcs_vari = float(lcs(x, new_oracle))
    lcs_score = (lcs_vari + count) / max(len(x), len(y))
    return lcs_score

def lcs_sim_d(x, y):
    assert len(x) == len(y)
    ma = -1000
    #print (x)
    assert len(x) > 0
    for i in range(len(x)):
        lcs_vari = float(lcs(x[i], y[i]))
        try:
            lcs_score = (lcs_vari) / max(len(x[i]), len(y[i]))
        except:
            lcs_score = 1
        ma = max(ma, lcs_score)

    return ma
def Edit_Distance(s1, s2):
    if len(s1) > len(s2):
        s1, s2 = s2, s1

    distances = range(len(s1) + 1)
    for i2, c2 in enumerate(s2):
        distances_ = [i2+1]
        for i1, c1 in enumerate(s1):
            if c1 == c2:
                distances_.append(distances[i1])
            else:
                distances_.append(1 + min((distances[i1], distances[i1 + 1], distances_[-1])))
        distances = distances_
    return distances[-1]

def ed_sim(x, y): #y denotes the oracle sentence
    count = 0
    new_oracle = []
    for i in y:
        if i == "<padding>":
            count += 1
        else:
            new_oracle.append(i)

    min_ed = 1e18
    for i in range(len(x) - count):
        new_x = []
        for t in range(len(x)):
            if t - i >= 0 and t - i < count:
                continue
            else:
                new_x.append(x[t])

        ed = Edit_Distance(new_x, new_oracle)
        min_ed = min(ed, min_ed)

    if min_ed == 1e18:
        min_ed = 0
    try:
        return 1. - float(min_ed) / max(len(x), len(y))
    except:
        return 1

def ed_sim_d(x, y):
    max_ed = -1e18
    for i in range(len(x)):
        ed = Edit_Distance(x[i], y[i])
        try:
            ed = 1. - float(ed) / max(len(x[i]), len(y[i]))
        except:
            ed = 1
        max_ed = max(ed, max_ed)
    #return 1. - float(min_ed) / max(len(x[i]), len(y[i]))

    # if min_ed == 1e18:
    #     min_ed = 0

    #return 1. - float(min_ed) / max(len(x[i]), len(y[i]))
    return max_ed



def ComScore(trans, simCom, term):
    ret = trans
    [r.append(0) for r in ret]

    for i in range(len(ret)):
        for t in range(len(ret)):
            if i != t:#simCom(ret[1], ret[2])
                wdiff = read_from_wdiff(ret[i][1],ret[t][1], term)
                old_tokens, new_tokens = sentences_from_wdiff(wdiff)
                ret[i][4] += simCom(old_tokens, new_tokens)
                #ret[t][3] += simCom(ret[t][1], ret[i][1])

    return ret

def getDiffEn(ori, n):
    start = -1
    endori = -1
    endnew = -1
    for i in range(len(ori)):
        if ori[i] != n[i] and start == -1:
            start = i
        if ori[len(ori) - 1 - i] != n[len(n) - 1 - i]:
            endori = len(ori) - 1 - i
            endnew = len(n) - 1 - i
        if start != -1 and endori != -1 and endnew != -1:
            break
    
    return start, endori, endnew

def Vote(orien1, orizh1, orialign, trans, simCom, threshold, prob=False, repaired = None, term=-1):
    rep = ""
    orien = orien1.split()
    orizh = orizh1.split()

    for i in tqdm(range(len(trans))):
        en = trans[i][0].split()
        zh = trans[i][1].split()
        #wdiff = read_from_wdiff(orizh, zh)
        #old_tokens, new_tokens = sentences_from_wdiff(wdiff)

        if orien == en:
            rep = zh
        else:
            start, endori, endnew = getDiffEn(orien, en)
            orizhwords = []
            zhwords = []

            for t in range(start, endori + 1):
                if orien[t] in orialign:
                    orizhwords += orialign[orien[t]]
            
            for t in range(start, endnew + 1):
                if en[t] in trans[i][2]:
                    zhwords += trans[i][2][en[t]]

            if len(orizhwords) == 0 or len(zhwords) == 0:
                continue

            orizhwords = sorted(orizhwords)
            zhwords = sorted(zhwords)
            
            flag = False
            for key in range(1, len(orizhwords)):
                if orizhwords[key] != orizhwords[key - 1] + 1:
                    flag = True
                    break
            
            for key in range(1, len(zhwords)):
                if zhwords[key] != zhwords[key - 1] + 1:
                    flag = True
                    break

            if flag == True:
                continue

            rep = []
            for t in range(zhwords[0]):
                rep.append(zh[t])

            for t in orizhwords:
                rep.append(orizh[t])

            for t in range(zhwords[-1] + 1, len(zh)):
                rep.append(zh[t])

        if repaired is not None:
            wdiff = read_from_wdiff(repaired, " ".join(rep), term)
            old_tokens, new_tokens = sentences_from_wdiff(wdiff)
            if simCom(old_tokens, new_tokens) >= threshold:
                return " ".join(rep)
        else:
            return " ".join(rep)

    return " ".join(orizh)

def RepairWithScores(muts, scoreType = None, term = -1):
    repaired = []
    repairedProb = []
    #thresholds = {"LCS": 0.9, "ED":0.9, "BLEU":0.9, "TFIDF":0.9}
    global thresholds
    threshold = thresholds[scoreType]

    cache = {}
    cacheProb = {}

    for i in tqdm(range(len(muts))):
        #if i % 2 == 0:
        #    mut = muts[i + 1]
        #else:
        #    mut = muts[i - 1]
        try:    
            mut = muts[i]
            trans = []
            used = []
            orien = ""
            orizh = ""
            orialign = []
            for m in mut:
                en = m[0].strip()
                zh = m[1].strip()
                orien = en 
                orizh = zh
                orialign = m[2]
                if en not in used:
                    used.append(en)
                    trans.append([en, zh, m[2], m[3]])
                
                en = m[4].strip()
                zh = m[5].strip()
                
                #wdiff = read_from_wdiff(orizh, zh)
                #old_tokens, new_tokens = sentences_from_wdiff(wdiff)

                if en not in used:
                    used.append(en)
                    trans.append([en, zh, m[6], m[7]])
                
                assert scoreType in ["LCS", "ED", "BLEU", "TFIDF"]
                
                if scoreType == "LCS":
                    simCom = lcs_sim_d#LCSsim
                    #trans = ComScore(trans, LCSsimCom)
                elif scoreType == "ED":
                    simCom = ed_sim_d#EDsim
                    #trans = ComScore(trans, EDsim)
                elif scoreType == "BLEU":
                    simCom = bleu_sim_d#BLEUsim
                    #trans = ComScore(trans, BLEUsim)
                elif scoreType == "TFIDF":
                    simCom = tf_cos_d_sim#TFIDFsim
                    #trans = ComScore(trans, TFIDFsim)
                
            if orien in cache and i % 2 == 0:
                repaired.append(cache[orien])
                repairedProb.append(cacheProb[orien])
                #print(repaired[-1])
                #print(repairedProb[-1])
                continue
            trans = ComScore(trans, simCom, term)
            
            probSort = sorted(trans, key=lambda x: x[3], reverse=True)
            scoreSort = sorted(trans, key=lambda x: x[4], reverse=True)
            #print (probSort)
            #print (term)
        except:
            repaired.append(orizh)
            repairedProb.append(orizh)
            continue
        #exit()
        if i % 2 == 0: # Ori
            try:
                repaired.append(Vote(orien, orizh, orialign, scoreSort, simCom, threshold, prob=False))
            except:
                repaired.append(orizh)
            cache[orien] = repaired[-1]
            try:
                repairedProb.append(Vote(orien, orizh, orialign, probSort, simCom, threshold, prob=True))
            except:
                repairedProb.append(orizh)
            cacheProb[orien] = repairedProb[-1] 
            #print (repaired[-1])
            #print (repairedProb[-1])
        else: # Mut
            try:
                repaired.append(Vote(orien, orizh, orialign, scoreSort, simCom, threshold, prob=False, repaired=repaired[-1], term=term))
            except:
                repaired.append(orizh)

            try:
                repairedProb.append(Vote(orien, orizh, orialign, probSort, simCom, threshold, prob=True, repaired=repairedProb[-1], term=term))
            except:
                repairedProb.append(orizh)
            #print (repaired[-1])
            #print (repairedProb[-1])
            #cg = repaired[-1]
            #repaired[-1] = repaired[-2]
            #repaired[-2] = cg

            #cg = repairedProb[-1]
            #repairedProb[-1] = repairedProb[-2]
            #repairedProb[-2] = cg
        #print (repaired[-1])

    return repaired, repairedProb

def runProcessing(muts, met, term):
    print (term)
    rep, repprob = RepairWithScores(muts, met, term)
    with open(f"{met}_Repaired_{term}.zh", "w") as f:
        [f.write(r + "\n") for r in rep]
    with open(f"{met}_Repaired_{term}.prob.zh", "w") as f:
        [f.write(r + "\n") for r in repprob]

def ReadFile(met):
    f = open(f"index.{met}", "r")
    lines = f.readlines()
    f.close()

    mutsIndex = []
    for line in lines:
        mutsIndex.append([int(num) for num in line.strip().split()])

    with open(f"bugs_{met}.txt", "r") as f:
        lines = f.readlines()

    bugs = [line.strip() for line in lines]

    muts = [[] for bug in bugs]
    with open(f"{met}_Mu.en", "r") as f:
        enlines = f.readlines()
    
    with open(f"{met}_Mu.zh", "r") as f:
        zhlines = f.readlines()

    with open(f"align_ori.{met}.txt", "r") as f:
        aligns = f.readlines()

    print (f"{met}")
    assert len(enlines) == len(zhlines) 
    assert len(enlines) == len(mutsIndex) * 2

    for i in range(0, len(enlines), 2):
        index = i // 2
        enoriline = enlines[i].strip()
        zhoriline = zhlines[i].strip().split("\t")[0]
        oriprob = float(zhlines[i].strip().split("\t")[1])
        enline = enlines[i + 1].strip()
        zhline = zhlines[i + 1].strip().split("\t")[0]
        prob = float(zhlines[i + 1].strip().split("\t")[1])

        
        alignori = eval(aligns[i])
        align = eval(aligns[i + 1])

        muts[2 * mutsIndex[index][0] + mutsIndex[index][1]].append([enoriline, zhoriline, alignori, oriprob, enline, zhline, align, prob])
      
    batchsize = len(muts) // (processNum - 1)
    if batchsize % 2 == 1:
        batchsize += 1
    pro = [Process(target=runProcessing, args=(muts[i * batchsize: min(len(muts), (i + 1) * batchsize)], met, i,)) for i in range(processNum)]
    for p in pro:
        p.start()

    for p in pro:
        p.join()

    ret = []
    #for i in range(processNum):
    s = " ".join([f"{met}_Repaired_{t}.zh" for t in range(processNum)])
    os.system(f"cat {s} > {met}_Repaired.zh")
    s = " ".join([f"{met}_Repaired_{t}.prob.zh" for t in range(processNum)])
    os.system(f"cat {s} > {met}_Repaired.prob.zh")
        #with open(f"{met}_Repaired_{term}.zh")
    #for i in range()
        #print (muts[0])
    #rep, repprob = RepairWithScores(muts, met)
    #with open(f"{met}_Repaired.zh", "w") as f:
    #    [f.write(r + "\n") for r in rep]
    #with open(f"{met}_Repaired.prob.zh", "w") as f:
    #    [f.write(r + "\n") for r in repprob]

for met in metrics:
    ReadFile(met)
