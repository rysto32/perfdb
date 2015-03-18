
#include "UncoreAllocatedCounter.h"

UncoreAllocatedCounter::UncoreAllocatedCounter(UncoreCounter & cnt, int num_units)
    : counter(cnt), values(num_units), absoluteValues(num_units)
{
}

UncoreCounter & 
UncoreAllocatedCounter::GetCounter() const
{
    return counter;
}

uint64_t
UncoreAllocatedCounter::GetValue(int unit) const
{
    return values.at(unit);
}

void
UncoreAllocatedCounter::UpdateValue(int unit, uint64_t val)
{
    values.at(unit) = val - absoluteValues.at(unit);
    absoluteValues.at(unit) = val;
}

