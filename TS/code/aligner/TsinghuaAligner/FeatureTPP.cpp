#include "FeatureTPP.h"

/************************************************
  constructor
************************************************/
FeatureTPP::FeatureTPP()
{
}

/************************************************
  initialize
************************************************/
void FeatureTPP::init(TTable* s2tTTablePtr,
	                  TTable* t2sTTablePtr)
{
	this->s2tTTablePtr = s2tTTablePtr;
	this->t2sTTablePtr = t2sTTablePtr;
}

/************************************************
  compute feature value
************************************************/
float FeatureTPP::computeValue(const vector<int>& srcIDVec,
	                           const vector<int>& trgIDVec,
					           const Alignment& alignment) const
{
	float value = 0.0;

	// aligned word pairs
	for (int i = 0; i < (int)alignment.linkVec.size(); i++)
	{
		int srcID = srcIDVec[alignment.linkVec[i].first],
		    trgID = trgIDVec[alignment.linkVec[i].second];

		value += s2tTTablePtr->getProb(srcID, trgID) + 
		         t2sTTablePtr->getProb(trgID, srcID);
	}

	// unaligned source words
	for (int i = 1; i < (int)srcIDVec.size(); i++)
	{
		if (alignment.srcAligned[i].empty())
		{
			value += t2sTTablePtr->getProb(trgIDVec[0], srcIDVec[i]);
		}
	}

	// unaligned target words
	for (int i = 1; i < (int)trgIDVec.size(); i++)
	{
		if (alignment.trgAligned[i].empty())
		{
			value += s2tTTablePtr->getProb(srcIDVec[0], trgIDVec[i]);
		}
	}

	return value;
}

/************************************************
  compute feature gain
************************************************/
float FeatureTPP::computeGain(const vector<int>& srcIDVec,
	                          const vector<int>& trgIDVec,
					          const Alignment& alignment,
					          const pair<int, int>& link) const
{
	float gain = 0.0;

	// aligned word pairs
	gain += s2tTTablePtr->getProb(srcIDVec[link.first], trgIDVec[link.second]) + 
	        t2sTTablePtr->getProb(trgIDVec[link.second], srcIDVec[link.first]);

	// unaligned source words
	if (alignment.srcAligned[link.first].empty())
	{
		gain -= t2sTTablePtr->getProb(trgIDVec[0], srcIDVec[link.first]);
	}

	// unaligned target words
	if (alignment.trgAligned[link.second].empty())
	{
		gain -= s2tTTablePtr->getProb(srcIDVec[0], trgIDVec[link.second]);
	}

	return gain;
}

