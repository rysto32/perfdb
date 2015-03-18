#ifndef PMCCONTEXT_H
#define PMCCONTEXT_H

#include <map>
#include <string>

#include <sys/types.h>
#include <pmc.h>

#include "StatContext.h"

class PmcContext : public StatContext
{
	struct Pmc
	{
		pmc_id_t m_id;
		uint64_t m_lastAbsolute;
		uint64_t m_value;

		Pmc(pmc_id_t id)
		    : m_id(id), m_lastAbsolute(0)
		{
		}
	};

	typedef std::map<int, Pmc> PmcCpuMap;
	typedef std::map<std::string, PmcCpuMap> PmcMap;

	PmcMap m_pmcs;
	int m_cpuMask;

	static std::string getPmcInitErrorMessage(int error);
	static std::string getPmcErrorMessage(const std::string & event, 
	    const char* function, int error);

	void clearCpuMap(PmcCpuMap & map);

	void readStat(const PmcMap::iterator & it);
public:
	PmcContext();

	~PmcContext()
	{
		clearStats();
	}

	void setCpuMask(int cpuMask)
	{
		m_cpuMask = cpuMask;
	}

	void loadStat(const std::string & name);
	void clearStats();

	uint64_t getStat(const std::string & name) throw (StatNotLoaded);
	uint64_t getStatCpu(const std::string & name, int cpu) 
	    throw (StatNotLoaded);
	void readStats();

	int getNumUnits() const;
};

#endif
