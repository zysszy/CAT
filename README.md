# CAT (Improving Machine Translation Systems via Isotopic Replacement)

Machine translation plays an essential role in people’s daily international communication. However, machine translation systems are far from perfect. To tackle this problem, researchers have proposed several approaches to testing machine translation. A promising trend among these approaches is to use word replacement, where only one word in the original sentence is replaced with another word to form a sentence pair. However, precise control of the impact of word replacement remains an outstanding issue in these approaches. 

To address this issue, we propose CAT, a novel word-replacement-based approach, whose basic idea is to identify word replacement with controlled impact (referred to as isotopic replacement). To achieve this purpose, we use a neural-based language model to encode the sentence context, and design a neural-network-based algorithm to evaluate context-aware semantic similarity between two words. Furthermore, similar to TransRepair, a state-of-the-art word-replacement-based approach, CAT also provides automatic fixing of revealed bugs without model retraining. 

Our evaluation on Google Translate and Transformer indicates that CAT achieves significant improvements over TransRepair. In particular, 1) CAT detects seven more types of bugs than TransRepair; 2) CAT detects 129% more translation bugs than TransRepair; 3) CAT repairs twice more bugs than TransRepair, many of which may bring serious consequences if left unfixed; and 4) CAT has better efficiency than TransRepair in input generation (0.01s v.s. 0.41s) and comparable efficiency with TransRepair in bug repair (1.92s v.s. 1.34s).


The main file tree of CAT

```
.
├── Labeled data
│   ├── RQ1 Test Input Generation
│   ├── RQ2 Bug Detection
│   ├── RQ3 Bug Repair
│   └── Extended Analysis
├── TS
├── MutantGen-Test.py
├── MutantGen-Repair.py
├── Repair.py
├── Testing.py
├── NewThres
│   ├── TestGenerator-NMT
│   └── TestGenerator-NMTRep
└── NMT_zh_en0-8Mu
    ├── padTrans
    └── repair-new
```

The manual assessment results are in the ```Labeled data``` folder.

For Testing:
```
python3 Testing.py
```
After it, the results are in the ```NMT_zh_en0-8Mu/padTrans``` folder.

For Repair:
```
python3 Repair.py
```
After it, the results are in the ```TS/quickstart0/repair-NEW``` folder.

# Data

The ```LookUpTable.txt``` used in ```NMT_zh_en_0-8Mu/padTrans``` and ```NMT_zh_en_0-8Mu/repair-new``` is available at https://drive.google.com/file/d/1fjGpryzGohla0ZA4u7KDgRJeAHegy0A1/view?usp=sharing

# Dependenices
```
NLTK 3.2.1
Pytorch 1.6.1
Python 3.7
Ubuntu 16.04
Transformers 3.3.0
```
