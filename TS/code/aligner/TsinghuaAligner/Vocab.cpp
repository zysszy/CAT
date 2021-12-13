#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <ctime>
#include "Vocab.h"

/************************************************
  constructor
************************************************/
Vocab::Vocab()
{
}

/************************************************
  load
************************************************/
void Vocab::load(const char* fileName,
	             const set<string>& wordSet)
{
	cout << "Load vocabulary file \""
	     << fileName
		 << "\" ... "
		 << flush;

	clock_t tb = clock();

	ifstream in(fileName);

	if (!in)
	{
		cerr << "\nERROR at [Vocab:load]: "
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

		// get word and id
		istringstream iss(line.c_str());
		int id;
		string word;
		iss >> id >> word;

		// ignore words not in the set
		set<string>::const_iterator iter = wordSet.find(word);

		if (iter == wordSet.end())
		{
			continue;
		}

		// add
		word2id.insert(map<string, int>::value_type(word, id));
		id2word.insert(map<int, string>::value_type(id, word));
	}

	clock_t te = clock();

	cout << (int)word2id.size()
	     << " of "
		 << count
		 << " entries loaded with "
		 << (int)wordSet.size() - (int)word2id.size()
		 << " OOV(s) in "
		 << (float)(te - tb) / CLOCKS_PER_SEC
		 << " second(s)"
		 << endl;
}

/************************************************
  dump
************************************************/
void Vocab::dump(const char* fileName)
{
	ofstream out(fileName);

	map<int, string>::const_iterator iter;

	for (iter = id2word.begin();
	     iter != id2word.end();
		 iter++)
	{
		out << iter->first
		    << " "
			<< iter->second
			<< endl;
	}
}

/************************************************
  get a word
************************************************/
string Vocab::getWord(int id) const
{
	if (id == 0)
	{
		return "$NULL";  // empty word
	}
	else if (id == 1)
	{
		return "$UNK";   // unknown words
	}
	else
	{
		map<int, string>::const_iterator iter = id2word.find(id);

		if (iter == id2word.end())
		{
			return "$UNK";
		}
		else
		{
			return iter->second;
		}
	}
}

/************************************************
  get an id
************************************************/
int Vocab::getID(const string& word) const
{
	map<string, int>::const_iterator iter = word2id.find(word);

	if (iter == word2id.end())
	{
		if (word == "$NULL")
		{
			return 0;
		}
		else
		{
			return 1;  // reserved for unknown words
		}
	}
	else
	{
		return iter->second;
	}
}

/************************************************
  get size
************************************************/
int Vocab::getSize() const
{
	return (int)word2id.size();
}

