#include <algorithm>
#include "Alignment.h"

/************************************************
  constructor
************************************************/
Alignment::Alignment(int srcSentLen,
	                 int trgSentLen)
{
	srcAligned.resize(srcSentLen + 1);
	trgAligned.resize(trgSentLen + 1);
	srcMaxFert = 0;
	trgMaxFert = 0;
}

/************************************************
  overload <<
************************************************/
ostream& operator<<(ostream& out, const Alignment& right)
{
	// link vector
	for (int i = 0; i < (int)right.linkVec.size(); i++)
	{
		out << right.linkVec[i].first
		    << ":"
			<< right.linkVec[i].second
			<< " ";
	}

	out << "||| ";

	// source aligned
	for (int i = 1; i < (int)right.srcAligned.size(); i++)
	{
		out << i
		    << "-[";

		for (int j = 0; j < (int)right.srcAligned[i].size(); j++)
		{
			out << right.srcAligned[i][j];

			if (j != (int)right.srcAligned[i].size() - 1)
			{
				out << ", ";
			}
		}

		out << "] ";
	}

	out << "||| ";

	// target aligned
	for (int i = 1; i < (int)right.trgAligned.size(); i++)
	{
		out << i
		    << "-[";

		for (int j = 0; j < (int)right.trgAligned[i].size(); j++)
		{
			out << right.trgAligned[i][j];

			if (j != (int)right.trgAligned[i].size() - 1)
			{
				out << ", ";
			}
		}

		out << "] ";
	}

	out << "||| ";

	// source maximal fertility
	out << right.srcMaxFert
	    << " ||| ";

	// target maximal fertility
	out << right.trgMaxFert
	    << " ||| ";

	// feature value vector
	for (int i = 0; i < (int)right.fvv.size(); i++)
	{
		out << right.fvv[i]
		    << " ";
	}

	// score
	out << "||| "
	    << right.score;

	return out;
}

/************************************************
  overload ==
************************************************/
bool Alignment::operator==(const Alignment& right) const
{
	if (linkVec == right.linkVec &&
		srcAligned == right.srcAligned &&
		trgAligned == right.trgAligned &&
		srcMaxFert == right.srcMaxFert &&
		trgMaxFert == right.trgMaxFert &&
		fvv == right.fvv &&
		score == right.score)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/************************************************
  overload =
************************************************/
Alignment& Alignment::operator=(const Alignment& right)
{
	if (this != &right)
	{
		linkVec = right.linkVec;
		srcAligned = right.srcAligned;
		trgAligned = right.trgAligned;
		srcMaxFert = right.srcMaxFert;
		trgMaxFert = right.trgMaxFert;
		fvv = right.fvv;
		score = right.score;
	}

	return *this;
}

/************************************************
  overload <
************************************************/
bool Alignment::operator<(const Alignment& right) const
{
	if (score < right.score)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/************************************************
  overload >
************************************************/
bool Alignment::operator>(const Alignment& right) const
{
	if (*this == right ||
		*this < right)
	{
		return false;
	}
	else
	{
		return true;
	}
}

/************************************************
  add a link
************************************************/
void Alignment::addLink(const pair<int, int>& link)
{
	linkVec.push_back(link);
	sort(linkVec.begin(), linkVec.end());

	srcAligned[link.first].push_back(link.second);
	sort(srcAligned[link.first].begin(), srcAligned[link.first].end());

	trgAligned[link.second].push_back(link.first);
	sort(trgAligned[link.second].begin(), trgAligned[link.second].end());

	int srcFert = (int)srcAligned[link.first].size();

	if (srcFert > srcMaxFert)
	{
		srcMaxFert = srcFert;
	}

	int trgFert = (int)trgAligned[link.second].size();

	if (trgFert > trgMaxFert)
	{
		trgMaxFert = trgFert;
	}
}

/************************************************
  judge whether a link exists
************************************************/
bool Alignment::exists(const pair<int, int>& link) const
{
	for (int i = 0; i < (int)linkVec.size(); i++)
	{
		if (linkVec[i] == link)
		{
			return true;
		}
	}

	return false;
}

