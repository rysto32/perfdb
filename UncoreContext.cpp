
#include "UncoreContext.h"

#include "StatContext.h"
#include "UncoreAgent.h"
#include "UncoreAllocatedCounter.h"

#include <sstream>

UncoreContext::UncoreContext()
{
    counters.push_back(UncoreCounter(UNCORE_IMC_FIXED, 0xF0, 0xD0));
    counters.push_back(UncoreCounter(UNCORE_IMC, 0xD8, 0xA0));
    counters.push_back(UncoreCounter(UNCORE_IMC, 0xDC, 0xA8));
    counters.push_back(UncoreCounter(UNCORE_IMC, 0xE0, 0xB0));
    counters.push_back(UncoreCounter(UNCORE_IMC, 0xE4, 0xB8));

    AddEvent(UncoreEvent("IMC.RPQ_CYCLES_NE", UNCORE_IMC, 0x11, 1));
    AddEvent(UncoreEvent("IMC.WPQ_CYCLES_NE", UNCORE_IMC, 0x21, 1));
    AddEvent(UncoreEvent("IMC.CAS_COUNT.RD", UNCORE_IMC, 0x4, 0x3));
    AddEvent(UncoreEvent("IMC.CAS_COUNT.WR", UNCORE_IMC, 0x4, 0xC));
    AddEvent(UncoreEvent("IMC.FIXED", UNCORE_IMC_FIXED, 0, 0));

    // Integrated Memory Controller
    UncoreAgent *imc = new UncoreAgent;
    agentsList.push_back(imc);
    /*imc->AddUnit(0, 255, 16, 4);
    imc->AddUnit(1, 255, 16, 5);
    imc->AddUnit(2, 255, 16, 0);
    imc->AddUnit(3, 255, 16, 1);*/
    
    imc->AddUnit(0, 255, 16, 5);
    imc->AddUnit(1, 255, 16, 0);
    imc->AddUnit(2, 255, 16, 1);

    agents[UNCORE_IMC_FIXED] = imc;
    agents[UNCORE_IMC] = imc;
}

UncoreContext::~UncoreContext()
{
    clearStats();
}

void
UncoreContext::AddEvent(const UncoreEvent & event)
{
    events.insert(std::pair<std::string, UncoreEvent>(event.getName(), event));
}

void
UncoreContext::AllocateCounter(UncoreEvent & ev, UncoreCounter & counter)
{

    allocatedCounters[ev.getName()] = new UncoreAllocatedCounter(counter, getNumUnits());;

    fprintf(stderr, "agent type = %x\n", counter.GetAgentType());
    agents[counter.GetAgentType()]->ConfigureCounter(counter, ev);
}

void
UncoreContext::loadStat(const std::string & name)
{
    // If we are already counting this event then there is nothing to do.
    if (allocatedCounters.count(name) != 0)
        return;
    
    EventMap::iterator ev;

    ev = events.find(name);

    if (ev == events.end()) {
        std::ostringstream msg;

        msg << "Event '" << name << "' does not exist";
        throw StatException(msg.str());
    }

    CounterList::iterator it;
    for (it = counters.begin(); it != counters.end(); ++it) {
        if (it->AllocateFor(ev->second)) {
            AllocateCounter(ev->second, *it);
            return;
        }
    }

    throw StatException("Could not allocate counter");
}


void
UncoreContext::clearStats()
{
    AllocatedCounterMap::iterator it;

    for (it = allocatedCounters.begin(); it != allocatedCounters.end(); ++it) {
        UncoreAllocatedCounter *alloc = it->second;
        UncoreCounter & counter = alloc->GetCounter();

        agents[counter.GetAgentType()]->UnconfigureCounter(counter);
        counter.Release();
        delete alloc;
    }
    allocatedCounters.clear();
}

uint64_t
UncoreContext::getStat(const std::string & name) throw (StatNotLoaded)
{
    uint64_t total;
    int i;

    AllocatedCounterMap::iterator it = allocatedCounters.find(name);

    if (it == allocatedCounters.end())
        throw StatNotLoaded();

    total = 0;
    for (i = 0; i < getNumUnits(); i++) {
        total += it->second->GetValue(i);
    }

    return (total);
}

uint64_t
UncoreContext::getStatCpu(const std::string & name, int cpu) throw (StatNotLoaded)
{
    AllocatedCounterMap::iterator it = allocatedCounters.find(name);

    if (it == allocatedCounters.end())
        throw StatNotLoaded();

    return (it->second->GetValue(cpu));
}

void
UncoreContext::readStats()
{
    AllocatedCounterMap::iterator it;
    int unit;
    uint64_t value;

    for (it = allocatedCounters.begin(); it != allocatedCounters.end(); ++it) {
        UncoreCounter & counter = it->second->GetCounter();
        for (unit = 0; unit < getNumUnits(); unit++) {
            value = agents[counter.GetAgentType()]->GetCounterValue(unit, counter);
            it->second->UpdateValue(unit, value);
        }
    }
}

int
UncoreContext::getNumUnits() const
{

    return (3);
}

