#include <algorithm>
#include <map>
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

/************************************************
  itg 
************************************************/
bool Alignment::itg() const
{
	vector<pair<int, int> > alignment = linkVec;
	int srcSentLen = 0,
	    trgSentLen = 0;

	// must be one-to-one alignment
	for (int i = 0; i < (int)alignment.size(); i++)
	{
		if (alignment[i].first > srcSentLen)
		{
			srcSentLen = alignment[i].first;
		}

		if (alignment[i].second > trgSentLen)
		{
			trgSentLen = alignment[i].second;
		}

		for (int j = i + 1; j < (int)alignment.size(); j++)
		{
			if (alignment[i].first == alignment[j].first ||
				alignment[i].second == alignment[j].second)
			{
				return false;
			}
		}
	}

	if (alignment.size() < 4)
	{
		return true;
	}

	// compute alignedness
	vector<int>srcCov(srcSentLen + 1, 0),
	           trgCov(trgSentLen + 1, 0);

	for (int i = 0; i < (int)alignment.size(); i++)
	{
		srcCov[alignment[i].first] = 1;
		trgCov[alignment[i].second] = 1;
	}

	// compute empty position count
	vector<int> srcEmptyCount(srcSentLen + 1, 0),
	            trgEmptyCount(trgSentLen + 1, 0);

	for (int i = 1; i < (int)srcCov.size(); i++)
	{
		if (i == 1)
		{
			if (srcCov[i] == 0)
			{
				srcEmptyCount[i] = 1;
			}
		}
		else
		{
			srcEmptyCount[i] = srcEmptyCount[i - 1];

			if (srcCov[i] == 0)
			{
				srcEmptyCount[i]++;
			}
		}
	}

	for (int i = 1; i < (int)trgCov.size(); i++)
	{
		if (i == 1)
		{
			if (trgCov[i] == 0)
			{
				trgEmptyCount[i] = 1;
			}
		}
		else
		{
			trgEmptyCount[i] = trgEmptyCount[i - 1];

			if (trgCov[i] == 0)
			{
				trgEmptyCount[i]++;
			}
		}
	}

	// sort alignment
	vector<pair<int, int> > a;
	int max_sp = 0,
	    max_tp = 0;

	for (int i = 0; i < (int)alignment.size(); i++)
	{
		int sp = alignment[i].first - srcEmptyCount[alignment[i].first - 1],
		    tp = alignment[i].second - trgEmptyCount[alignment[i].second - 1];
		a.push_back(pair<int, int>(sp, tp));

		if (sp > max_sp)
		{
			max_sp = sp;
		}

		if (tp > max_tp)
		{
			max_tp = tp;
		}
	}

	sort(a.begin(), a.end());

	if (0)
	{
		cout << "alignment: ";

		for (int i = 0; i < (int)alignment.size(); i++)
		{
			cout << alignment[i].first
			     << ":"
				 << alignment[i].second
				 << " ";
		}

		cout << "\na: ";

		for (int i = 0; i < (int)a.size(); i++)
		{
			cout << a[i].first
			     << ":"
				 << a[i].second
				 << " ";
		}

		cout << endl;
	}

	// shift-reduce
	vector<vector<int> > stack;
	vector<int> block;
	block.push_back(a[0].first);
	block.push_back(a[0].first);
	block.push_back(a[0].second);
	block.push_back(a[0].second);
	stack.push_back(block);
	int i = 1;

	bool itg = true;

	while (!stack.empty())
	{
		if (stack.size() == 1)
		{
			// suceessful
			if (stack[0][0] == 1 &&
				stack[0][1] == max_sp &&
				stack[0][2] == 1 &&
				stack[0][3] == max_tp &&
				i == a.size())
			{
				stack.pop_back();
			}
			// shift
			else
			{
				vector<int> block;
				block.push_back(a[i].first);
				block.push_back(a[i].first);
				block.push_back(a[i].second);
				block.push_back(a[i].second);
				stack.push_back(block);
				i++;
			}
		}
		else
		{
			// reduce
			bool reduced = false;
			int size = stack.size();

			// monotone
			if (stack[size - 2][1] + 1 == stack[size - 1][0] &&
				stack[size - 2][3] + 1 == stack[size - 1][2])
			{
				vector<int> block;
				block.push_back(stack[size - 2][0]);
				block.push_back(stack[size - 1][1]);
				block.push_back(stack[size - 2][2]);
				block.push_back(stack[size - 1][3]);

				stack.pop_back();
				stack.pop_back();
				stack.push_back(block);

				reduced = true;
			}
			// swap
			else if (stack[size - 2][1] + 1 == stack[size - 1][0] &&
				     stack[size - 1][3] + 1 == stack[size - 2][2])
			{
				vector<int> block;
				block.push_back(stack[size - 2][0]);
				block.push_back(stack[size - 1][1]);
				block.push_back(stack[size - 1][2]);
				block.push_back(stack[size - 2][3]);

				stack.pop_back();
				stack.pop_back();
				stack.push_back(block);

				reduced = true;
			}

			// shift
			if (!reduced)
			{
				if (i < (int)a.size())
				{
					vector<int> block;
					block.push_back(a[i].first);
					block.push_back(a[i].first);
					block.push_back(a[i].second);
					block.push_back(a[i].second);
					stack.push_back(block);
					i++;
				}
				else
				{
					itg = false;
					break;
				}
			}
		}
	}

	return itg;
}

/************************************************
  bitg
************************************************/
bool Alignment::bitg() const
{
	// compute sentence length
	vector<pair<int, int> > alignment = linkVec;
	int srcSentLen = 0,
	    trgSentLen = 0;

	for (int i = 0; i < (int)alignment.size(); i++)
	{
		if (alignment[i].first > srcSentLen)
		{
			srcSentLen = alignment[i].first;
		}

		if (alignment[i].second > trgSentLen)
		{
			trgSentLen = alignment[i].second;
		}
	}

	// compute alignedness
	vector<int>srcCov(srcSentLen + 1, 0),
	           trgCov(trgSentLen + 1, 0);

	for (int i = 0; i < (int)alignment.size(); i++)
	{
		srcCov[alignment[i].first] = 1;
		trgCov[alignment[i].second] = 1;
	}

	// compute empty position count
	vector<int> srcEmptyCount(srcSentLen + 1, 0),
	            trgEmptyCount(trgSentLen + 1, 0);

	for (int i = 1; i < (int)srcCov.size(); i++)
	{
		if (i == 1)
		{
			if (srcCov[i] == 0)
			{
				srcEmptyCount[i] = 1;
			}
		}
		else
		{
			srcEmptyCount[i] = srcEmptyCount[i - 1];

			if (srcCov[i] == 0)
			{
				srcEmptyCount[i]++;
			}
		}
	}

	for (int i = 1; i < (int)trgCov.size(); i++)
	{
		if (i == 1)
		{
			if (trgCov[i] == 0)
			{
				trgEmptyCount[i] = 1;
			}
		}
		else
		{
			trgEmptyCount[i] = trgEmptyCount[i - 1];

			if (trgCov[i] == 0)
			{
				trgEmptyCount[i]++;
			}
		}
	}

	if (0)
	{
		cout << "\nsrcSentLen: "
		     << srcSentLen
			 << "\ntrgSentLen: "
			 << trgSentLen
			 << endl;

		cout << "\nsrcCov: ";

		for (int i = 0; i < (int)srcCov.size(); i++)
		{
			cout << srcCov[i]
			     << " ";
		}

		cout << "\ntrgCov: ";

		for (int i = 0; i < (int)trgCov.size(); i++)
		{
			cout << trgCov[i]
			     << " ";
		}

		cout << "\nsrcEmptyCount: ";

		for (int i = 0; i < (int)srcEmptyCount.size(); i++)
		{
			cout << srcEmptyCount[i]
			     << " ";
		}

		cout << "\ntrgEmptyCount: ";

		for (int i = 0; i < (int)trgEmptyCount.size(); i++)
		{
			cout << trgEmptyCount[i]
			     << " ";
		}
	}

	// remove unaligned positions
	vector<pair<int, int> > a;
	int max_sp = 0,
	    max_tp = 0;

	for (int i = 0; i < (int)alignment.size(); i++)
	{
		int sp = alignment[i].first - srcEmptyCount[alignment[i].first - 1],
		    tp = alignment[i].second - trgEmptyCount[alignment[i].second - 1];
		a.push_back(pair<int, int>(sp, tp));

		if (sp > max_sp)
		{
			max_sp = sp;
		}

		if (tp > max_tp)
		{
			max_tp = tp;
		}
	}

	sort(a.begin(), a.end());

	// compute siblings
	map<int, vector<int> > srcSib,
	                       trgSib;

	for (int i = 1; i <= max_sp; i++)
	{
		vector<int> sib;
		srcSib.insert(map<int, vector<int> >::value_type(i, sib));
	}

	for (int i = 1; i <= max_tp; i++)
	{
		vector<int> sib;
		trgSib.insert(map<int, vector<int> >::value_type(i, sib));
	}

	for (int i = 0; i < (int)a.size(); i++)
	{
		int sp = a[i].first,
		    tp = a[i].second;

		// update source siblings
		map<int, vector<int> >::iterator iter = srcSib.find(sp);
		iter->second.push_back(tp);

		// update target siblings
		iter = trgSib.find(tp);
		iter->second.push_back(sp);
	}

	map<int, vector<int> >::iterator iter;

	for (iter = srcSib.begin(); iter != srcSib.end(); iter++)
	{
		sort(iter->second.begin(), iter->second.end());
	}

	for (iter = trgSib.begin(); iter != trgSib.end(); iter++)
	{
		sort(iter->second.begin(), iter->second.end());
	}

	// no many-to-many links
	for (int i = 0; i < (int)a.size(); i++)
	{
		int sp = a[i].first,
		    tp = a[i].second;
		map<int, vector<int> >::iterator iter1 = srcSib.find(sp),
		                                 iter2 = trgSib.find(tp);

		if ((int)iter1->second.size() > 1 &&
			(int)iter2->second.size() > 1)
		{
			if (0)
			{
				cout << "many-to-many: " << *this << endl;
			}

			return false;
		}
	}

	// no links break in one-to-many links
	for (iter = srcSib.begin(); iter != srcSib.end(); iter++)
	{
		if ((int)iter->second.size() < 2)
		{
			continue;
		}

		for (int i = 0; i < (int)a.size(); i++)
		{
			if (a[i].first != iter->first &&
				a[i].second >= iter->second[0] &&
				a[i].second <= iter->second.back())
			{
				if (0)
				{
					cout << "source break in: " << *this << endl;
				}

				return false;
			}
		}
	}

	for (iter = trgSib.begin(); iter != trgSib.end(); iter++)
	{
		if ((int)iter->second.size() < 2)
		{
			continue;
		}

		for (int i = 0; i < (int)a.size(); i++)
		{
			if (a[i].second != iter->first &&
				a[i].first >= iter->second[0] &&
				a[i].first <= iter->second.back())
			{
				if (0)
				{
					cout << "target break in: " << *this << endl;
				}

				return false;
			}
		}
	}

	// build blocks
	vector<vector<int> > blockVec;

	for (int i = 1; i <= max_sp; i++)
	{
		map<int, vector<int> >::iterator iter1 = srcSib.find(i),
		                                 iter2 = trgSib.find(iter1->second[0]);

		// one-to-one
		if (iter1->second.size() == 1 &&
			iter2->second.size() == 1)
		{
			vector<int> block;
			block.push_back(i);
			block.push_back(i);
			block.push_back(iter1->second[0]);
			block.push_back(iter1->second[0]);

			blockVec.push_back(block);
		}
		// one-to-many
		else if (iter1->second.size() > 1)
		{
			vector<int> block;
			block.push_back(i);
			block.push_back(i);
			block.push_back(iter1->second[0]);
			block.push_back(iter1->second.back());

			blockVec.push_back(block);
		}
		// many-to-one
		else if (iter2->second.size() > 1)
		{
			vector<int> block;
			block.push_back(iter2->second[0]);
			block.push_back(iter2->second.back());
			block.push_back(iter1->second[0]);
			block.push_back(iter1->second[0]);

			blockVec.push_back(block);

			i = iter2->second.back();
		}
		// otherwise ...
		else
		{
			cout << "You are not supposed to be here!" << endl;
			exit(1);
		}
	}

	if (0)
	{
		cout << "alignment: ";

		for (int i = 0; i < (int)alignment.size(); i++)
		{
			cout << alignment[i].first
			     << ":"
				 << alignment[i].second
				 << " ";
		}

		cout << "\na: ";

		for (int i = 0; i < (int)a.size(); i++)
		{
			cout << a[i].first
			     << ":"
				 << a[i].second
				 << " ";
		}

		cout << "\nmax_sp: "
		     << max_sp
			 << "\nmax_tp: "
			 << max_tp
			 << endl;

		cout << "\nblockVec: ";

		for (int i = 0; i < (int)blockVec.size(); i++)
		{
			cout << blockVec[i][0]
			     << "-"
				 << blockVec[i][1]
				 << "-"
				 << blockVec[i][2]
				 << "-"
				 << blockVec[i][3]
				 << " ";
		}

		cout << endl;
	}
	
	// shift-reduce
	vector<vector<int> > stack;
	stack.push_back(blockVec[0]);
	int i = 1;

	bool itg = true;

	while (!stack.empty())
	{
		if (stack.size() == 1)
		{
			// suceessful
			if (stack[0][0] == 1 &&
				stack[0][1] == max_sp &&
				stack[0][2] == 1 &&
				stack[0][3] == max_tp &&
				i == blockVec.size())
			{
				stack.pop_back();
			}
			// shift
			else
			{
				stack.push_back(blockVec[i]);
				i++;
			}
		}
		else
		{
			// reduce
			bool reduced = false;
			int size = stack.size();

			// monotone
			if (stack[size - 2][1] + 1 == stack[size - 1][0] &&
				stack[size - 2][3] + 1 == stack[size - 1][2])
			{
				vector<int> block;
				block.push_back(stack[size - 2][0]);
				block.push_back(stack[size - 1][1]);
				block.push_back(stack[size - 2][2]);
				block.push_back(stack[size - 1][3]);

				stack.pop_back();
				stack.pop_back();
				stack.push_back(block);

				reduced = true;
			}
			// swap
			else if (stack[size - 2][1] + 1 == stack[size - 1][0] &&
				     stack[size - 1][3] + 1 == stack[size - 2][2])
			{
				vector<int> block;
				block.push_back(stack[size - 2][0]);
				block.push_back(stack[size - 1][1]);
				block.push_back(stack[size - 1][2]);
				block.push_back(stack[size - 2][3]);

				stack.pop_back();
				stack.pop_back();
				stack.push_back(block);

				reduced = true;
			}

			// shift
			if (!reduced)
			{
				if (i < (int)blockVec.size())
				{
					stack.push_back(blockVec[i]);
					i++;
				}
				else
				{
					itg = false;
					break;
				}
			}
		}
	}

	return itg;
}

