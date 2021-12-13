#! /usr/bin/env python

import sys
import os
import copy
import time
import random

root_dir = ''
GEN_NOISE =  root_dir + '/bin/genNoise.py'
TRAINER = root_dir + '/bin/trainer'

def version():
	'''Display version.'''
	s = '----------------------------------------\n' + \
	    ' unsupervisedTraining v0.1\n' + \
		' 2014/09/07 - 2014/09/07\n' + \
		' (c) 2014 Yang Liu\n' + \
		'----------------------------------------\n'
	print s

def help():
	'''Display helping message.'''
	s = 'Usage: unsupervisedTraining [--help] ...\n' + \
	    'Required arguments:\n' + \
		'  --src-file <src_file>                  source file\n' + \
		'  --trg-file <trg_file>                  target file\n'  + \
		'  --src-vcb-file <src_vcb_file>          source vocabulary file\n' + \
		'  --trg-vcb-file <trg_vcb_file>          target vocabulary file\n' + \
		'  --s2t-ttable-file <s2t_ttable_file>    source-to-target TTable file\n' + \
		'  --t2s-ttable-file <t2s_ttable_file>    target-to-source TTable file\n' + \
		'Optional arguments:\n' + \
		'  --training-corpus-size [1, +00)        training corpus size (default: 10)\n' + \
		'  --sent-length-limit [1, +00)           sentence length limit (default: 100)\n' + \
		'  --shuffle {0, 1}                       generating noises by shuffling words randomly (default: 1)\n' + \
		'  --replace {0, 1}                       generating noises by replacing words randomly (default: 0)\n' + \
		'  --insert {0, 1}                        generating noises by inserting words randomly (default: 0)\n' + \
		'  --delete {0, 1}                        generating noises by deleting words randomly (default: 0)\n' + \
		'  --beam-size [1, +00)                   beam size (default: 5)\n' + \
		'  --sample-size [1, +00)                 sample size (deault: 10)\n' + \
		'  --sgd-iter-limit [1, +00)              SGD iteration limit (default: 100)\n' + \
		'  --sgd-converge-threshold (0, +00)      SGD convergence threshold (default: 0.01)\n' + \
		'  --sgd-converge-limit [1, +00)          SGD convergence limit (default: 3)\n' + \
		'  --sgd-lr-numerator (0, +00)            SGD learning rate numerator (default: 1.0)\n' + \
		'  --sgd-lr-denominator (0, +00)          SGD learning rate denominator (default: 1.0)\n'
	print s
	sys.exit()

def select_training_corpus(srcFileName, \
	                       trgFileName, \
					       trainingCorpusSize, \
						   sentLengthLimit):
	'''Randomly select training corpus.'''
	# build candidate set
	candList = []
	f1 = open(srcFileName)
	f2 = open(trgFileName)
	sentID = 0
	while True:
		line1 = f1.readline()
		line2 = f2.readline()
		sentID += 1
		if line1 == '' or \
		   line2 == '':
			break
		if len(line1.split()) <= sentLengthLimit and \
		   len(line2.split()) <= sentLengthLimit:
			candList.append(sentID)
	# randomly select training corpus
	idList = random.sample(candList, min(len(candList), trainingCorpusSize))
	idList.sort()
	f3 = open(srcFileName)
	f4 = open(trgFileName)
	f5 = open('source.txt', 'w')
	f6 = open('target.txt', 'w')
	subscript = 0
	sentID = 0
	while True:
		line1 = f3.readline()
		line2 = f4.readline()
		sentID += 1
		if line1 == '' or \
		   line2 == '':
			break
		if sentID == idList[subscript]:
			f5.write(line1)
			f6.write(line2)
			subscript += 1
			if subscript == len(idList):
				break

def gen_trainer_ini(srcVcbFileName, \
                    trgVcbFileName, \
	         		s2tTTableFileName, \
			        t2sTTableFileName, \
			        beamSize, \
		        	sampleSize, \
			        sgdIterLimit, \
			        sgdConvergeThreshold, \
			        sgdConvergeLimit, \
			        sgdLRNumerator, \
			        sgdLRDenominator):
	'''Generate initialization file.'''
	f = open('trainer.ini', 'w')
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
			'[target sibling distance feature weight] -1.0\n' + \
			'[one-to-one link count feature weight] 1.0\n' + \
			'[one-to-many link count feature weight] 1.0\n' + \
			'[many-to-one link count feature weight] -1.0\n' + \
			'[many-to-many link count feature weight] -1.0\n' + \
			'\n# search setting\n' + \
			'[beam size] ' + str(beamSize) + '\n' + \
			'\n# sampling setting\n' + \
			'[sample size] ' + str(sampleSize) + '\n' + \
			'\n# SGD setting\n' + \
			'[SGD iteration limit] ' + str(sgdIterLimit) + '\n' + \
			'[SGD convergence threshold] ' + str(sgdConvergeThreshold) + '\n' + \
			'[SGD convergence limit] ' + str(sgdConvergeLimit) + '\n' + \
			'[SGD learning rate numerator] ' + str(sgdLRNumerator) + '\n' + \
			'[SGD learning rate denominator] ' + str(sgdLRDenominator))

def gen_aligner_ini(srcVcbFileName, \
	                trgVcbFileName, \
				    s2tTTableFileName, \
					t2sTTableFileName, \
					weightFileName):
	'''Generate aligner initialization file.'''
	# get weight list
	weightList = []
	f1 = open(weightFileName)
	while True:
		line = f1.readline()
		if line == '':
			break
		for w in line.split():
			weight = float(w)
			weightList.append(weight)
	# generate ini file
	f2 = open('TsinghuaAligner.ini', 'w')
	f2.write('# knowledge sources\n' + \
	         '[source vocabulary file] ' + srcVcbFileName + '\n' + \
			 '[target vocabulary file] ' + trgVcbFileName + '\n' + \
			 '[source-to-target translation probability table file] ' + s2tTTableFileName + '\n' + \
			 '[target-to-source translation probability table file] ' + t2sTTableFileName + '\n' + \
			 '\n# feature weights\n' + \
			 '[translation probability product feature weight] ' + str(weightList[0]) + '\n' + \
			 '[link count feature weight] ' + str(weightList[1]) + '\n' + \
			 '[relative position absolute distance feature weight] ' + str(weightList[2]) + '\n' + \
			 '[cross count feature weight] ' + str(weightList[3]) + '\n' + \
			 '[mono neighbor count feature weight] ' + str(weightList[4]) + '\n' + \
			 '[swap neighbor count feature weight] ' + str(weightList[5]) + '\n' + \
			 '[source linked word count feature weight] ' + str(weightList[6]) + '\n' + \
			 '[target linked word count feature weight] ' + str(weightList[7]) + '\n' + \
			 '[source maximal fertility feature weight] ' + str(weightList[8]) + '\n' + \
			 '[target maximal fertility feature weight] ' + str(weightList[9]) + '\n' + \
			 '[source sibling distance feature weight] ' + str(weightList[10]) + '\n' + \
			 '[target sibling distance feature weight] ' + str(weightList[11]) + '\n' + \
			 '[one-to-one link count feature weight] ' + str(weightList[12]) + '\n' + \
			 '[one-to-many link count feature weight] ' + str(weightList[13]) + '\n' + \
			 '[many-to-one link count feature weight] ' + str(weightList[14]) + '\n' + \
			 '[many-to-many link count feature weight] ' + str(weightList[15]) + '\n' + \
			 '\n# search setting\n' + \
			 '[beam size] 1' + \
			 '\n# structural constraint\n' + \
			 '# 0: arbitrary\n' + \
			 '# 1: itg\n' + \
			 '# 2: bitg\n' + \
			 '[structural constraint] 2\n' + \
			 '\n# speed-up setting' + '\n' + \
			 '[enable pre-pruning] 1\n' + \
			 '[pre-pruning threshold] 0')

def unsupervised_training(srcFileName, \
	                      trgFileName, \
			    		  srcVcbFileName, \
				    	  trgVcbFileName, \
					   	  s2tTTableFileName, \
    					  t2sTTableFileName, \
	    				  trainingCorpusSize, \
    					  sentLengthLimit, \
    					  shuffle, \
    					  replace, \
    					  insert, \
    					  delete, \
    					  beamSize, \
    					  sampleSize, \
    					  sgdIterLimit, \
    					  sgdConvergeThreshold, \
    					  sgdConvergeLimit, \
    					  sgdLRNumerator, \
    					  sgdLRDenominator):
	'''Unsupervised training.'''
	# randomly select training corpus
	print '\nStep 1: randomly select training corpus'
	select_training_corpus(srcFileName, \
	                       trgFileName, \
						   trainingCorpusSize, \
						   sentLengthLimit)
	# generate noises
	print '\nStep 2: generate noises'
	os.system(GEN_NOISE + \
	          ' --src-file source.txt' + \
			  ' --trg-file target.txt' + \
			  ' --trn-file data.txt' + \
			  ' --shuffle ' + str(shuffle) + \
			  ' --replace ' + str(replace) + \
			  ' --insert ' + str(insert) + \
			  ' --delete ' + str(delete))
	# generate trainer initialization file
	print '\nStep 3: generate training initialization file'
	gen_trainer_ini(srcVcbFileName, \
	                trgVcbFileName, \
			        s2tTTableFileName, \
			        t2sTTableFileName, \
			        beamSize, \
			        sampleSize, \
			        sgdIterLimit, \
			        sgdConvergeThreshold, \
			        sgdConvergeLimit, \
			        sgdLRNumerator, \
			        sgdLRDenominator)
	# unsupervised training
	print '\nStep 4: contrastive unsupervised training with top-n sampling'
	os.system(TRAINER + ' trainer.ini data.txt weights.txt')
	# generate TsinghuaAligner initialization file
	gen_aligner_ini(srcVcbFileName, \
	                trgVcbFileName, \
				    s2tTTableFileName, \
					t2sTTableFileName, \
					'weights.txt')
	# clean
	print '\nStep 5: clean temporary files'
	os.system('rm source.txt target.txt data.txt trainer.ini weights.txt')

if __name__ == '__main__':
	# display version	
	version()
	# initialize arguments
	srcFileName = ''             # source file name
	trgFileName = ''             # target file name
	srcVcbFileName = ''          # source vocabulary file name
	trgVcbFileName = ''          # target vocabulary file name
	s2tTTableFileName = ''       # source-to-target TTable file name
	t2sTTableFileName = ''       # target-to-source TTable file name
	trainingCorpusSize = 10      # training corpus size
	sentLengthLimit = 100        # sentence length limit of training corpus
	shuffle = 1                  # generating noises by shffuling words randomly
	replace = 0                  # generating noises by replacing words randomly
	insert = 0                   # generating noises by inserting words randomly
	delete = 0                   # generating noises by deleting words randomly
	beamSize = 5                 # beam size
	sampleSize = 10              # sample size
	sgdIterLimit = 100           # SGD iteration limit
	sgdConvergeThreshold = 0.01  # SGD convergence threshold
	sgdConvergeLimit = 3         # SGD convergence limit
	sgdLRNumerator = 1.0         # SGD learning rate numerator
	sgdLRDenominator = 1.0       # SGD learning rate denominator
	# analyze command-line arguments
	i = 1
	while i < len(sys.argv):
		if sys.argv[i] == '--src-file':
			srcFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--trg-file':
			trgFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--src-vcb-file':
			srcVcbFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--trg-vcb-file':
			trgVcbFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--s2t-ttable-file':
			s2tTTableFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--t2s-ttable-file':
			t2sTTableFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--training-corpus-size':
			trainingCorpusSize = int(sys.argv[i + 1])
		elif sys.argv[i] == '--sent-length-limit':
			sentLengthLimit = int(sys.argv[i + 1])
		elif sys.argv[i] == '--shuffle':
			shuffle = int(sys.argv[i + 1])
		elif sys.argv[i] == '--replace':
			replace = int(sys.argv[i + 1])
		elif sys.argv[i] == '--insert':
			insert = int(sys.argv[i + 1])
		elif sys.argv[i] == '--delete':
			delete = int(sys.argv[i + 1])
		elif sys.argv[i] == '--beam-size':
			beamSize = int(sys.argv[i + 1])
		elif sys.argv[i] == '--sample-size':
			sampleSize = int(sys.argv[i + 1])
		elif sys.argv[i] == '--sgd-iter-limit':
			sgdIterLimit = int(sys.argv[i + 1])
		elif sys.argv[i] == '--sgd-converge-threshold':
			sgdConvergeThreshold = float(sys.argv[i + 1])	
		elif sys.argv[i] == '--sgd-converge-limit':
			sgdConvergeLimit = int(sys.argv[i + 1])
		elif sys.argv[i] == '--sgd-lr-numerator':
			sgdLRNumerator = float(sys.argv[i + 1])
		elif sys.argv[i] == '--sgd-lr-denominator':
			sgdLRDenominator = float(sys.argv[i + 1])
		else:
			help()
		i += 2
	# check required arguments
	if srcFileName == '' or \
	   trgFileName == '' or \
	   srcVcbFileName == '' or \
	   trgVcbFileName == '' or \
	   s2tTTableFileName == '' or \
	   t2sTTableFileName == '':
		help()
	# unsupervised training
	unsupervised_training(srcFileName, \
	                      trgFileName, \
						  srcVcbFileName, \
						  trgVcbFileName, \
						  s2tTTableFileName, \
						  t2sTTableFileName, \
						  trainingCorpusSize, \
						  sentLengthLimit, \
						  shuffle, \
						  replace, \
						  insert, \
						  delete, \
						  beamSize, \
						  sampleSize, \
						  sgdIterLimit, \
						  sgdConvergeThreshold, \
						  sgdConvergeLimit, \
						  sgdLRNumerator, \
						  sgdLRDenominator)
