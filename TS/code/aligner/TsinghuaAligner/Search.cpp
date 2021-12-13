#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include "Search.h"
#include "Link.h"

/************************************************
  constructor
************************************************/
Search::Search()
{
}

/************************************************
  initialize
************************************************/
void Search::init(const char* iniFileName,
	              const char* srcFileName,
				  const char* trgFileName)
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
		
		// structural constraint
		if (line.find("[structural constraint]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> structuralConstraint;
		}

		// enable pre-pruning
		if (line.find("[enable pre-pruning]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> enablePrePruning;
		}

		// pre-pruning threshold
		if (line.find("[pre-pruning threshold]") != string::npos)
		{
			int spp = line.find(']') + 2;
			istringstream iss(line.substr(spp, (int)line.size() - spp).c_str());
			iss >> prePruningThreshold;
		}
	}

	// build word sets
	set<string> srcWordSet,
	            trgWordSet;
	buildWordSet(srcFileName, srcWordSet);
	buildWordSet(trgFileName, trgWordSet);

	// initialize knowledge sources
	srcVcb.load(srcVcbFileName.c_str(), srcWordSet);
	trgVcb.load(trgVcbFileName.c_str(), trgWordSet);

	s2tTTable.init(&srcVcb, &trgVcb);
	s2tTTable.load(s2tTTableFileName.c_str());

	t2sTTable.init(&trgVcb, &srcVcb);
	t2sTTable.load(t2sTTableFileName.c_str());

	fg.init(&s2tTTable, &t2sTTable);
}

/*************************************************
  build word set
*************************************************/
void Search::buildWordSet(const char* fileName,
	                      set<string>& wordSet)
{
	// clear
	wordSet.clear();

	// build
	ifstream in(fileName);
	string line;

	while (getline(in, line))
	{
		istringstream iss(line.c_str());
		string w;

		while (iss >> w)
		{
			wordSet.insert(w);
		}
	}
}

/************************************************
  get id vector
************************************************/
void Search::getIDVec(const string& line,
	                  Vocab* vcbPtr,
	                  vector<int>& idVec)
{
	idVec.clear();
	idVec.push_back(0);  // the empty word

	istringstream iss(line.c_str());
	string w;

	while (iss >> w)
	{
		int id = vcbPtr->getID(w);
		idVec.push_back(id);
	}
}

/************************************************
  align a file
************************************************/
void Search::alignFile(const char* iniFileName,
	                   const char* srcFileName,
			           const char* trgFileName,
				       const char* agtFileName,
				       int nBestListSize,
				       int verboseLevel,
					   int posterior)
{
	// initialize
	init(iniFileName, srcFileName, trgFileName);

	ifstream in1(srcFileName),
	         in2(trgFileName);
	ofstream out1(agtFileName),
	         out2;

	if (nBestListSize > 1)
	{
		out2.open("nBestList.txt");
	}

	string line1,
	       line2;
	int sentCount = 0,
	    wordCount = 0;
	float totalModelScore = 0.0;

	clock_t tb = clock();

	while (getline(in1, line1) &&
		   getline(in2, line2))
	{
		if (line1.empty() ||
			line2.empty())
		{
            out1 << endl;
			continue;
		}

		sentCount++;

		if (sentCount % 10000 == 0)
		{
			cout << sentCount << endl;
		}

		// get id vectors
		vector<int> srcIDVec,
		            trgIDVec;
		getIDVec(line1, &srcVcb, srcIDVec);
		getIDVec(line2, &trgVcb, trgIDVec);

		wordCount += (int)srcIDVec.size() + (int)trgIDVec.size() - 2;

		// compute n-best list
		vector<Alignment> nBestList;
		map<pair<int, int>, float> posteriorMap;
		clock_t t1 = clock();
		computeNBestList(srcIDVec, trgIDVec, nBestListSize, posterior, nBestList, posteriorMap);
		clock_t t2 = clock();
		float tt = (float)(t2 - t1) / CLOCKS_PER_SEC;
		totalModelScore += nBestList[0].score;

		if (verboseLevel > 0)
		{
			vector<string> srcWordVec,
			               trgWordVec;

			istringstream iss1(line1.c_str());
			string w;

			while (iss1 >> w)
			{
				srcWordVec.push_back(w);
			}

			istringstream iss2(line2.c_str());

			while (iss2 >> w)
			{
				trgWordVec.push_back(w);
			}

			cout << "\n("
			     << sentCount
				 << ")\n"
				 << "[source] ";

			for (int i = 0; i < (int)srcWordVec.size(); i++)
			{
				cout << srcWordVec[i]
				     << "/"
					 << i + 1
					 << " ";
			}

			cout << "\n[target] ";

			for (int i = 0; i < (int)trgWordVec.size(); i++)
			{
				cout << trgWordVec[i]
				     << "/"
					 << i + 1
					 << " ";
			}

			cout << "\n[alignment] ";

			for (int i = 0; i < (int)nBestList[0].linkVec.size(); i++)
			{
				// standard format
				cout << nBestList[0].linkVec[i].first
				     << ":"
					 << nBestList[0].linkVec[i].second;

				if (i != (int)nBestList[0].linkVec.size() - 1)
				{
					cout << " ";
				}
			}

			cout << "\n[score] "
			     << nBestList[0].score
				 << "\n[time] "
				 << tt
				 << endl;
		}

		// dump 1-best alignment
		// posterior
		if (posterior)
		{
			map<pair<int, int>, float>::const_iterator iter;
				
			// sort
			vector<Link> linkVec;
				
			for (iter = posteriorMap.begin();
			     iter != posteriorMap.end();
				 iter++)
			{
				Link link(iter->first.first, iter->first.second, iter->second);
				linkVec.push_back(link);
			}

			sort(linkVec.begin(), linkVec.end(), std::greater<Link>());

			// dump
			for (int i = 0; i < (int)linkVec.size(); i++)
			{
				out1 << linkVec[i];

				if (i != (int)linkVec.size() - 1)
				{
					out1 << " ";
				}
			}

			out1 << endl;
		}
		// Moses format
		else
		{
			for (int i = 0; i < (int)nBestList[0].linkVec.size(); i++)
			{
				out1 << nBestList[0].linkVec[i].first - 1
			 	    << "-"
					 << nBestList[0].linkVec[i].second - 1;

				if (i != (int)nBestList[0].linkVec.size() - 1)
				{
					out1 << " ";
				}
			}

			out1 << endl;
		}

		// dump n-best list
		if (nBestListSize > 1)
		{
			out2 << "<sentPair id="
			     << sentCount
				 << "> "
				 << nBestList.size()
				 << endl;

			for (int i = 0; i < (int)nBestList.size(); i++)
			{
				out2 << "<candidate id="
				     << i + 1
					 << ">";

				for (int j = 0; j < (int)nBestList[i].linkVec.size(); j++)
				{
					// standard format
					out2 << nBestList[i].linkVec[j].first
					     << ":"
						 << nBestList[i].linkVec[j].second;

					if (j != (int)nBestList[i].linkVec.size() - 1)
					{
						out2 << " ";
					}
				}

				out2 << "</candidate>"
				     << "\n<costs>";

				for (int j = 0; j < (int)nBestList[i].fvv.size(); j++)
				{
					out2 << nBestList[i].fvv[j];

					if (j != (int)nBestList[i].fvv.size() - 1)
					{
						out2 << " ";
					}
				}

				out2 << "</costs>"
				     << endl;
			}

			out2 << "</sentPair>\n\n";
		}
	}

	clock_t te = clock();

	float totalTime = (float)(te - tb) / CLOCKS_PER_SEC;

	cout << "\n[SENT] "
	     << sentCount
		 << "\n[WORD] "
		 << wordCount
		 << "\n[SCORE] "
		 << totalModelScore
		 << "\n[TIME] "
		 << totalTime
		 << " second(s)"
		 << "\n[AVG TIME] "
		 << totalTime / sentCount
		 << " second(s)"
		 << "\n[SPEED] "
		 << wordCount / totalTime
		 << " word/second"
		 << endl;
}


/************************************************
  add stack
************************************************/
void Search::addStack(const Alignment& a,
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
void Search::computeNBestList(const vector<int>& srcIDVec,
	                          const vector<int>& trgIDVec,
							  int nBestListSize,
							  int posterior,
			      			  vector<Alignment>& nBestList,
							  map<pair<int, int>, float>& posteriorMap)
{
	set<Alignment> exploredSpace;

	// standard
	if (structuralConstraint == 0)
	{
		computeNBestListSTD(srcIDVec, trgIDVec, nBestListSize, posterior, nBestList, exploredSpace);
	}
	// ITG
	else if (structuralConstraint == 1)
	{
		computeNBestListITG(srcIDVec, trgIDVec, nBestListSize, posterior, nBestList, exploredSpace);
	}
	// block ITG
	else
	{
		computeNBestListBITG(srcIDVec, trgIDVec, nBestListSize, posterior, nBestList, exploredSpace);
	}

	// compute posteriors
	if (posterior)
	{
		computePosterior(exploredSpace, posteriorMap);
	}
}

void Search::computeNBestListSTD(const vector<int>& srcIDVec,
	                             const vector<int>& trgIDVec,
							     int nBestListSize,
								 int posterior,
			      			     vector<Alignment>& nBestList,
								 set<Alignment>& exploredSpace)
{
	// initialize
	vector<Alignment> activeVec;
	Alignment a((int)srcIDVec.size() - 1, (int)trgIDVec.size() - 1);
	fg.computeScore(srcIDVec, trgIDVec, a, weightVec);
	addStack(a, beamSize, activeVec);
	addStack(a, nBestListSize, nBestList);
	
	if (posterior)
	{
		exploredSpace.insert(a);
	}

	// pre-pruning
	if (enablePrePruning)
	{
		// compute promising link vector
		vector<pair<int, int> > candVec;

		for (int i = 1; i < (int)srcIDVec.size(); i++)
		{
			for (int j = 1; j < (int)trgIDVec.size(); j++)
			{
				float score = s2tTTable.getProb(srcIDVec[i], trgIDVec[j]) -
				              s2tTTable.getProb(srcIDVec[0], trgIDVec[j]) +
							  t2sTTable.getProb(trgIDVec[j], srcIDVec[i]) -
							  t2sTTable.getProb(trgIDVec[0], srcIDVec[i]);

				if (score > prePruningThreshold)
				{
					candVec.push_back(pair<int, int>(i, j));
				}
			}
		}

		while (!activeVec.empty())
		{
			vector<Alignment> hold;

			for (int subscript = 0; subscript < (int)activeVec.size(); subscript++)
			{
				Alignment a = activeVec[subscript];

				for (int i = 0; i < (int)candVec.size(); i++)
				{
					// ignore if the link exists
					pair<int, int> link = candVec[i];

					if (a.exists(link))
					{
						continue;
					}

					// generate a new alignment
					Alignment newA = a;
					fg.computeScore(srcIDVec, trgIDVec, newA, link, weightVec);

					// update the n-best list
					addStack(newA, nBestListSize, nBestList);

					// update hold
					if (newA.score > a.score)
					{
						addStack(newA, beamSize, hold);
					}

					// update explored space
					if (posterior)
					{
						exploredSpace.insert(newA);
					}
				}
			}

			activeVec = hold;
		}
	}
	// standard algorithm
	else
	{
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
						addStack(newA, nBestListSize, nBestList);

						// update hold
						if (newA.score > a.score)
						{
							addStack(newA, beamSize, hold);
						}
						
						// update explored space
						if (posterior)
						{
							exploredSpace.insert(newA);
						}
					}
				}
			}

			activeVec = hold;
		}
	}

	// sort
	sort(nBestList.begin(), nBestList.end(), std::greater<Alignment>());
}

void Search::computeNBestListITG(const vector<int>& srcIDVec,
	                             const vector<int>& trgIDVec,
							     int nBestListSize,
								 int posterior,
			      			     vector<Alignment>& nBestList,
								 set<Alignment>& exploredSpace)
{
	// initialize
	vector<Alignment> activeVec;
	Alignment a((int)srcIDVec.size() - 1, (int)trgIDVec.size() - 1);
	fg.computeScore(srcIDVec, trgIDVec, a, weightVec);
	addStack(a, beamSize, activeVec);
	addStack(a, nBestListSize, nBestList);

	if (posterior)
	{
		exploredSpace.insert(a);
	}

	// pre-pruning
	if (enablePrePruning)
	{
		// compute promising link vector
		vector<pair<int, int> > candVec;

		for (int i = 1; i < (int)srcIDVec.size(); i++)
		{
			for (int j = 1; j < (int)trgIDVec.size(); j++)
			{
				float score = s2tTTable.getProb(srcIDVec[i], trgIDVec[j]) -
				              s2tTTable.getProb(srcIDVec[0], trgIDVec[j]) +
							  t2sTTable.getProb(trgIDVec[j], srcIDVec[i]) -
							  t2sTTable.getProb(trgIDVec[0], srcIDVec[i]);

				if (score > prePruningThreshold)
				{
					candVec.push_back(pair<int, int>(i, j));
				}
			}
		}

		while (!activeVec.empty())
		{
			vector<Alignment> hold;

			for (int subscript = 0; subscript < (int)activeVec.size(); subscript++)
			{
				Alignment a = activeVec[subscript];

				for (int i = 0; i < (int)candVec.size(); i++)
				{
					// ignore if the link exists
					pair<int, int> link = candVec[i];

					if (a.exists(link))
					{
						continue;
					}

					// generate a new alignment
					Alignment newA = a;
					fg.computeScore(srcIDVec, trgIDVec, newA, link, weightVec);

					// check structural constraint
					if (!newA.itg())
					{
						continue;
					}

					// update the n-best list
					addStack(newA, nBestListSize, nBestList);

					// update hold
					if (newA.score > a.score)
					{
						addStack(newA, beamSize, hold);
					}

					// update explored space
					if (posterior)
					{
						exploredSpace.insert(newA);
					}
				}
			}

			activeVec = hold;
		}
	}
	// standard algorithm
	else
	{
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

						// check structural constraint
						if (!newA.itg())
						{
							continue;
						}

						// update the n-best list
						addStack(newA, nBestListSize, nBestList);

						// update hold
						if (newA.score > a.score)
						{
							addStack(newA, beamSize, hold);
						}
						
						// update explored space
						if (posterior)
						{
							exploredSpace.insert(newA);
						}
					}
				}
			}

			activeVec = hold;
		}
	}

	// sort
	sort(nBestList.begin(), nBestList.end(), std::greater<Alignment>());
}

void Search::computeNBestListBITG(const vector<int>& srcIDVec,
	                              const vector<int>& trgIDVec,
							      int nBestListSize,
								  int posterior,
			      			      vector<Alignment>& nBestList,
								  set<Alignment>& exploredSpace)
{
	// initialize
	vector<Alignment> activeVec;
	Alignment a((int)srcIDVec.size() - 1, (int)trgIDVec.size() - 1);
	fg.computeScore(srcIDVec, trgIDVec, a, weightVec);
	addStack(a, beamSize, activeVec);
	addStack(a, nBestListSize, nBestList);

	if (posterior)
	{
		exploredSpace.insert(a);
	}

	// pre-pruning
	if (enablePrePruning)
	{
		// compute promising link vector
		vector<pair<int, int> > candVec;

		for (int i = 1; i < (int)srcIDVec.size(); i++)
		{
			for (int j = 1; j < (int)trgIDVec.size(); j++)
			{
				float score = s2tTTable.getProb(srcIDVec[i], trgIDVec[j]) -
				              s2tTTable.getProb(srcIDVec[0], trgIDVec[j]) +
							  t2sTTable.getProb(trgIDVec[j], srcIDVec[i]) -
							  t2sTTable.getProb(trgIDVec[0], srcIDVec[i]);

				if (score > prePruningThreshold)
				{
					candVec.push_back(pair<int, int>(i, j));
				}
			}
		}

		while (!activeVec.empty())
		{
			vector<Alignment> hold;

			for (int subscript = 0; subscript < (int)activeVec.size(); subscript++)
			{
				Alignment a = activeVec[subscript];

				for (int i = 0; i < (int)candVec.size(); i++)
				{
					// ignore if the link exists
					pair<int, int> link = candVec[i];

					if (a.exists(link))
					{
						continue;
					}

					// generate a new alignment
					Alignment newA = a;
					fg.computeScore(srcIDVec, trgIDVec, newA, link, weightVec);

					// check structural constraint
					if (!newA.bitg())
					{
						continue;
					}

					// update the n-best list
					addStack(newA, nBestListSize, nBestList);

					// update hold
					if (newA.score > a.score)
					{
						addStack(newA, beamSize, hold);
					}
		
					// update explored space
					if (posterior)
					{
						exploredSpace.insert(newA);
					}
				}
			}

			activeVec = hold;
		}
	}
	// standard algorithm
	else
	{
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

						// check structural constraint
						if (!newA.bitg())
						{
							continue;
						}

						// update the n-best list
						addStack(newA, nBestListSize, nBestList);

						// update hold
						if (newA.score > a.score)
						{
							addStack(newA, beamSize, hold);
						}
						
						// update explored space
						if (posterior)
						{
							exploredSpace.insert(newA);
						}
					}
				}
			}

			activeVec = hold;
		}
	}

	// sort
	sort(nBestList.begin(), nBestList.end(), std::greater<Alignment>());
}

/************************************************
  compute posterior probabilities
************************************************/
void Search::computePosterior(const set<Alignment>& exploredSpace,
	                          map<pair<int, int>, float>& posterior)
{
	// compute log sum
	vector<float> scoreVec;

	set<Alignment>::const_iterator iter;

	for (iter = exploredSpace.begin();
	     iter != exploredSpace.end();
		 iter++)
	{
		scoreVec.push_back(iter->score);
	}

	float sum = logSum(scoreVec);

	// compute link posterior porbabilities
	for (iter = exploredSpace.begin();
	     iter != exploredSpace.end();
		 iter++)
	{
		Alignment a = *iter;

		for (int i = 0; i < (int)a.linkVec.size(); i++)
		{
			pair<int, int> link = a.linkVec[i];
			float p = exp(a.score - sum);

			map<pair<int, int>, float>::iterator it = posterior.find(link);

			if (it == posterior.end())
			{
				posterior.insert(map<pair<int, int>, float>::value_type(link, p));
			}
			else
			{
				it->second += p;
			}
		}
	}
}

/************************************************
  compute log sum
************************************************/
float Search::logSum(const vector<float>& v) const
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
