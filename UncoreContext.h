

#ifndef UNCORE_CONTEXT_H
#define UNCORE_CONTEXT_H

#include <map>
#include <vector>

#include "pointervector.h"
#include "StatContext.h"
#include "UncoreCounter.h"
#include "UncoreEvent.h"

class UncoreAgent;
class UncoreAllocatedCounter;

class UncoreContext : public StatContext
{
private:
	typedef std::vector<UncoreCounter> CounterList;
	CounterList counters;

	typedef std::map<std::string, UncoreAllocatedCounter*> AllocatedCounterMap;
	AllocatedCounterMap allocatedCounters;

	typedef std::map<std::string, UncoreEvent> EventMap;
	EventMap events;

    typedef std::map<int, UncoreAgent*> AgentMap;
    AgentMap agents;

    PointerVector<UncoreAgent> agentsList;

	void AddEvent(const UncoreEvent & event);
    void AllocateCounter(UncoreEvent & ev, UncoreCounter & counter);
	
public:
	UncoreContext();
	~UncoreContext();

	void loadStat(const std::string & name);
	void clearStats();

	uint64_t getStat(const std::string & name) throw (StatNotLoaded);
	uint64_t getStatCpu(const std::string & name, int cpu) 
	    throw (StatNotLoaded);
	void readStats();

	int getNumUnits() const;
};

#endif

