#ifndef SEARCH_H
#define SEARCH_H

#include <vector>
#include <set>
#include "Vocab.h"
#include "TTable.h"
#include "FeatureGroup.h"

class Search
{
public:
	/* member functions */
	// constructor
	Search();
	// align a file
	void alignFile(const char* iniFileName,
	               const char* srcFileName,
			       const char* trgFileName,
				   const char* agtFileName,
				   int nBestListSize,
				   int verboseLevel,
				   int posterior);

private:
	/* data members */
	// knowledge sources
	Vocab srcVcb;                      // source vocabulary
	Vocab trgVcb;                      // target vocabulary
	TTable s2tTTable;                  // source-to-target translation probability table
	TTable t2sTTable;                  // target-to-source translation probability table

	// features
	FeatureGroup fg;                   // feature group
	vector<float> weightVec;           // feature weight vector

	// search setting
	int beamSize;                      // beam size

	// structural constraint
	int structuralConstraint;          // structural constraint

	// speed-up setting
	int enablePrePruning;              // enable pre-pruning
	float prePruningThreshold;         // pre-pruning threshold

	/* member functions */
	// initialize
	void init(const char* iniFileName,
	          const char* srcFileName,
			  const char* trgFileName);
	// build word set
	void buildWordSet(const char* fileName,
	                   set<string>& wordSet);
	// get id vector
	void getIDVec(const string& line,
	              Vocab* vcbPtr,
	              vector<int>& idVec);
	// add stack
	void addStack(const Alignment& a,
	              int limit,
				  vector<Alignment>& stack);
	// compute n-best list
	void computeNBestList(const vector<int>& srcIDVec,
	                      const vector<int>& trgIDVec,
						  int nBestListSize,
						  int posterior,
						  vector<Alignment>& nBestList,
						  map<pair<int, int>, float>& posteriorMap);
	void computeNBestListSTD(const vector<int>& srcIDVec,
	                         const vector<int>& trgIDVec,
						     int nBestListSize,
							 int posterior,
						     vector<Alignment>& nBestList,
							 set<Alignment>& exploredSpace);
	void computeNBestListITG(const vector<int>& srcIDVec,
	                         const vector<int>& trgIDVec,
						     int nBestListSize,
							 int posterior,
						     vector<Alignment>& nBestList,
							 set<Alignment>& exploredSpace);
	void computeNBestListBITG(const vector<int>& srcIDVec,
	                          const vector<int>& trgIDVec,
						      int nBestListSize,
							  int posterior,
						      vector<Alignment>& nBestList,
							  set<Alignment>& exploredSpace);
	// compute posterior probabilities
	void computePosterior(const set<Alignment>& exploredSpace,
	                      map<pair<int, int>, float>& posterior);
	// compute log sum
	float logSum(const vector<float>& v) const;
};

#endif
