#ifndef STATISTIC_H
#define STATISTIC_H

#include <memory>
#include <math.h>
#include <map>

#include "expression.h"
#include "pmccontext.h"

class Statistic
{
	std::auto_ptr<std::string> name;
	std::auto_ptr<Expression> expr;

	double goodThreshold;
	double okThreshold;
	double badThreshold;

	std::map<int, double> cachedValues;

	enum { ALL_CPUS = -1 };

	/* true if a larger value is better */
	bool ascending() const
	{
		return (goodThreshold > okThreshold);
	}

public:
	enum Status { STAT_GOOD = 1, STAT_OK, STAT_BAD, STAT_TERRIBLE };

	Statistic(std::string *n, Expression *e, double good, double ok, 
	    double bad)
		: name(n), expr(e), goodThreshold(good), okThreshold(ok),
		    badThreshold(bad)
	{
	}

	Statistic(const std::string &n, Expression *e, double good, double ok, 
	    double bad)
		: name(new std::string(n)), expr(e), goodThreshold(good), 
		    okThreshold(ok), badThreshold(bad)
	{
	}

	const std::string &getName() const
	{
		return *name.get();
	}

	Expression &getExpr() const
	{
		return *expr.get();
	}

	Status getStatus(double value) const
	{
		if (ascending()) {
			if (isgreater(value, goodThreshold))
				return (STAT_GOOD);
			else if (isgreater(value, okThreshold))
				return (STAT_OK);
			else if (isgreater(value, badThreshold))
				return (STAT_BAD);
			else
				return (STAT_TERRIBLE);
		} else {
			if (isless(value, goodThreshold))
				return (STAT_GOOD);
			else if (isless(value, okThreshold))
				return (STAT_OK);
			else if (isless(value, badThreshold))
				return (STAT_BAD);
			else
				return (STAT_TERRIBLE);
		}
	}

	void setLastValue(double value, int cpu = ALL_CPUS)
	{
		cachedValues[cpu] = value;
	}

	double getLastValue(int cpu = ALL_CPUS)
	{
		std::map<int, double>::iterator it = cachedValues.find(cpu);

		if(it == cachedValues.end())
			throw PmcNotLoaded();

		return (it->second);
	}
};

#endif
