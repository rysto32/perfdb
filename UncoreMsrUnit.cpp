
#include "UncoreMsrUnit.h"
#include "UncoreEvent.h"

UncoreMsrUnit::UncoreMsrUnit(uint32_t baseMsr)
  : baseMsr(baseMsr)
{
}

void 
UncoreMsrUnit::FreezeCounters()
{
    SetMsrBit(baseMsr + PMON_BOX_CTL, PMON_BOX_CTL_FRZ);
}

void
UncoreMsrUnit::ThawCounters()
{
    ClearMsrBit(baseMsr + PMON_BOX_CTL, PMON_BOX_CTL_FRZ);
}

void
UncoreMsrUnit::ConfigureCounter(const UncoreCounter & counter, const UncoreEvent & ev)
{
    uint32_t ctl;

    FreezeCounters();
    ctl  = PMON_CTL_EN | PMON_CTL_RESET;
    ctl |= ev.GetUmask() << PMON_CTL_UMASK_SHIFT;
    ctl |= ev.GetCode() << PMON_CTL_EV_SEL_SHIFT;
    WriteMsr(baseMsr + counter.GetCtlOffset(), ctl);
    ThawCounters();
}

void
UncoreMsrUnit::UnconfigureCounter(const UncoreCounter &counter)
{
    FreezeCounters();
    SetMsrBit(baseMsr + counter.GetCtlOffset(), PMON_CTL_RESET);
    ThawCounters();
}

uint64_t
UncoreMsrUnit::GetCounterValue(const UncoreCounter & counter)
{
    FreezeCounters();
    uint64_t ctr = ReadMsr(baseMsr + counter.GetCtrOffset());    
    ThawCounters();
    
    return (ctr);
}
