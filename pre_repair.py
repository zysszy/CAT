import os
import time

ft = time.time()
#metrics = ["LCS", "ED", "TFIDF", "BLEU"]
metrics = ["LCS", "ED", "TFIDF", "BLEU"]
titles = ["NMT_zh_en0-8Mu"]
repdir = ["repair-new"]
qk = "quickstart0"
def newrepair():
    #os.system(f"cp NMT_zh_en0-8Mu/repair/pre_align.sh")
    for title in titles: 
        for rdir in repdir:
            os.system(f"cd {title}/{rdir} && sh pre_align.sh")
            for met in metrics:
                os.system(f"cp {title}/{rdir}/{met}_Mu_align.zh TS/{qk}/chinese_e.txt")
                os.system(f"cp {title}/{rdir}/{met}_Mu.en TS/{qk}/english_e.txt")
                os.system(f"cp {title}/{rdir}/{met}_Mu.* TS/{qk}/repairALL-NEW/")
                os.system(f"cp {title}/{rdir}/index.{met} TS/{qk}/repairALL-NEW/")

                #os.system(f"cd TS/{qk} && sh do.sh && python3 read.py")
                os.system(f"cd TS && python3 startall.py")
                os.system(f"cd TS/{qk} && python3 read.py")
                os.system(f"cp TS/{qk}/word_alignment.txt TS/{qk}/repairALL-NEW/align_ori.{met}.txt")
                os.system(f"cp TS/{qk}/word_alignment.txt TS/{qk}/repairALL-NEW/align_ori.{met}.txt")
                os.system(f"cp ./NewThres/TestGenerator-NMTRep/bugs* TS/{qk}/repairALL-NEW/")

newrepair()
print (time.time() - ft)
