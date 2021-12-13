#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include "Train.h"

/************************************************
  constructor
************************************************/
Train::Train()
{
	// plant the seed
	srand(time(NULL));
}

/************************************************
  initialize
************************************************/
void Train::init(const char* iniFileName,
	             const char* trnFileName)
{
	ifstream in(iniFileName);

	if (!in)
	{
		cerr << "\nERROR at [Train::init]: "
		     << "cannot open file \""
			 << iniFileName
			 << "\"!"
			 << endl;

		exit(1);
	}

	string line,
	       srcVcbFileName,
		   trgVcbFileName,
		   s2tTTableFileName,
		   t2sTTableFileName;

	while (getline(in, line))
	{
		// source vocalubary file name
		if (line.find("[source vocabulary file]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> srcVcbFileName;
		}
		
		// target vocalubary file name
		if (line.find("[target vocabulary file]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> trgVcbFileName;
		}

		// source-to-target translation probability table file name
		if (line.find("[source-to-target translation probability table file]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> s2tTTableFileName;
		}

		// target-to-source translation probability table file name
		if (line.find("[target-to-source translation probability table file]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> t2sTTableFileName;
		}

		// feature weights
		if (line.find("feature weight]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			string w;
			iss >> w;
			float weight = atof(w.c_str());
			weightVec.push_back(weight);
		}

		// beam size
		if (line.find("[beam size]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> beamSize;
		}

		// sample size
		if (line.find("[sample size]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> sampleSize;
		}
		
		// SGD iteration limit
		if (line.find("[SGD iteration limit]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> SGDIterationLimit;
		}
		
		// SGD convergence threshold
		if (line.find("[SGD convergence threshold]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> SGDConvergenceThreshold;
		}

		// SGD convergence limit
		if (line.find("[SGD convergence limit]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> SGDConvergenceLimit;
		}
		
		// SGD learning rate numerator
		if (line.find("[SGD learning rate numerator]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> SGDLearningRateNumerator;
		}

		// SGD learning rate denominator
		if (line.find("[SGD learning rate denominator]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> SGDLearningRateDenominator;
		}
	}

	// build word sets
	set<string> srcWordSet,
	            trgWordSet;
	buildWordSets(trnFileName, srcWordSet, trgWordSet);

	// initialize knowledge sources
	srcVcb.load(srcVcbFileName.c_str(), srcWordSet);
	trgVcb.load(trgVcbFileName.c_str(), trgWordSet);

	s2tTTable.init(&srcVcb, &trgVcb);
	s2tTTable.load(s2tTTableFileName.c_str());

	t2sTTable.init(&trgVcb, &srcVcb);
	t2sTTable.load(t2sTTableFileName.c_str());

	fg.init(&s2tTTable, &t2sTTable);
}

/************************************************
  build word sets
************************************************/
void Train::buildWordSets(const char* trnFileName,
	                      set<string>& srcWordSet,
			    		  set<string>& trgWordSet)
{
	ifstream in(trnFileName);
	string line;

	while (getline(in, line))
	{
		if (line.empty())
		{
			continue;
		}

		// observed source sentence
		int spp1 = 0,
		    spp2 = line.find(" ||| ", spp1);
		istringstream iss1(line.substr(spp1, spp2 - spp1).c_str());
		string w;

		while (iss1 >> w)
		{
			srcWordSet.insert(w);
		}

		// observed target sentence
		spp1 = spp2 + 5;
		spp2 = line.find(" ||| ", spp1);
		istringstream iss2(line.substr(spp1, spp2 - spp1).c_str());

		while (iss2 >> w)
		{
			trgWordSet.insert(w);
		}

		// noisy source sentence
		spp1 = spp2 + 5;
		spp2 = line.find(" ||| ", spp1);
		istringstream iss3(line.substr(spp1, spp2 - spp1).c_str());

		while (iss3 >> w)
		{
			srcWordSet.insert(w);
		}

		// noisy target sentence
		spp1 = spp2 + 5;
		spp2 = (int)line.size();
		istringstream iss4(line.substr(spp1, spp2 - spp1).c_str());

		while (iss4 >> w)
		{
			trgWordSet.insert(w);
		}
	}

	if (0)
	{
		cout << "The source word set is: ";

		set<string>::const_iterator iter;

		for (iter = srcWordSet.begin(); iter != srcWordSet.end(); iter++)
		{
			cout << *iter
			     << " ";
		}

		cout << "\nThe target word set is: ";

		for (iter = trgWordSet.begin(); iter != trgWordSet.end(); iter++)
		{
			cout << *iter
			     << " ";
		}

		cout << endl;
	}
}

/*************************************************
  get training data
*************************************************/
void Train::getTrainingData(const char* trnFileName,
	                        vector<vector<vector<int> > >& dataVec,
			    			vector<int>& idVec)
{
	// clear
	dataVec.clear();
	idVec.clear();

	ifstream in(trnFileName);
	string line;
	int id = 0;

	while (getline(in, line))
	{
		vector<int> observedSrcIDVec,
		            observedTrgIDVec,
					noisySrcIDVec,
					noisyTrgIDVec;
		string line1,
		       line2,
			   line3,
			   line4;

		// observed source id vector
		observedSrcIDVec.push_back(0);

		int spp1 = 0,
		    spp2 = line.find(" ||| ", spp1);
		istringstream iss1(line.substr(spp1, spp2 - spp1).c_str());
		string w;

		while (iss1 >> w)
		{
			int srcID = srcVcb.getID(w);
			observedSrcIDVec.push_back(srcID);
		}

		// observed target id vector
		observedTrgIDVec.push_back(0);

		spp1 = spp2 + 5;
		spp2 = line.find(" ||| ", spp1);
		istringstream iss2(line.substr(spp1, spp2 - spp1).c_str());

		while (iss2 >> w)
		{
			int trgID = trgVcb.getID(w);
			observedTrgIDVec.push_back(trgID);
		}

		// noisy source id vector
		noisySrcIDVec.push_back(0);

		spp1 = spp2 + 5;
		spp2 = line.find(" ||| ", spp1);
		istringstream iss3(line.substr(spp1, spp2 - spp1).c_str());

		while (iss3 >> w)
		{
			int srcID = srcVcb.getID(w);
			noisySrcIDVec.push_back(srcID);
		}

		// noisy target id vector
		noisyTrgIDVec.push_back(0);

		spp1 = spp2 + 5;
		spp2 = (int)line.size();
		istringstream iss4(line.substr(spp1, spp2 - spp1).c_str());

		while (iss4 >> w)
		{
			int trgID = trgVcb.getID(w);
			noisyTrgIDVec.push_back(trgID);
		}

		// update
		vector<vector<int> > v;
		v.push_back(observedSrcIDVec);
		v.push_back(observedTrgIDVec);
		v.push_back(noisySrcIDVec);
		v.push_back(noisyTrgIDVec);

		dataVec.push_back(v);
		idVec.push_back(id++);
	}
}

/************************************************
  train
************************************************/
void Train::train(const char* iniFileName,
	              const char* trnFileName,
				  const char* weightFileName)
{
	clock_t tb = clock();

	// initialize
	init(iniFileName, trnFileName);

	// get the training data
	vector<vector<vector<int> > > dataVec;
	vector<int> idVec;
	getTrainingData(trnFileName, dataVec, idVec);

	// stochastic gradient descent
	int t = 0,
	    convergenceCount = 0;
	bool converged = false;

	for (int iteration = 1; iteration <= SGDIterationLimit; iteration++)
	{
		cout << "\niteration "
		     << iteration
			 << endl;

		// randomly shuffle the training data
		random_shuffle(idVec.begin(), idVec.end());

		// go through each training example
		for (int subscript = 0; subscript < (int)idVec.size(); subscript++)
		{
			t++;

			int id = idVec[subscript];

			// get the training example
			vector<int> observedSrcIDVec = dataVec[id][0],
			            observedTrgIDVec = dataVec[id][1],
						noisySrcIDVec = dataVec[id][2],
						noisyTrgIDVec = dataVec[id][3];

			// compute expectations
			vector<float> observedExpectation,
			              noisyExpectation;

			computeExpectation(observedSrcIDVec, observedTrgIDVec, observedExpectation);
			computeExpectation(noisySrcIDVec, noisyTrgIDVec, noisyExpectation);

			if (0)
			{
				cout << "[observedSrcIDVec] ";

				for (int i = 0; i < (int)observedSrcIDVec.size(); i++)
				{
					cout << srcVcb.getWord(observedSrcIDVec[i])
					     << "/"
						 << observedSrcIDVec[i]
						 << " ";
				}

				cout << "\n[observedTrgIDVec] ";

				for (int i = 0; i < (int)observedTrgIDVec.size(); i++)
				{
					cout << trgVcb.getWord(observedTrgIDVec[i])
					     << "/"
						 << observedTrgIDVec[i]
						 << " ";
				}

				cout << "\n[observedExpectation] ";

				for (int i = 0; i < (int)observedExpectation.size(); i++)
				{
					cout << observedExpectation[i]
					     << " ";
				}

				cout << "\n[noisySrcIDVec] ";

				for (int i = 0; i < (int)noisySrcIDVec.size(); i++)
				{
					cout << srcVcb.getWord(noisySrcIDVec[i])
					     << "/"
						 << noisySrcIDVec[i]
						 << " ";
				}

				cout << "\n[noisyTrgIDVec] ";

				for (int i = 0; i < (int)noisyTrgIDVec.size(); i++)
				{
					cout << trgVcb.getWord(noisyTrgIDVec[i])
					     << "/"
						 << noisyTrgIDVec[i]
						 << " ";
				}

				cout << "\n[noisyExpectation] ";

				for (int i = 0; i < (int)noisyExpectation.size(); i++)
				{
					cout << noisyExpectation[i]
					     << " ";
				}
				
				cout << endl;
			}

			// compute learning rate
			float learningRate = SGDLearningRateNumerator / (t + SGDLearningRateDenominator);

			// update weight vector
			float diff = 0.0;

			for (int i = 0; i < (int)weightVec.size(); i++)
			{
				float change = learningRate * (observedExpectation[i] - noisyExpectation[i]);
				weightVec[i] += change;
				diff += pow(change, 2.0);
			}

			diff = sqrt(diff);

			// show
			cout << "<"
			     << t
				 << "> ";

			for (int i = 0; i < (int)weightVec.size(); i++)
			{
				cout << weightVec[i]
				     << " ";
			}

			cout << "("
			     << diff
				 << ")"
				 << endl;

			// convergence
			if (diff < SGDConvergenceThreshold)
			{
				convergenceCount++;
			}
			else
			{
				convergenceCount = 0;
			}

			if (convergenceCount == SGDConvergenceLimit)
			{
				converged = true;
				break;
			}
		}

		if (converged)
		{
			break;
		}
	}

	// normalize the weight vector
	normalize(weightVec);

	// dump
	dump(weightFileName, weightVec);

	clock_t te = clock();

	cout << "\nFinal weight vector: ";

	for (int i = 0; i < (int)weightVec.size(); i++)
	{
		cout << weightVec[i]
		     << " ";
	}

	cout << "\nTraining time: "
	     << (float)(te - tb) / CLOCKS_PER_SEC
		 << " second(s)"
		 << endl;
}

/************************************************
  add stack
************************************************/
void Train::addStack(const Alignment& a,
	                 int limit,
				     vector<Alignment>& stack)
{
	// emtpy
	if (stack.empty())
	{
		stack.push_back(a);
		return;
	}

	float minScore = 0.0;
	int minSubscript = 0;

	for (int i = 0; i < (int)stack.size(); i++)
	{
		if (stack[i].linkVec == a.linkVec)
		{
			return;
		}

		if (i == 0 ||
			stack[i].score < minScore)
		{
			minScore = stack[i].score;
			minSubscript = i;
		}
	}

	if ((int)stack.size() >= limit)
	{
		if (a.score > minScore)
		{
			stack[minSubscript] = a;
		}
	}
	else
	{
		stack.push_back(a);
	}
}

/************************************************
  compute n-best list
************************************************/
void Train::computeNBestList(const vector<int>& srcIDVec,
	                         const vector<int>& trgIDVec,
			    			 vector<Alignment>& nBestList)
{
	// initialize
	vector<Alignment> activeVec;
	Alignment a((int)srcIDVec.size() - 1, (int)trgIDVec.size() - 1);
	fg.computeScore(srcIDVec, trgIDVec, a, weightVec);
	addStack(a, beamSize, activeVec);
	addStack(a, sampleSize, nBestList);

	// extend
	while (!activeVec.empty())
	{
		vector<Alignment> hold;

		for (int subscript = 0; subscript < (int)activeVec.size(); subscript++)
		{
			Alignment a = activeVec[subscript];

			for (int i = 1; i < (int)srcIDVec.size(); i++)
			{
				for (int j = 1; j < (int)trgIDVec.size(); j++)
				{
					// ignore if the link exists
					pair<int, int> link(i, j);

					if (a.exists(link))
					{
						continue;
					}

					// generate a new alignment
					Alignment newA = a;
					fg.computeScore(srcIDVec, trgIDVec, newA, link, weightVec);

					// update the n-best list
					addStack(newA, sampleSize, nBestList);

					// update hold
					if (newA.score > a.score)
					{
						addStack(newA, beamSize, hold);
					}
				}
			}
		}

		activeVec = hold;
	}
}

/************************************************
  log sum
************************************************/
float Train::logSum(const vector<float>& v)
{
	float maxScore = 0.0;

	for (int i = 0; i < (int)v.size(); i++)
	{
		if (i == 0 ||
			v[i] > maxScore)
		{
			maxScore = v[i];
		}
	}

	float sum = 0.0;

	for (int i = 0; i < (int)v.size(); i++)
	{
		sum += exp(v[i] - maxScore);
	}

	return maxScore + log(sum);
}

/************************************************
  compute expectation
************************************************/
void Train::computeExpectation(const vector<int>& srcIDVec,
	                           const vector<int>& trgIDVec,
			    			   vector<float>& expectation)
{
	// compute n-best list
	vector<Alignment> nBestList;
	computeNBestList(srcIDVec, trgIDVec, nBestList);

	// compute the expectation
	expectation.resize(weightVec.size(), 0.0);

	vector<float> scoreVec;

	for (int i = 0; i < (int)nBestList.size(); i++)
	{
		scoreVec.push_back(nBestList[i].score);
	}

	float sum = logSum(scoreVec);

	for (int i = 0; i < (int)nBestList.size(); i++)
	{
		float prob = exp(nBestList[i].score - sum);

		for (int j = 0; j < (int)nBestList[i].fvv.size(); j++)
		{
			expectation[j] += prob * nBestList[i].fvv[j];
		}
	}
}

/************************************************
  normalize weight vector
************************************************/
void Train::normalize(vector<float>& v)
{
	float sum = 0.0;

	for (int i = 0; i < (int)v.size(); i++)
	{
		sum += fabs(v[i]);
	}

	for (int i = 0; i < (int)v.size(); i++)
	{
		v[i] /= sum;
	}
}

/************************************************
  dump weight vector
************************************************/
void Train::dump(const char* fileName,
	             const vector<float>& v)
{
	ofstream out(fileName);

	for (int i = 0; i < (int)v.size(); i++)
	{
		out << v[i]
		    << " ";
	}
}
