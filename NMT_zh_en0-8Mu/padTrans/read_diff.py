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

tttt = time.time()


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

    # aggressive
    words = now.strip().split()
    now_count = 0
    for i in range(len(words)):
        if words[i] in ["[-", "{+"]:
            if words[i] == "{+" and i >= 1 and words[i - 1] == "-]":
                continue
            now_count += 1
            old_tokens, new_tokens = get_sub_sentence(words, i)
            count_o.append(old_tokens)
            count_n.append(new_tokens)
    
    if now_count == 0:
        old_tokens, new_tokens = get_sub_sentence(words, len(words))
        count_o.append(old_tokens)
        count_n.append(new_tokens)


    return count_o, count_n 


def read_from_wdiff(old_sentence, new_sentence):
    #print (old_sentence)
    f = open("memory_2.txt", "w")
    f.write(old_sentence.strip())
    f.close()
    f = open("memory_1.txt", "w")
    f.write(new_sentence.strip())
    f.close()

    diff = os.popen("wdiff memory_1.txt memory_2.txt")
    lines = diff.readlines()
    print (lines)
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




nltk.download('punkt') # if necessary...


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

f = open("Com_Mutated.en", "r")
en_new_lines = f.readlines()
f.close()

f = open("Com_Original.en", "r")
en_old_lines = f.readlines()
f.close()

f = open("Com_Mutated.zh", "r")
new_lines = f.readlines()
f.close()

f = open("Com_Original.zh", "r")
old_lines = f.readlines()
f.close()

f = open("Com_oracle.zh", "r")
ora_lines = f.readlines()
f.close()

assert len(en_old_lines) == len(en_new_lines)
assert len(en_old_lines) == len(old_lines)
assert len(old_lines) == len(new_lines)
assert len(old_lines) == len(ora_lines)

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

    return 1. - float(min_ed) / max(len(x), len(y))

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

old_lcs_sum = 0
new_lcs_sum = 0
#ed_sum = 0
old_ed_sum = 0
new_ed_sum = 0

old_ti_sum = 0
new_ti_sum = 0

old_bleu_sum = 0
new_bleu_sum = 0

lcs_l = []
lcs_d = []
ed_l = []
ed_d = []
tfidf_l = []
tfidf_d = []
bleu_l = []
bleu_d = []

from copy import deepcopy

f_edit = open("Com_Answer.zh", "w")
#for i in tqdm(range(len(old_lines))):
#    # break
#    old_ti_sim = tf_cos_sim(old_lines[i], ora_lines[i])
#    new_ti_sim = tf_cos_sim(new_lines[i], ora_lines[i])

for i in tqdm(range(len(old_lines))):
    old_line = old_lines[i].strip().split()
    new_line = new_lines[i].strip().split()
    ora_line = ora_lines[i].strip().split()
    
    wdiff = read_from_wdiff(" ".join(old_line), " ".join(new_line))
    old_tokens, new_tokens = sentences_from_wdiff(wdiff)

    old_lcs_sim = 1#lcs_sim(old_line, ora_line)
    new_lcs_sim = 1#lcs_sim(new_line, ora_line)
    
    lcs_d_sim = lcs_sim_d(old_tokens, new_tokens)
    lcs_d.append(deepcopy([lcs_d_sim, -1, -1, old_lines[i], en_old_lines[i], new_lines[i], en_new_lines[i], ora_lines[i]]))


    tf_d_sim = tf_cos_d_sim(old_tokens, new_tokens)
    tfidf_d.append(deepcopy([tf_d_sim, -1, -1, old_lines[i], en_old_lines[i], new_lines[i], en_new_lines[i], ora_lines[i]]))

    delta = abs(old_lcs_sim - new_lcs_sim)
    lcs_l.append(deepcopy([delta, old_lcs_sim, new_lcs_sim, old_lines[i], en_old_lines[i], new_lines[i], en_new_lines[i], ora_lines[i]]))

    old_lcs_sum += old_lcs_sim #lcs_sim(old_line, ora_line)  
    new_lcs_sum += new_lcs_sim #lcs_sim(new_line, ora_line)
    
    old_ed_sim = 1#ed_sim(old_line, ora_line)
    new_ed_sim = 1#ed_sim(new_line, ora_line)
 
    ed_d_sim = ed_sim_d(old_tokens, new_tokens)

    delta = abs(old_ed_sim - new_ed_sim)
    ed_l.append(deepcopy([delta, old_ed_sim, new_ed_sim, old_lines[i], en_old_lines[i], new_lines[i], en_new_lines[i], ora_lines[i]]))
    ed_d.append(deepcopy([ed_d_sim, -1, -1, old_lines[i], en_old_lines[i], new_lines[i], en_new_lines[i], ora_lines[i]]))
    bleu_d_sim = bleu_sim_d(old_tokens, new_tokens)

    bleu_d.append(deepcopy([bleu_d_sim, -1, -1, old_lines[i], en_old_lines[i], new_lines[i], en_new_lines[i], ora_lines[i]]))
    old_ed_sum += old_ed_sim #ed_sim(old_line, ora_line)
    new_ed_sum += new_ed_sim #ed_sim(new_line, ora_line)
    
    old_ti_sim = 1#tf_cos_sim(old_lines[i], ora_lines[i])
    new_ti_sim = 1#tf_cos_sim(new_lines[i], ora_lines[i])
 
    delta = abs(old_ti_sim - new_ti_sim)
    tfidf_l.append(deepcopy([delta, old_ti_sim, new_ti_sim, old_lines[i], en_old_lines[i], new_lines[i], en_new_lines[i], ora_lines[i]]))

    old_ti_sum += old_ti_sim #tfidf_cos_sim(old_lines[i], ora_lines[i])
    new_ti_sum += new_ti_sim #tfidf_cos_sim(new_lines[i], ora_lines[i])

    #en_lines = []
    #or_lines = []
    #en_new_lines_b = []
    #en_new_line = bleu_tokenize(new_lines[i].replace("<padding>", "padding"))
    #for t in range(len(en_new_line)):
    #    if en_new_line[t] == "padding":
    #        en_new_line[t] = "<padding>"
    #en_new_lines_b.append(en_new_line)

    #en_line = bleu_tokenize(old_lines[i].replace("<padding>", "padding"))
    #for t in range(len(en_line)):
    #    if en_line[t] == "padding":
    #        en_line[t] = "<padding>"

    #en_lines.append(en_line)
    #or_line = bleu_tokenize(ora_lines[i].replace("<padding>", "padding"))

    #for t in range(len(or_line)):
    #    if or_line[t] == "padding":
    #        or_line[t] = "<padding>"

    #or_lines.append(or_line)
    score = 1#compute_bleu(or_lines, en_lines)#nltk.translate.bleu_score.sentence_bleu([or_line], en_line, weights=(0.25, 0.25, 0.25, 0.25), smoothing_function=nltk.translate.bleu_score.SmoothingFunction.method4)
    score_new = 1#compute_bleu(or_lines, en_new_lines_b)#nltk.translate.bleu_score.sentence_bleu([or_line], en_line, weights=(0.25, 0.25, 0.25, 0.25), smoothing_function=nltk.translate.bleu_score.SmoothingFunction.method4)
    #print (en_line)
    delta = abs(score - score_new)
    bleu_l.append([delta, score, score_new, old_lines[i], en_old_lines[i], new_lines[i], en_new_lines[i], ora_lines[i]])

#lcs_l = sorted(lcs_l)[::-1]
#ed_l = sorted(ed_l)[::-1]
#tfidf_l = sorted(tfidf_l)[::-1]

def write_file(l, file_name):
    f = open(file_name, "w")
    for t in l:
        f.write(str(t[0]) + " " + str(t[1]) + " " + str(t[2]) + "\n")
        for k in range(3, 8):
            f.write(str(t[k].strip()) + "\n")
        f.write("\n")
    f.close()


sum_list = []

for i in range(4):
    sum_list.append(0)

def write_all(lcs, ed, tfidf, bleu, lcs_d, ed_d, tfidf_d, file_name):
    global sum_list
    f = open(file_name, "w")
    l = lcs
    for site in range(len(l)):
        t = l[site]
        flag = False
        if t[0] > 0.1 * max(t[1], t[2]):
            flag = True
        flag = str(flag)
        f.write("LCS:\t" + str(t[0]) + " " + str(t[1]) + " " + str(t[2]) + " " + flag + "\n")
        flag = False
        if ed[site][0] > 0.1 * max(ed[site][1], ed[site][2]):
            flag = True
        flag = str(flag)
        f.write("ED:\t" + str(ed[site][0]) + " " + str(ed[site][1]) + " " + str(ed[site][2]) + " " + flag + "\n")
        flag = False
        if tfidf[site][0] > 0.1 * max(tfidf[site][1], tfidf[site][2]):
            flag = True
        flag = str(flag)
        f.write("Tf-idf:\t" + str(tfidf[site][0]) + " " + str(tfidf[site][1]) + " " + str(tfidf[site][2]) + " " + flag + "\n")
        flag = False
        if bleu[site][0] > 0.1 * max(bleu[site][1], bleu[site][2]):
            flag = True
        flag = str(flag)
        f.write("BLEU:\t" + str(bleu[site][0]) + " " + str(bleu[site][1]) + " " + str(bleu[site][2]) + " " + flag + "\n")

        flag = False
        if lcs_d[site][0] <= 0.95:#ed[site][0] > 0.1 * max(ed[site][1], ed[site][2]):
            flag = True
        flag = str(flag)
        f.write("LCS-Wdiff:\t" + str(lcs_d[site][0]) + " " + flag + "\n")

        flag = False
        if ed_d[site][0] <= 0.95:#ed[site][0] > 0.1 * max(ed[site][1], ed[site][2]):
            flag = True
        flag = str(flag)
        f.write("ED-Wdiff:\t" + str(ed_d[site][0]) + " " + flag +"\n")

        flag = False
        if tfidf_d[site][0] <= 0.95:#ed[site][0] > 0.1 * max(ed[site][1], ed[site][2]):
            flag = True
        flag = str(flag)
        f.write("Tf-idf-Wdiff:\t" + str(tfidf_d[site][0]) + " " + flag + "\n")
        

        flag = False
        if tfidf_d[site][0] <= 0.85:#ed[site][0] > 0.1 * max(ed[site][1], ed[site][2]):
            flag = True
        flag = str(flag)
        f.write("BLEU-Wdiff:\t" + str(bleu_d[site][0]) + " " + flag + "\n")
        sum_list[0] += t[0]
        sum_list[1] += ed[site][0]
        sum_list[2] += tfidf[site][0]
        sum_list[3] += bleu[site][0]
        
        for k in range(3, 8):
            f.write(str(t[k].strip()) + "\n")
        f.write("\n")
    sum_list[0] /= len(l)
    sum_list[1] /= len(l)
    sum_list[2] /= len(l)
    sum_list[3] /= len(l)
    f.close()

write_all(lcs_l, ed_l, tfidf_l, bleu_l, lcs_d, ed_d, tfidf_d, "Com_ALL.txt")

lcs_l = sorted(lcs_l)[::-1]
ed_l = sorted(ed_l)[::-1]
lcs_d = sorted(lcs_d)[::-1]
ed_d = sorted(ed_d)[::-1]
tfidf_l = sorted(tfidf_l)[::-1]
tfidf_d = sorted(tfidf_d)[::-1]
bleu_l = sorted(bleu_l)[::-1]
write_file(lcs_l, "Com_LCS.txt")
write_file(ed_l, "Com_ED.txt")
write_file(tfidf_l, "Com_TFIDF.txt")
write_file(bleu_l, "Com_BLEU.txt")
write_file(lcs_d, "Com_LCS_d.txt")
write_file(ed_d, "Com_ED_d.txt")
write_file(tfidf_d, "Com_TFIDF_d.txt")

old_lcs = float(old_lcs_sum) / len(old_lines)
new_lcs = float(new_lcs_sum) / len(old_lines)
lcs = old_lcs + new_lcs
lcs /= 2

old_ed = float(old_ed_sum) / len(old_lines)
new_ed = float(new_ed_sum) / len(old_lines)
ed = old_ed + new_ed
ed /= 2

old_ti = float(old_ti_sum) / len(old_lines)
new_ti = float(new_ti_sum) / len(old_lines)
ti = old_ti + new_ti
ti /= 2

for i in range(4):
    f_edit.write(str(sum_list[i]) + "\n")

#f_edit.write("old lcs: " + str(old_lcs) + "\n")
#f_edit.write("new lcs: " + str(new_lcs) + "\n")
#f_edit.write("lcs: " + str(lcs) + "\n")

#f_edit.write("old ed: " + str(old_ed) + "\n")
#f_edit.write("new ed: " + str(new_ed) + "\n")
#f_edit.write("ed: " + str(ed) + "\n")

#f_edit.write("old ti: " + str(old_ti) + "\n")
#f_edit.write("new ti: " + str(new_ti) + "\n")
#f_edit.write("ti: " + str(ti) + "\n")
f_edit.close()
print (time.time() - tttt)
