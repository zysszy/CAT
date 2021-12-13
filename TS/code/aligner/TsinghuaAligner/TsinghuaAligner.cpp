#include <iostream>
#include <cstdlib>
#include "Search.h"

/************************************************
  version
************************************************/
void version()
{
	cout << "----------------------------------------\n"
	        " TsinghuaAligner v0.6\n"
			" 2014/12/13 - 2014/12/13\n"
			" (c) 2014 Yang Liu\n"
			"----------------------------------------\n";
}

/************************************************
  help
************************************************/
void help()
{
	cout << "Usage: TsinghuaAligner [--help] ...\n"
	        "Required arguments:\n"
			"  --ini-file <ini_file>      initialization file\n"
			"  --src-file <src_file>      source file\n"
			"  --trg-file <trg_file>      target file\n"
			"  --agt-file <agt_file>      alignment file\n"
			"Optional arguments:\n"
			"  --nbest-list [1, +00)      n-best list size (default: 1)\n"
			"  --verbose {0, 1}           displays run-time information\n"
			"                               * 0: document level (default)\n"
			"                               * 1: sentence level\n"
			"  --posterior {0, 1}         outputs posterior probabilities (default: 0)\n"
			"  --help                     prints this message to STDOUT\n";

	exit(1);
}

/************************************************
  main function
************************************************/
int main(int argc, char** argv)
{
	// version
	version();
	
	// initialize arguments
	string iniFileName,
	       srcFileName,
		   trgFileName,
		   agtFileName;
	int nBestListSize = 1,
	    verboseLevel = 0,
		posterior = 0;

	// analyze command-line arguments
	int i = 1;

	while (i < argc)
	{
		if (string(argv[i]) == "--ini-file")
		{
			iniFileName = argv[i + 1];
		}
		else if (string(argv[i]) == "--src-file")
		{
			srcFileName = argv[i + 1];
		}
		else if (string(argv[i]) == "--trg-file")
		{
			trgFileName = argv[i + 1];
		}
		else if (string(argv[i]) == "--agt-file")
		{
			agtFileName = argv[i + 1];
		}
		else if (string(argv[i]) == "--nbest-list")
		{
			nBestListSize = atoi(argv[i + 1]);
		}
		else if (string(argv[i]) == "--verbose")
		{
			verboseLevel = atoi(argv[i + 1]);
		}
		else if (string(argv[i]) == "--posterior")
		{
			posterior = atoi(argv[i + 1]);
		}
		else
		{
			help();
		}

		i += 2;
	}

	// check required arguments
	if (iniFileName.empty() ||
		srcFileName.empty() ||
		trgFileName.empty() ||
		agtFileName.empty())
	{
		help();
	}

	// align
	Search s;
	s.alignFile(iniFileName.c_str(),
	            srcFileName.c_str(),
				trgFileName.c_str(),
				agtFileName.c_str(),
				nBestListSize,
				verboseLevel,
				posterior);

	return 0;
}
