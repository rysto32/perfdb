#ifndef STAT_CONTEXT_H
#define STAT_CONTEXT_H

#include <stdexcept>
#include <string>

class StatException : public std::runtime_error
{
	public:
	StatException(const std::string &msg)
	  : std::runtime_error(msg)
	{
	}
};

class StatNotLoaded : public std::exception
{
	const char *what() const throw ()
	{
		return "pmc not loaded";
	}
};

class StatContext 
{
public:

	virtual void loadStat(const std::string & name) = 0;
	virtual void clearStats() = 0;

	virtual uint64_t getStat(const std::string & name) throw (StatNotLoaded) = 0;
	virtual uint64_t getStatCpu(const std::string & name, int cpu) 
	    throw (StatNotLoaded) = 0;
	virtual void readStats() = 0;

	virtual int getNumUnits() const = 0;
};

#endif

