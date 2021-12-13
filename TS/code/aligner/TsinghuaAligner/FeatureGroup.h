#ifndef FEATURE_GROUP_H
#define FEATURE_GROUP_H

#include "FeatureTPP.h"
#include "FeatureLinkCount.h"
#include "FeatureRelPosAbsDist.h"
#include "FeatureCrossCount.h"
#include "FeatureNeighborCount.h"
#include "FeatureLinkedWordCount.h"
#include "FeatureMaxFertility.h"
#include "FeatureSibDist.h"
#include "FeatureLinkType.h"

class FeatureGroup
{
public:
	/* member functions */
	// constructor
	FeatureGroup();
	// initialize
	void init(TTable* s2tTTablePtr,
	          TTable* t2sTTablePtr);
	// compute score
	void computeScore(const vector<int>& srcIDVec,
	                  const vector<int>& trgIDVec,
					  Alignment& alignment,
					  const vector<float>& weightVec) const;
	void computeScore(const vector<int>& srcIDVec,
	                  const vector<int>& trgIDVec,
					  Alignment& alignment,
					  const pair<int, int>& link,
					  const vector<float>& weightVec) const;

private:
	/* data members */
	// features
	FeatureTPP featTPP;              // translation probability product
	FeatureLinkCount featLC;         // link count
	FeatureRelPosAbsDist featRPAD;   // relative position absolute distance
	FeatureCrossCount featCC;        // cross count
	FeatureNeighborCount featNC;     // neighbor count
	FeatureLinkedWordCount featLWC;  // linked word count
	FeatureMaxFert featMF;           // maximal fertility
	FeatureSibDist featSD;           // sibling distance
	FeatureLinkType featLT;          // link type

	/* member functions */
	// dot product
	float dotProduct(const vector<float>& v1,
	                 const vector<float>& v2) const;
};

#endif
