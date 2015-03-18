
#ifndef UNCORE_AGENT_H
#define UNCORE_AGENT_H

#include <map>
#include "UncoreUnit.h"

class UncoreCounter;
class UncoreEvent;

class UncoreAgent
{
private:
    typedef std::map<int, UncoreUnit*> UnitMap;
    UnitMap units;

public:
    ~UncoreAgent();

    void AddUnit(int num, int bus, int slot, int f);

    void ConfigureCounter(const UncoreCounter & counter, const UncoreEvent & ev);
    void UnconfigureCounter(const UncoreCounter &counter);

    uint64_t GetCounterValue(int unit, const UncoreCounter & counter);
};

#endif

