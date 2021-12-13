#! /usr/bin/env python

import os
import sys
import copy
import time

root_dir = ''
TSINGHUA_ALIGNER = root_dir + '/bin/TsinghuaAligner'
CONVERT_NBEST_LIST_FORMAT = root_dir + '/bin/convertNBestListFormat'
GEN_INI = root_dir + '/bin/genIni'
MERGE_NBEST_LIST = root_dir +  '/bin/mergeNBestList'
OPTIMIZE_AER = root_dir +  '/bin/optimizeAER'
WA_EVAL = root_dir + '/bin/waEval'

def version():
	'''Display version.'''
	s = '----------------------------------------\n' + \
	    ' supervisedTraining v0.1\n' + \
		' 2014/09/06 - 2014/09/06\n' + \
		' (c) 2014 Yang Liu\n' + \
		'----------------------------------------\n'
	print s

def help():
	'''Display helping message.'''
	s = 'Usage: supervisedTraining [--help] ...\n' + \
	    'Required arguments:\n' + \
		'  --src-vcb-file <file>                  source vocabulary file\n' + \
		'  --trg-vcb-file <file>                  target vocabulary file\n' + \
		'  --s2t-ttable-file <file>               source-to-target TTable file\n' + \
		'  --t2s-ttable-file <file>               target-to-source TTable file\n' + \
		'  --dev-src-file <file>                  devset source file \n' + \
		'  --dev-trg-file <file>                  devset target file \n' + \
		'  --dev-agt-file <file>                  devset alignment file\n' + \
		'Optional arguments:\n' + \
		'  --iter-limit [1, +00)                  MERT iteration limit (default: 10)\n' + \
		'  --nbest-list-size [1, +00)             n-best list size (default: 100)\n' + \
		'  --beam-size [1, +00)                   beam size (default: 1)\n' + \
		'  --structural-constraint {0, 1, 2}      structural constraint\n' + \
		'                                            0: arbitrary (default)\n' + \
		'                                            1: ITG\n' + \
		'                                            2: BITG\n' + \
		'  --enable-prepruning {0, 1}             enable prepruning\n' + \
		'                                            0: disable (default)\n' + \
		'                                            1: enable\n' + \
		'  --prepruning-threshold (-00, +00)      prepruning threshold (default: 0)\n' + \
		'  --help                                 prints this message to STDOUT\n'
	print s
	sys.exit()

def gen_bound_file():
	'''Generate a bound file.'''
	f = open('bounds.txt', 'w')
	# lower bound
	f.write('0.1 -100 -100 -100 -100 -100 -100 -100 -100 -100 -100 -100 -100 -100 -100 -100\n')
	# upper bound
	f.write('100 100 100 100 100 100 100 100 100 100 100 100 100 100 100 100\n')
	# initial value
	f.write('1.0 1.0 -1.0 -1.0 1.0 -1.0 1.0 1.0 1.0 1.0 -1.0 1.0 1.0 1.0 1.0 -1.0\n')

def gen_ini_file(srcVcbFileName, \
                 trgVcbFileName, \
				 s2tTTableFileName, \
				 t2sTTableFileName, \
				 beamSize, \
				 structuralConstraint, \
				 enablePrePruning, \
				 prePruningThreshold):
	'''Generate the starting initialization file.'''
	f = open('TsinghuaAligner_iter1.ini', 'w')
	f.write('# knowledge sources\n' + \
	        '[source vocabulary file] ' + srcVcbFileName + '\n' + \
			'[target vocabulary file] ' + trgVcbFileName + '\n' + \
			'[source-to-target translation probability table file] ' + s2tTTableFileName + '\n' + \
			'[target-to-source translation probability table file] ' + t2sTTableFileName + '\n' + \
			'\n# feature weights\n' + \
			'[translation probability product feature weight] 1.0\n' + \
			'[link count feature weight] 1.0\n' + \
			'[relative position absolute distance feature weight] -1.0\n' + \
			'[cross count feature weight] -1.0\n' + \
			'[mono neighbor count feature weight] 1.0\n' + \
			'[swap neighbor count feature weight] -1.0\n' + \
			'[source linked word count feature weight] 1.0\n' + \
			'[target linked word count feature weight] 1.0\n' + \
			'[source maximal fertility feature weight] 1.0\n' + \
			'[target maximal fertility feature weight] 1.0\n' + \
			'[source sibling distance feature weight] -1.0\n' + \
			'[target sibling distance feature weight] 1.0\n' + \
			'[one-to-one link count feature weight] 1.0\n' + \
			'[one-to-many link count feature weight] 1.0\n' + \
			'[many-to-one link count feature weight] 1.0\n' + \
			'[many-to-many link count feature weight] -1.0\n' + \
			'\n# search setting\n' + \
			'[beam size] ' + str(beamSize) + '\n' + \
			'\n# structural constraint\n' + \
			'# 0: arbitrary\n' + \
			'# 1: ITG\n' + \
			'# 2: BITG\n' + \
			'[structural constraint] ' + str(structuralConstraint) + '\n' + \
			'\n# speed-up setting\n' + \
			'[enable pre-pruning] ' + str(enablePrePruning) + '\n' + \
			'[pre-pruning threshold] ' + str(prePruningThreshold))

def get_AER(AERFileName):
	'''Get AER.'''
	aer = 0.0
	f = open(AERFileName)
	while True:
		line = f.readline()
		if line == '':
			break
		if '[AER]' in line:
			aer = float(line.split(']')[1].strip())
			break
	return aer

def get_weight_list(iniFileName):
	'''Get weight list.'''
	weightList = []
	f = open(iniFileName)
	while True:
		line = f.readline()
		if line == '':
			break
		if 'feature weight]' in line:
			weight = float(line.split(']')[1].strip())
			weightList.append(weight)
	return weightList

def mert(iniFileName, \
	     boundFileName, \
		 devSrcFileName, \
		 devTrgFileName, \
		 devAgtFileName, \
		 iterLimit, \
		 nBestListSize):
	'''Minimum error rate training.'''
	bestAER = 1.0
	bestIter = 1
	for iter in range(1, iterLimit + 1):
		# generate n-best list
		os.system(TSINGHUA_ALIGNER + \
		          ' --ini-file ' + 'TsinghuaAligner_iter' + str(iter) + '.ini' + \
				  ' --src-file ' + devSrcFileName + \
				  ' --trg-file ' + devTrgFileName + \
				  ' --agt-file ' + 'alignment_iter' + str(iter) + '.txt' + \
				  ' --nbest-list ' + str(nBestListSize))
		# compute AER
		os.system(WA_EVAL + ' ' + \
		          devAgtFileName + ' ' + \
				  'alignment_iter' + str(iter) + '.txt' + \
				  ' > AER_iter' + str(iter) + '.txt')
		# update best AER and best weight list
		AER = get_AER('AER_iter' + str(iter) + '.txt')
		weightList = get_weight_list('TsinghuaAligner_iter' + str(iter) + '.ini')
		if AER < bestAER:
			bestAER = AER
			bestIter = iter
		if iter == iterLimit:
			continue
		# merge n-best list
		if iter == 1:
			os.system('mv nBestList.txt nBestList1.txt')
		else:
			os.system(MERGE_NBEST_LIST + \
			          ' nBestList' + str(iter - 1) + '.txt' + \
			          ' nBestList.txt' + \
					  ' nBestList' + str(iter) + '.txt')
		# convert n-best list format
		os.system(CONVERT_NBEST_LIST_FORMAT + \
		          ' ' + devAgtFileName + \
				  ' nBestList' + str(iter) + '.txt' + \
				  ' cands_' + str(iter) + '.txt' + \
				  ' feats_' + str(iter) + '.txt')
		# minimum error rate training
		os.system(OPTIMIZE_AER + \
		          ' bounds.txt' + \
				  ' cands_' + str(iter) + '.txt' + \
				  ' feats_' + str(iter) + '.txt' + \
				  ' weights_' + str(iter) + '.txt')
		# generate the initialization file for the next iteration
		os.system(GEN_INI + \
		          ' -o TsinghuaAligner_iter1.ini' + \
				  ' -l weights_' + str(iter) + '.txt' + \
				  ' -n TsinghuaAligner_iter' + str(iter + 1) + '.ini')
	# generate the final initialization file
	os.system('cp TsinghuaAligner_iter' + str(bestIter) + '.ini' + \
		      ' TsinghuaAligner.ini')

	# show
	print '\n\nbest AER:', bestAER
	print 'best iteration:', bestIter
	print 'best weight:', get_weight_list('TsinghuaAligner_iter' + str(bestIter) + '.ini')
	# clean
	os.system('rm *.txt TsinghuaAligner_*.ini')

def supervised_training(srcVcbFileName, \
                        trgVcbFileName, \
						s2tTTableFileName, \
						t2sTTableFileName, \
						devSrcFileName, \
						devTrgFileName, \
						devAgtFileName, \
						iterLimit, \
						nBestListSize, \
						beamSize,
						structuralConstraint, \
						enablePrePruning, \
						prePruningThreshold):
	'''Supervised learning.'''
	# generate the bound file
	gen_bound_file()
	# generate the starting initialization file
	gen_ini_file(srcVcbFileName, \
	             trgVcbFileName, \
				 s2tTTableFileName, \
				 t2sTTableFileName, \
				 beamSize, \
				 structuralConstraint, \
				 enablePrePruning, \
				 prePruningThreshold)
	# minimum error rate training
	mert('TsinghuaAligner_iter1.ini', \
	     'bounds.txt', \
		 devSrcFileName, \
		 devTrgFileName, \
		 devAgtFileName, \
		 iterLimit, \
		 nBestListSize)

if __name__ == '__main__':
	version()
	# initialize arguments
	srcVcbFileName = ''        # the source vocabulary file
	trgVcbFileName = ''        # the target vocabulary file
	s2tTTableFileName = ''     # source-to-target TTable file name
	t2sTTableFileName = ''     # target-to-source TTable file name
	devSrcFileName = ''        # the source file in the development set
	devTrgFileName = ''        # the target file in the development set
	devAgtFileName = ''        # the alignment file in the development set
	iterLimit = 10             # iteration limit
	nBestListSize = 100        # n-best list size
	beamSize = 1               # beam size
	structuralConstraint = 0   # structural constraint
	enablePrePruning = 0       # enable pre-pruning
	prePruningThreshold = 0.0  # pre-pruning threshold
	# analyze command-line arguments
	i = 1
	while i < len(sys.argv):
		if sys.argv[i] == '--src-vcb-file':
			srcVcbFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--trg-vcb-file':
			trgVcbFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--s2t-ttable-file':
			s2tTTableFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--t2s-ttable-file':
			t2sTTableFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--dev-src-file':
			devSrcFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--dev-trg-file':
			devTrgFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--dev-agt-file':
			devAgtFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--iter-limit':
			iterLimit = int(sys.argv[i + 1])
		elif sys.argv[i] == '--nbest-list-size':
			nBestListSize = int(sys.argv[i + 1])
		elif sys.argv[i] == '--beam-size':
			beamSize = int(sys.argv[i + 1])
		elif sys.argv[i] == '--structural-constraint':
			structuralConstraint = int(sys.argv[i + 1])
		elif sys.argv[i] == '--enable-prepruning':
			enablePrePruning = int(sys.argv[i + 1])
		elif sys.argv[i] == '--prepruning-threshold':
			prePruningThreshold = float(sys.argv[i + 1])
		else:
			help()
		i += 2
	# check required arguments
	if srcVcbFileName == '' or \
	   trgVcbFileName == '' or \
	   s2tTTableFileName == '' or \
	   t2sTTableFileName == '' or \
	   devSrcFileName == '' or \
	   devTrgFileName == '' or \
	   devAgtFileName == '':
		help()
	# supervised training
	supervised_training(srcVcbFileName, \
	                    trgVcbFileName, \
						s2tTTableFileName, \
						t2sTTableFileName, \
						devSrcFileName, \
						devTrgFileName, \
						devAgtFileName, \
						iterLimit, \
						nBestListSize, \
						beamSize, \
						structuralConstraint, \
						enablePrePruning, \
						prePruningThreshold)
