#! /usr/bin/env python

import sys
import copy
import random

def version():
	'''Display version.'''
	s = '----------------------------------------\n' + \
	    ' genNoise v0.1\n' + \
		' 2014/09/03 - 2014/09/03\n' + \
		' (c) 2014 Yang Liu\n' + \
		'----------------------------------------\n'
	print s

def help():
	'''Display helping message.'''
	s = 'Usage: genNoise [--help] ...\n' + \
	    'Required arguments:\n' + \
		'  --src-file <src_file>  source file\n' + \
		'  --trg-file <trg_file>  target file\n' + \
		'  --trn-file <trn_file>  training data file\n' + \
		'Optional arguments:\n' + \
		'  --shuffle {0, 1}       randomly shuffle words (default: 1)\n' + \
		'  --replace {0, 1}       randomly replace words (default: 0)\n' + \
		'  --delete {0, 1}        randomly delete words (default: 0)\n' + \
		'  --insert {0, 1}        randomly insert words (default: 0)\n' + \
		'  --help                 prints this message to STDOUT\n'
	print s
	sys.exit()

def get_word_set(fileName):
	'''Get word set.'''
	wordSet = []
	f = open(fileName)
	while True:
		line = f.readline()
		if line == '':
			break
		for w in line.split():
			if w not in wordSet:
				wordSet.append(w)
	return wordSet

def gen_noise_shuffle(srcWordList, \
                      trgWordList):
	'''Shuffle.'''
	swl = copy.deepcopy(srcWordList)
	twl = copy.deepcopy(trgWordList)
	random.shuffle(swl)
	random.shuffle(twl)
	return [(swl, twl)]

def gen_noise_replace(srcWordList, \
                      trgWordList, \
					  srcWordSet, \
					  trgWordSet):
	'''Replace.'''
	# copy
	swl = copy.deepcopy(srcWordList)
	twl = copy.deepcopy(trgWordList)
	# source
	i = random.randint(0, len(swl) - 1)
	j = random.randint(0, len(srcWordSet) - 1)
	swl[i] = srcWordSet[j]
	# target
	i = random.randint(0, len(twl) - 1)
	j = random.randint(0, len(trgWordSet) - 1)
	twl[i] = trgWordSet[j]
	return [(swl, twl)]

def gen_noise_delete(srcWordList, \
                     trgWordList):
	'''Delete.'''
	# copy
	swl = copy.deepcopy(srcWordList)
	twl = copy.deepcopy(trgWordList)
	# source
	i = random.randint(0, len(swl) - 1)
	swl.pop(i)
	# target
	i = random.randint(0, len(twl) - 1)
	twl.pop(i)
	return [(swl, twl)]

def gen_noise_insert(srcWordList, \
                     trgWordList, \
					 srcWordSet, \
					 trgWordSet):
	'''Insert.'''
	# copy
	swl = copy.deepcopy(srcWordList)
	twl = copy.deepcopy(trgWordList)
	# source 
	i = random.randint(0, len(swl))
	j = random.randint(0, len(srcWordSet) - 1)
	swl.insert(i, srcWordSet[j])
	# target
	i = random.randint(0, len(twl))
	j = random.randint(0, len(trgWordSet) - 1)
	twl.insert(i, trgWordSet[j])
	return [(swl, twl)]

def gen_noise(srcFileName, \
              trgFileName, \
			  trnFileName, \
			  shuffle, \
			  replace, \
			  delete, \
			  insert):
	'''Generate noises.'''
	# get word sets
	srcWordSet = get_word_set(srcFileName)
	trgWordSet = get_word_set(trgFileName)
	# generate noises
	f1 = open(srcFileName)
	f2 = open(trgFileName)
	f3 = open(trnFileName, 'w')
	while True:
		line1 = f1.readline()
		line2 = f2.readline()
		if line1 == '' or \
		   line2 == '':
			break
		srcWordList = line1.split()
		trgWordList = line2.split()
		noise = []
		if shuffle:
			noise.extend(gen_noise_shuffle(srcWordList, trgWordList))
		if replace:
			noise.extend(gen_noise_replace(srcWordList, trgWordList, srcWordSet, trgWordSet))
		if delete:
			noise.extend(gen_noise_delete(srcWordList, trgWordList))
		if insert:
			noise.extend(gen_noise_insert(srcWordList, trgWordList, srcWordSet, trgWordSet))
		for x in noise:
			f3.write(' '.join(srcWordList) + ' ||| ' + \
			         ' '.join(trgWordList) + ' ||| ' + \
					 ' '.join(x[0]) + ' ||| ' + \
					 ' '.join(x[1]) + '\n')

if __name__ == '__main__':
	# display version
	version()
	# initialize arguments
	srcFileName = ''
	trgFileName = ''
	trnFileName = ''
	shuffle = 1
	replace = 0
	delete = 0
	insert = 0
	# analyze command-line arguments
	i = 1
	while i < len(sys.argv):
		if sys.argv[i] == '--src-file':
			srcFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--trg-file':
			trgFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--trn-file':
			trnFileName = sys.argv[i + 1]
		elif sys.argv[i] == '--shuffle':
			shuffle = int(sys.argv[i + 1])
		elif sys.argv[i] == '--replace':
			replace = int(sys.argv[i + 1])
		elif sys.argv[i] == '--delete':
			delete = int(sys.argv[i + 1])
		elif sys.argv[i] == '--insert':
			insert = int(sys.argv[i + 1])
		else:
			help()
		i += 2
	# check required arguments
	if srcFileName == '' or \
	   trgFileName == '' or \
	   trnFileName == '':
		help()
	# generate noises
	gen_noise(srcFileName, \
	          trgFileName, \
			  trnFileName, \
			  shuffle, \
			  replace, \
			  delete, \
			  insert)
