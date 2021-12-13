#ifndef FEATURE_TPP_H
#define FEATURE_TPP_H

#include "TTable.h"
#include "Alignment.h"

class FeatureTPP
{
public:
	/* member functions */
	// constructor
	FeatureTPP();
	// initialize
	void init(TTable* s2tTTablePtr,
	          TTable* t2sTTablePtr);
	// compute feature value
	float computeValue(const vector<int>& srcIDVec,
	                   const vector<int>& trgIDVec,
					   const Alignment& alignment) const;
	// compute feature gain
	float computeGain(const vector<int>& srcIDVec,
	                  const vector<int>& trgIDVec,
					  const Alignment& alignment,
					  const pair<int, int>& link) const;

private:
	/* data members */
	TTable* s2tTTablePtr;  // source-to-target translation probability table pointer
	TTable* t2sTTablePtr;  // target-to-source translation probability table pointer
};

#endif
