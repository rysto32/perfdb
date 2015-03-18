
#include "UncoreCounter.h"
#include "UncoreEvent.h"

#include <assert.h>

UncoreCounter::UncoreCounter(uint32_t type, uint32_t ctl, uint32_t ctr)
  : agent_type(type), ctl_offset(ctl), ctr_offset(ctr), free(true)
{
}



bool
UncoreCounter::AllocateFor(UncoreEvent & event)
{
    if (!free)
        return false;

    // Check that this event can be configued in this counter type. 
    if ((agent_type & event.getCounters()) == 0)
        return false;

    free = false;
    return true;
}


void
UncoreCounter::Release()
{
    assert (!free);
    free = true;
}

uint32_t
UncoreCounter::GetAgentType() const
{
    return agent_type;
}

uint32_t
UncoreCounter::GetCtlOffset() const
{
    return ctl_offset;
}

uint32_t
UncoreCounter::GetCtrOffset() const
{
    return ctr_offset;
}

