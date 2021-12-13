#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdlib>

using namespace std;

/************************************************
  version
************************************************/
void version()
{
	cout << "-------------------------------------------\n"
	        " waEval v0.2\n"
			" 2014/10/07 - 2014/10/07\n"
			" (c) 2014 Yang Liu\n"
			"-------------------------------------------\n";
}

/************************************************
  get an alignment from a string
************************************************/
void getAlignment(const string& str,
                  map<pair<int, int>, int>& alignment)
{
	istringstream iss(str.c_str());
	string w;

	while (iss >> w)
	{
		int spp1 = w.find(':'),
		    spp2 = w.find('/');

		if (spp2 == string::npos)
		{
			int src_pos = atoi(w.substr(0, spp1).c_str()),
			    trg_pos = atoi(w.substr(spp1 + 1, w.size() - spp1 - 1).c_str()),
				type = 1;  // default type of a link is "sure"
			alignment.insert(map<pair<int, int>, int>::value_type(pair<int, int>(src_pos, trg_pos), type));
		}
		else
		{
			int src_pos = atoi(w.substr(0, spp1).c_str()),
			    trg_pos = atoi(w.substr(spp1 + 1, spp2 - spp1 - 1).c_str()),
				type = atoi(w.substr(spp2 + 1, w.size() - spp2 - 1).c_str());
			alignment.insert(map<pair<int, int>, int>::value_type(pair<int, int>(src_pos, trg_pos), type));
		}
	}
}

/************************************************
  get an alignment from a string (Moses)
************************************************/
void getAlignment2(const string& str,
                   map<pair<int, int>, int>& alignment)
{
	istringstream iss(str.c_str());
	string w;

	while (iss >> w)
	{
		int spp = w.find('-'),
		    sp = atoi(w.substr(0, spp).c_str()) + 1,
			tp = atoi(w.substr(spp + 1, (int)w.size() - spp - 1).c_str()) + 1,
			type = 1;
		alignment.insert(map<pair<int, int>, int>::value_type(pair<int, int>(sp, tp), type));
	}
}


/************************************************
  main function
************************************************/
int main(int argc, char** argv)
{
	// version
	version();

	if (argc < 3)
	{
		cerr << "Usage: waEval refFile testFile\n";
		exit(1);
	}

	ifstream in1(argv[1]),
	         in2(argv[2]);
	string line1,
	       line2;
	float total_match_sure = 0,
	      total_match_possible = 0,
	      total_actual = 0,
		  total_sure = 0;
	int sentPairID = 0;
	vector<pair<float, int> > aerVec;  // store AER for each sentence pair

	while (getline(in1, line1) &&
		   getline(in2, line2))
	{
		sentPairID++;

		// get an alignment
		map<pair<int, int>, int> ref, 
		                         tst;

		getAlignment(line1, ref);
		getAlignment2(line2, tst);

		// compute AER for a single sentence pair
		float match_sure = 0,
		      match_possible = 0,
			  actual = tst.size(),
			  sure = 0;

		map<pair<int, int>, int>::iterator iter1;

		for (iter1 = ref.begin(); iter1 != ref.end(); iter1++)
		{
			if (iter1->second == 1)
			{
				sure++;
			}
		}

		for (iter1 = tst.begin(); iter1 != tst.end(); iter1++)
		{
			map<pair<int, int>, int>::iterator iter2 = ref.find(iter1->first);

			if (iter2 != ref.end())
			{
				match_possible++;

				if (iter2->second == 1)
				{
					match_sure++;
				}
			}
		}

		float precision = match_possible / actual,
		      recall = match_sure / sure,
		      aer = 1.0 - (match_sure + match_possible) / (actual + sure);
		aerVec.push_back(pair<float, int>(aer, sentPairID));

		cout << "("
		     << sentPairID
		     << ") "
			 << match_sure
			 << " "
			 << match_possible
			 << " "
			 << actual
			 << " "
			 << sure
			 << " -> "
			 << aer
			 << "\n";

		total_match_sure += match_sure;
		total_match_possible += match_possible;
		total_actual += actual;
		total_sure += sure;
	}

	float precision = total_match_possible / total_actual,
	      recall = total_match_sure / total_sure,
		  aer = 1.0 - (total_match_sure + total_match_possible) /
		        (total_actual + total_sure);

	cout << "\n[total matched sure] "
	     << total_match_sure
		 << "\n[total matched possible] "
		 << total_match_possible
		 << "\n[total actual] "
		 << total_actual
		 << "\n[total sure] "
		 << total_sure
		 << "\n\n"
		 << "[Precision] "
		 << precision
		 << "\n[Recall] "
		 << recall
		 << "\n[AER] "
		 << aer
		 << "\n\n";

	sort(aerVec.begin(), aerVec.end(), greater<pair<float, int> >());

	cout << "Top 10 wrong predictions:\n";

	for (int i = 0; i < 10 && i < (int)aerVec.size(); i++)
	{
		cout << "("
		     << aerVec[i].second
			 << ") "
			 << aerVec[i].first
			 << "\n";		 
	}

	return 0;
}
