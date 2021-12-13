python3 pre_align.py LCS_Mu.zh LCS_Mu_align.zh
python3 pre_align.py ED_Mu.zh ED_Mu_align.zh
python3 pre_align.py TFIDF_Mu.zh TFIDF_Mu_align.zh
python3 pre_align.py BLEU_Mu.zh BLEU_Mu_align.zh
python3 read_index.py index.LCS index_Mu.LCS #BLEU_Mu_align.zh
python3 read_index.py index.ED index_Mu.ED #BLEU_Mu_align.zh
python3 read_index.py index.TFIDF index_Mu.TFIDF #BLEU_Mu_align.zh
python3 read_index.py index.BLEU index_Mu.BLEU #BLEU_Mu_align.zh
