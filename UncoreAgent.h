
#ifndef UNCORE_AGENT_H
#define UNCORE_AGENT_H

#include <vector>

#include "CounterAgent.h"
#include "UncoreUnit.h"

class UncoreCounter;
class UncoreEvent;

class UncoreAgent
{
private:
    typedef std::vector<UncoreUnit*> UnitMap;
    UnitMap units;

    CounterAgent agentType;

public:
    UncoreAgent(CounterAgent agent);
    ~UncoreAgent();

    void AddPciUnit(int bus, int slot, int f);
    void AddMsrUnit(uint32_t base_msr);

    void ConfigureCounter(const UncoreCounter & counter, const UncoreEvent & ev);
    void UnconfigureCounter(const UncoreCounter &counter);

    uint64_t GetCounterValue(int unit, const UncoreCounter & counter);

    CounterAgent GetCounterAgent() const;
    int GetNumAgents() const;
};

#endif

