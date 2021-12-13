#ifndef LINK_H
#define LINK_H

#include <iostream>

using namespace std;

class Link
{
	// overload <<
	friend ostream& operator<<(ostream& out, const Link& right);
public:
	/* member functions */
	// constructors
	Link();
	Link(int sp,
	     int tp,
		 float prob);
	// overload =
	Link& operator=(const Link& right);
	// overload ==
	bool operator==(const Link& right) const;
	// overload <
	bool operator<(const Link& right) const;
	// overload >
	bool operator>(const Link& right) const;

	/* data members */
	int sp;      // source position
	int tp;      // target position
	float prob;  // posterior probabilities
};

#endif
