#ifndef TTABLE_H
#define TTABLE_H

#include "Vocab.h"

class TTable
{
public:
	/* member functions */
	// constructor
	TTable();
	// initalize
	void init(Vocab* srcVcbPtr,
	          Vocab* trgVcbPtr);
	// load
	void load(const char* fileName);
	// dump
	void dump(const char* fileName);
	// get probability
	float getProb(int srcID,
	              int trgID) const;
	float getProb(const string& srcWord,
	              const string& trgWord) const;

private:
	/* data members */
	map<pair<int, int>, float> tTable;  // translation probability table
	Vocab* srcVcbPtr;                   // source vocabulary pointer
	Vocab* trgVcbPtr;                   // target vocabulary pointer
	float probSmooth;                   // smoothing probability
};

#endif
