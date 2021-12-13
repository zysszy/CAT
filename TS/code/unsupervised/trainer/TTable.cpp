#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "TTable.h"

/************************************************
  constructor
************************************************/
TTable::TTable()
{
	probSmooth = log(1e-7);
}

/************************************************
  initalize
************************************************/
void TTable::init(Vocab* srcVcbPtr,
	              Vocab* trgVcbPtr)
{
	this->srcVcbPtr = srcVcbPtr;
	this->trgVcbPtr = trgVcbPtr;
}

/************************************************
  load
************************************************/
void TTable::load(const char* fileName)
{
	cout << "Load translation probability table file \""
	     << fileName
		 << "\" ... "
		 << flush;

	clock_t tb = clock();

	ifstream in(fileName);

	if (!in)
	{
		cerr << "\nERROR at [TTable::load]: "
		     << "cannot open file \""
			 << fileName
			 << "\"!"
			 << endl;

		exit(1);
	}

	string line;
	int count = 0;

	while (getline(in, line))
	{
		if (line.empty())
		{
			continue;
		}

		count++;

		// get an entry
		istringstream iss(line.c_str());
		string w1, w2, w3;
		iss >> w1 >> w2 >> w3;

		int srcID = atoi(w1.c_str()),
		    trgID = atoi(w2.c_str());
		float prob = log(atof(w3.c_str()));

		if (prob < probSmooth)
		{
			prob = probSmooth;
		}

		if (srcVcbPtr->getWord(srcID) == "$UNK" ||
			trgVcbPtr->getWord(trgID) == "$UNK")
		{
			continue;
		}

		tTable.insert(map<pair<int, int>, float>::value_type(pair<int, int>(srcID, trgID), prob));
	}

	clock_t te = clock();

	cout << (int)tTable.size()
	     << " of "
		 << count
		 << " entries loaded in "
		 << (float)(te - tb) / CLOCKS_PER_SEC
		 << " second(s)"
		 << endl;
}

/************************************************
  dump
************************************************/
void TTable::dump(const char* fileName)
{
	ofstream out(fileName);

	map<pair<int, int>, float>::const_iterator iter;

	for (iter = tTable.begin(); iter != tTable.end(); iter++)
	{
		string srcWord = srcVcbPtr->getWord(iter->first.first),
		       trgWord = trgVcbPtr->getWord(iter->first.second);
		float prob = iter->second;

		out << srcWord
		    << " "
			<< trgWord
			<< " "
			<< prob
			<< endl;
	}
}

/************************************************
  get probability
************************************************/
float TTable::getProb(int srcID,
	                  int trgID) const
{
	pair<int, int> pr(srcID, trgID);

	map<pair<int, int>, float>::const_iterator iter = tTable.find(pr);

	if (iter == tTable.end())
	{
		return probSmooth;
	}
	else
	{
		return iter->second;
	}
}

float TTable::getProb(const string& srcWord,
	                  const string& trgWord) const
{
	int srcID = srcVcbPtr->getID(srcWord),
	    trgID = trgVcbPtr->getID(trgWord);

	return getProb(srcID, trgID);
}

