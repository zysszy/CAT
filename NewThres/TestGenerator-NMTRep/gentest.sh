#python3 bertMu.py bugs_LCS.txt Test.en
#exit
python3 bertMu.py bugs_LCS.txt LCS_Mu.en index.LCS
#python3 bertMuFilter.py LCS_Mu.en LCSMu.en

python3 bertMu.py bugs_ED.txt ED_Mu.en index.ED
#python3 bertMuFilter.py ED_Mu.en EDMu.en

python3 bertMu.py bugs_TFIDF.txt TFIDF_Mu.en index.TFIDF
#python3 bertMuFilter.py TFIDF_Mu.en TFIDFMu.en

python3 bertMu.py bugs_BLEU.txt BLEU_Mu.en index.BLEU
#python3 bertMuFilter.py BLEU_Mu.en BLEUMu.en
