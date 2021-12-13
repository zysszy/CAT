#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

/************************************************
  display version information 
************************************************/
void version()
{
	cout << "-------------------------------------------\n"
    		" convertNBestListFormat v0.1\n"
	    	" 2014/08/26 - 2014/08/26\n"
	    	" (c) 2013 Yang Liu\n"
	    	"-------------------------------------------\n";
}

/************************************************
  main
************************************************/
int main(int argc, char** argv)
{
	// display version information
	version();

	if (argc != 5)
	{
		cerr << "Usage: convertNBestListFormat refFile nBestListFile candsFile featsFile\n";
		exit(1);
	}

	ifstream in1(argv[1]),
		     in2(argv[2]);
	ofstream out1(argv[3]),
		     out2(argv[4]);

	vector<map<pair<int, int>, int> > refVec;
	string line;

	while (getline(in1, line))
	{
		map<pair<int, int>, int> ref;
		istringstream iss(line.c_str());
		string w;

		while (iss >> w)
		{
			int spp1 = w.find(':'),
			    spp2 = w.find('/');
			string s1 = w.substr(0, spp1),
			       s2 = w.substr(spp1 + 1, spp2 - spp1 - 1),
			       s3 = w.substr(spp2 + 1, w.size() - spp2 - 1);
			int sp = atoi(s1.c_str()),
			    tp = atoi(s2.c_str()),
			    type = atoi(s3.c_str());
			ref.insert(map<pair<int, int>, int>::value_type(make_pair(sp, tp), type));
		}

		refVec.push_back(ref);
	}

	int count = 0,
	    id = 0;
	map<pair<int, int>, int> a;

	while (getline(in2, line))
	{
		if (line.find("<sentPair") != string::npos)
		{
			count = 0;
		}
		else if (line.find("<candidate") != string::npos)
		{
			int spp1 = line.find('>') + 1,
			    spp2 = line.find('<', spp1);
			string str = line.substr(spp1, spp2 - spp1);
			istringstream iss(str.c_str());
			string w;
			a.clear();

			while (iss >> w)
			{
				spp1 = w.find(':');
				string s1 = w.substr(0, spp1),
				       s2 = w.substr(spp1 + 1, w.size() - spp1 - 1);
				int sp = atoi(s1.c_str()),
				    tp = atoi(s2.c_str()),
			        type = 1;
				a.insert(map<pair<int, int>, int>::value_type(make_pair(sp, tp), type));
			}
		}
		else if (line.find("<costs>") != -1)
		{
			count++;

			int spp1 = line.find('>') + 1,
			    spp2 = line.find('<', spp1);
			string str = line.substr(spp1, spp2 - spp1);
			istringstream iss(str.c_str());
			string w;

			while (iss >> w)
			{
				double value = atof(w.c_str());
				out2 << -1.0 * value << " ";
			}

			int match_sure = 0,
			    match_possible = 0,
			    sure = 0,
			    actual = a.size();

			map<pair<int, int>, int>::iterator iter1, iter2;

			for (iter1 = a.begin(); iter1 != a.end(); iter1++)
			{
				for (iter2 = refVec[id].begin(); iter2 != refVec[id].end(); iter2++)
				{
					if ((*iter1).first == (*iter2).first)
					{
						match_possible++;

						if ((*iter2).second == 1)
						{
							match_sure++;
						}
					}
				}
			}

			for (iter2 = refVec[id].begin(); iter2 != refVec[id].end(); iter2++)
			{
				if ((*iter2).second == 1)
				{
					sure++;
				}
			}

			out2 << match_sure << " " << match_possible << " " << actual << " " << sure << endl;
		}
		else if (line.find("</sentPair>") != string::npos)
		{
			out1 << id++ << " " << count << endl;
		}
	}

	return 0;
}
