#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>

using namespace std;

/************************************************
  display version information
************************************************/
void version()
{
	cout << "-------------------------------------------\n"
    		" genIni v0.1\n"
	    	" 2014/08/26 - 2014/08/26\n"
	    	" (c) 2004-2009 Yang Liu\n"
	    	"-------------------------------------------\n";
}

/************************************************
  help
************************************************/
void help()
{
	cout << "Usage: genIni [-h] -o <old_file> -l <lambda_file> -n <new_file>\n\n"
    		"Required arguments:\n"
	    	"  -o <old_file> is old ini file\n"
	    	"  -l <lambda_file> is lambda ini file\n"
	    	"  -n <new_file> is new ini file\n"
	    	"Optional arguments:\n"
	    	"  -h prints this help message to STDOUT\n";

	exit(1);
}

/************************************************
  main function
************************************************/
int main(int argc, char** argv)
{
	version();

	string oldFileName,
	       lambdaFileName,
	       newFileName;
	int number_of_systems = 0;

	for (int i = 1; i < argc; i++)
	{
		if (string(argv[i]) == "-o")
		{
			oldFileName = argv[++i];
		}
		else if (string(argv[i]) == "-l")
		{
			lambdaFileName = argv[++i];
		}
		else if (string(argv[i]) == "-n")
		{
			newFileName = argv[++i];
		}
		else if (string(argv[i]) == "-h")
		{
			help();
		}
		else
		{
			help();
		}
	}

	if (oldFileName == "" ||
        lambdaFileName == "" ||
        newFileName == "")
	{
		help();
	}

	ifstream in1(lambdaFileName.c_str());
	
	vector<float> lambdaVec;
	string line;
	
	getline(in1, line);
	getline(in1, line);

	istringstream iss(line.c_str());
	string w;

	while (iss >> w)
	{
		lambdaVec.push_back(atof(w.c_str()));
	}

	int index = 0;

	ifstream in2(oldFileName.c_str());
	ofstream out(newFileName.c_str());

	while (getline(in2, line))
	{
		if (line.find("feature weight]") != string::npos)
		{
			int spp = line.find(']') + 1;
			out << line.substr(0, spp)
				<< ' '
				<< lambdaVec[index++]
				<< endl;
		}
		else
		{
			out << line << endl;
		}
	}

	return 0;
}
