
#include "UncoreAgent.h"

UncoreAgent::~UncoreAgent()
{
    UnitMap::iterator it;
    
    for (it = units.begin(); it != units.end(); ++it) {
        delete it->second;
    }
    units.clear();
}

void
UncoreAgent::AddUnit(int num, int bus, int slot, int f)
{

    units.insert(std::pair<int, UncoreUnit*>(num, new UncoreUnit(num, bus, slot, f)));
}

#include <iostream>

void
UncoreAgent::ConfigureCounter(const UncoreCounter & counter, const UncoreEvent & ev)
{
    UnitMap::iterator it;

    for (it = units.begin(); it != units.end(); ++it) {
        it->second->ConfigureCounter(counter, ev);
    }
}

void
UncoreAgent::UnconfigureCounter(const UncoreCounter &counter)
{
    UnitMap::iterator it;

    for (it = units.begin(); it != units.end(); ++it) {
        it->second->UnconfigureCounter(counter);
    }
}

uint64_t
UncoreAgent::GetCounterValue(int unit, const UncoreCounter & counter)
{
    return units.at(unit)->GetCounterValue(counter);
}

