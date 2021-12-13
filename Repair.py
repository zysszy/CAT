import os

os.system("cp ./NMT_zh_en0-8Mu/padTrans/bugs* ./NewThres/TestGenerator-NMTRep")
os.system("cd ./NewThres/TestGenerator-NMTRep && sh gentest.sh")
os.system("cp ./NewThres/TestGenerator-NMTRep/bugs* ./NewThres/TestGenerator-NMTRep/*Mu* ./NewThres/TestGenerator-NMTRep/index* ./NMT_zh_en0-8Mu/repair-new")
os.system("cd ./NMT_zh_en0-8Mu/repair-new && sh desp.sh")
os.system("python3 ./pre_repair.py")
os.system("cd TS/quickstart0/repairALL-NEW && python3 repairALL.py")
os.system("cd TS/quickstart0/repairALL-NEW && sh test.sh")
