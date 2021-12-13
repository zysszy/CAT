#include "Link.h"

/************************************************
  constructors
************************************************/
Link::Link()
{
	sp = 0;
	tp = 0;
	prob = 0.0;
}

Link::Link(int sp,
	       int tp,
		   float prob)
{
	this->sp = sp;
	this->tp = tp;
	this->prob = prob;
}

/************************************************
  overload <<
************************************************/
ostream& operator<<(ostream& out, const Link& right)
{
	// Moses format
	out << right.sp - 1
	    << "-"
		<< right.tp - 1
		<< "/"
		<< right.prob;

	return out;
}

/************************************************
  overload =
************************************************/
Link& Link::operator=(const Link& right)
{
	if (this != &right)
	{
		sp = right.sp;
		tp = right.tp;
		prob = right.prob;
	}

	return *this;
}
	
/************************************************
  overload ==
************************************************/
bool Link::operator==(const Link& right) const
{
	if (sp == right.sp &&
		tp == right.tp &&
		prob == right.prob)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/************************************************
  overload <
************************************************/
bool Link::operator<(const Link& right) const
{
	if (prob < right.prob)
	{
		return true;
	}
	else if (prob == right.prob)
	{
		if (sp > right.sp)
		{
			return true;
		}
		else if (sp == right.sp)
		{
			if (tp > right.tp)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

/************************************************
  overload >
************************************************/
bool Link::operator>(const Link& right) const
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

