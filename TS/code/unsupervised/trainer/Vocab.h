#ifndef VOCAB_H
#define VOCAB_H

#include <map>
#include <set>
#include <string>

using namespace std;

class Vocab
{
public:
	/* member functions */
	// constructor
	Vocab();
	// load
	void load(const char* fileName,
	          const set<string>& wordSet);
	// dump
	void dump(const char* fileName);
	// get a word
	string getWord(int id) const;
	// get an id
	int getID(const string& word) const;
	// get size
	int getSize() const;

private:
	/* data members */
	map<int, string> id2word;  // id -> word
	map<string, int> word2id;  // word -> id
};

#endif
