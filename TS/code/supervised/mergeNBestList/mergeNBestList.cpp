#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cstdlib>
#include <vector>

using namespace std;

/************************************************
  version
************************************************/
void version()
{
	cout << "----------------------------------------\n"
	 	    " mergeNBestList v0.1\n"
		    " 2014/08/26 - 2014/08/26\n"
		    " (c) 2004-2009 Yang Liu\n"
		    "----------------------------------------\n";
}

/************************************************
  main function
************************************************/
int main(int argc, char** argv)
{
	// display version information
	version();

	if (argc != 4)
	{
		cerr << "Usage: mergeNBestList oldFile newFile mergedFile\n";
		exit(1);
	}

	ifstream in1(argv[1]),
		 in2(argv[2]);
	ofstream out(argv[3]);
	string line;

	map<pair<string, string>, int> cMap;
	string str1,
	       str2;
	int sentPairNO = 0;

	while (1)
	{
		bool flag1 = false,
			 flag2 = false;

		while (getline(in1, line))
		{
			if (line.find("<sentPair") != -1)
			{
				str1 = "";
				str2 = "";
			}
			else if (line.find("<candidate") != -1)
			{
				int spp1 = line.find('>') + 1,
				    spp2 = line.find('<', spp1);
				str1 = line.substr(spp1, spp2 - spp1);
			}
			else if (line.find("<costs") != -1)
			{
				int spp1 = line.find('>') + 1,
				    spp2 = line.find('<', spp1);
				str2 = line.substr(spp1, spp2 - spp1);

				cMap.insert(map<pair<string, string>, double>::value_type(make_pair(str1, str2), 1));
			}
			else if (line.find("</sentPair>") != -1)
			{
				flag1 = true;

				break;
			}
		}

		while (getline(in2, line))
		{
			if (line.find("<sentPair") != -1)
			{
				str1 = "";
				str2 = "";
			}
			else if (line.find("<candidate") != -1)
			{
				int spp1 = line.find('>') + 1,
				    spp2 = line.find('<', spp1);
				str1 = line.substr(spp1, spp2 - spp1);
			}
			else if (line.find("<costs") != -1)
			{
				int spp1 = line.find('>') + 1,
				    spp2 = line.find('<', spp1);
				str2 = line.substr(spp1, spp2 - spp1); 

				cMap.insert(map<pair<string, string>, double>::value_type(make_pair(str1, str2), 1));
			}
			else if (line.find("</sentPair>") != -1)
			{
				flag2 = true;

				break;
			}
		}

		if (!flag1 || !flag2)
		{
			break;
		}

		sentPairNO++;

		out << "<sentPair id="
		    << sentPairNO
		    << "> "
		    << cMap.size()
		    << "\n";

		map<pair<string, string>, int>::iterator iter;
		int count = 1;

		for (iter = cMap.begin(); iter != cMap.end(); iter++)
		{
			out << "<candidate id="
			    << count++
			    << ">"
			    << iter->first.first
			    << "</candidate>\n"
			    << "<costs>"
			    << iter->first.second
			    << "</costs>\n";
		}

		out << "</sentPair>\n\n";
			
		cMap.clear();
	}
	
	return 0;
}
