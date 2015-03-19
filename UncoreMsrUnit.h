
#ifndef UNCORE_MSR_UNIT_H
#define UNCORE_MSR_UNIT_H

#include "UncoreUnit.h"

class UncoreMsrUnit : public UncoreUnit
{
    uint32_t baseMsr;

    void FreezeCounters();
    void ThawCounters();
    
    static const uint64_t PMON_BOX_CTL = 0x00;
    static const uint64_t PMON_CTL = 0x01;
    static const uint64_t PMON_CTR = 0x08;

    static const uint32_t PMON_BOX_CTL_FRZ = (1 << 8);

    static const uint32_t PMON_CTL_EN = (1 << 22);
    static const uint32_t PMON_CTL_RESET = (1 << 17);
    static const uint32_t PMON_CTL_UMASK_SHIFT = 8;
    static const uint32_t PMON_CTL_EV_SEL_SHIFT = 0;
    
public:
    UncoreMsrUnit(uint32_t baseMsr);

    void ConfigureCounter(const UncoreCounter & counter, const UncoreEvent & ev);
    void UnconfigureCounter(const UncoreCounter &counter);

    uint64_t GetCounterValue(const UncoreCounter & counter);
};

#endif
