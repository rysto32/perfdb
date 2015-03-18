
#ifndef UNCORE_ALLOCATED_COUNTER_H
#define UNCORE_ALLOCATED_COUNTER_H

#include <stdint.h>
#include <vector>

class UncoreCounter;

class UncoreAllocatedCounter
{
private:
    UncoreCounter & counter;
    std::vector<uint64_t> values;
    std::vector<uint64_t> absoluteValues;

public:
    UncoreAllocatedCounter(UncoreCounter & counter, int num_units);

    UncoreCounter & GetCounter() const;

    uint64_t GetValue(int unit) const;
    void UpdateValue(int unit, uint64_t val);
};

#endif
