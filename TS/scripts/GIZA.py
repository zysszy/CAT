#! /usr/bin/env python

import os
import sys
import time

root_dir = '/home/hlxsb/Downloads/Word_Align/TsinghuaAligner'
MKCLS = root_dir + '/bin/mkcls'
PLAIN_2_SNT = root_dir + '/bin/plain2snt.out'
SNT_2_COOC = root_dir + '/bin/snt2cooc.out'
GIZA = root_dir + '/bin/GIZA++'
CD = 'cd'
CP = 'cp'
LS = 'ls'
RM = 'rm -rf'
MV = 'mv'
MKDIR = 'mkdir'

def version():
	'''Display version.'''
	s = '----------------------------------------\n' + \
	    ' GIZA v0.1\n' + \
		' 2014/01/27 - 2014/01/27\n' + \
		' (c) 2014 Yang Liu\n' + \
		'----------------------------------------\n'
	print s

def train(srcFileName, trgFileName):
	'''Train IBM models.'''
	os.system(CP + ' ' + srcFileName + ' source')
	os.system(CP + ' ' + trgFileName + ' target')
	os.system(MKCLS + ' -c50 -n2 -psource -Vsource.vcb.classes opt')
	os.system(MKCLS + ' -c50 -n2 -ptarget -Vtarget.vcb.classes opt')
	os.system(PLAIN_2_SNT + ' source target')
	os.system(SNT_2_COOC + ' source.vcb target.vcb source_target.snt > source_target.cooc')
	os.system(SNT_2_COOC + ' target.vcb source.vcb target_source.snt > target_source.cooc')
	os.system(GIZA + ' -S source.vcb -T target.vcb -C source_target.snt -CoocurrenceFile source_target.cooc')
	os.system(MV + ' *.t3.final source_target.tTable')
	os.system(GIZA + ' -S target.vcb -T source.vcb -C target_source.snt -CoocurrenceFile target_source.cooc')
	os.system(MV + ' 1*.t3.final target_source.tTable')
	os.system(RM + ' 1* source target *.cooc *.snt *.classes *.cats')

if __name__ == '__main__':
	version()
	if len(sys.argv) != 3:
		print 'Usage: GIZA srcFile trgFile'
		sys.exit()
	train(sys.argv[1], sys.argv[2])
