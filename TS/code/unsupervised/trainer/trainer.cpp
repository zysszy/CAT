#include <iostream>
#include <cstdlib>
#include "Train.h"

/************************************************
  version
************************************************/
void version()
{
	cout << "----------------------------------------\n"
	        " trainer v0.1\n"
			" 2014/09/01 - 2014/09/03\n"
			" (c) 2014 Yang Liu\n"
			"----------------------------------------\n";
}

/************************************************
  main function
************************************************/
int main(int argc, char** argv)
{
	// version
	version();

	if (argc != 4)
	{
		cerr << "Usage: trainer iniFile trnFile weightFile"
		     << endl;

		exit(1);
	}

	Train t;
	t.train(argv[1], argv[2], argv[3]);

	return 0;
}
