#ifndef ALIGNMENT_H
#define ALIGNMENT_H

#include <iostream>
#include <vector>

using namespace std;

class Alignment
{
	// overload <<
	friend ostream& operator<<(ostream& out, const Alignment& right);
public:
	/* member functions */
	// constructor
	Alignment(int srcSentLen,
	          int trgSentLen);
	// overload ==
	bool operator==(const Alignment& right) const;
	// overload =
	Alignment& operator=(const Alignment& right);
	// overload <
	bool operator<(const Alignment& right) const;
	// overload >
	bool operator>(const Alignment& right) const;
	// add a link
	void addLink(const pair<int, int>& link);
	// judge whether a link exists
	bool exists(const pair<int, int>& link) const;
	// itg
	bool itg() const;
	// block itg
	bool bitg() const;

	/* data members */
	vector<pair<int, int> > linkVec;       // link vector
	vector<vector<int> > srcAligned;       // aligned target positions for each source word
	vector<vector<int> > trgAligned;       // aligned source positions for each target word
	int srcMaxFert;                        // source maximal fertility
	int trgMaxFert;                        // target maximal fertility
	vector<float> fvv;                     // feature value vector
	float score;                           // score
};

#endif
