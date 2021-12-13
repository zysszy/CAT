import os

os.system("python3 ./MutantGen-Test.py")
os.system("cp NewThres/TestGenerator-NMT/*.txt NewThres/TestGenerator-NMT/*.index ./NMT_zh_en0-8Mu/padTrans")
os.system("cd ./NMT_zh_en0-8Mu/padTrans && sh desp.sh")
os.system("cd ./NMT_zh_en0-8Mu/padTrans && sh test.sh")
