#ifndef TRAIN_H
#define TRAIN_H

#include <vector>
#include <set>
#include "Vocab.h"
#include "TTable.h"
#include "FeatureGroup.h"

class Train
{
public:
	/* member functions */
	// constructor
	Train();
	// train
	void train(const char* iniFileName,
	           const char* trnFileName,
			   const char* weightFileName);

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

	// sampling setting
	int sampleSize;                    // sample size

	// SGD setting
	int SGDIterationLimit;             // SGD iteration limit
	float SGDConvergenceThreshold;     // SGD convergence threshold
	int SGDConvergenceLimit;           // SGD convergence limit
	float SGDLearningRateNumerator;    // SGD learning rate numerator
	float SGDLearningRateDenominator;  // SGD learning rate denominator

	/* member functions */
	// initialize
	void init(const char* iniFileName,
	          const char* trgFileName);
	// build word sets
	void buildWordSets(const char* trnFileName,
	                   set<string>& srcWordSet,
					   set<string>& trgWordSet);
	// get training data
	void getTrainingData(const char* trnFileName,
	                     vector<vector<vector<int> > >& dataVec,
						 vector<int>& idVec);
	// compute expectation
	void computeExpectation(const vector<int>& srcIDVec,
	                        const vector<int>& trgIDVec,
							vector<float>& expectation);
	// add stack
	void addStack(const Alignment& a,
	              int limit,
				  vector<Alignment>& stack);
	// compute n-best list
	void computeNBestList(const vector<int>& srcIDVec,
	                      const vector<int>& trgIDVec,
						  vector<Alignment>& stack);
	// log sum
	float logSum(const vector<float>& v);
	// normalize weight vector
	void normalize(vector<float>& v);
	// dump weight vector
	void dump(const char* fileName,
	          const vector<float>& v);
};

#endif
