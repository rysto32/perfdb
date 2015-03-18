
#include "UncoreContext.h"

#include "StatContext.h"
#include "UncoreAgent.h"
#include "UncoreAllocatedCounter.h"

#include <errno.h>
#include <iostream>
#include <fcntl.h>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/pciio.h>

UncoreContext::UncoreContext()
{
    int fd = open("/dev/pci", O_RDWR);
    if (fd < 0)
        throw StatException("Could not open /dev/pci");

    struct pci_io io;
    io.pi_sel.pc_domain = 0;
    io.pi_sel.pc_bus = 0;
    io.pi_sel.pc_dev = 0;
    io.pi_sel.pc_func = 0;

    io.pi_reg = 0;
    io.pi_width = 4;
    int error = ioctl(fd, PCIOCREAD, &io);

    if (error)
        throw StatException("Error reading from pci device");

    uint32_t did = io.pi_data;
    
    switch (did)
    {
    case 0xe008086:
        cpu_type = CPU_TYPE_IVY_BRIDGE;
        break;
    case 0x2f008086:
        cpu_type = CPU_TYPE_HASWELL;
        break;
    case 0x3c008086:
        cpu_type = CPU_TYPE_SANDY_BRIDGE;
        break;
    default:
        cpu_type = CPU_TYPE_UNKNOWN;
        break;
    }

    counters.push_back(UncoreCounter(UNCORE_IMC_FIXED, 0xF0, 0xD0));
    counters.push_back(UncoreCounter(UNCORE_IMC, 0xD8, 0xA0));
    counters.push_back(UncoreCounter(UNCORE_IMC, 0xDC, 0xA8));
    counters.push_back(UncoreCounter(UNCORE_IMC, 0xE0, 0xB0));
    counters.push_back(UncoreCounter(UNCORE_IMC, 0xE4, 0xB8));

    counters.push_back(UncoreCounter(UNCORE_R2PCIE, 0xD8, 0xA0));
    counters.push_back(UncoreCounter(UNCORE_R2PCIE, 0xDC, 0xA8));
    counters.push_back(UncoreCounter(UNCORE_R2PCIE, 0xE0, 0xB0));
    counters.push_back(UncoreCounter(UNCORE_R2PCIE, 0xE4, 0xB8));

    AddEvent(UncoreEvent("IMC.RPQ_CYCLES_NE", UNCORE_IMC, 0x11, 1));
    AddEvent(UncoreEvent("IMC.WPQ_CYCLES_NE", UNCORE_IMC, 0x21, 1));
    AddEvent(UncoreEvent("IMC.CAS_COUNT.RD", UNCORE_IMC, 0x4, 0x3));
    AddEvent(UncoreEvent("IMC.CAS_COUNT.WR", UNCORE_IMC, 0x4, 0xC));
    AddEvent(UncoreEvent("IMC.FIXED", UNCORE_IMC_FIXED, 0, 0));
    
    if (cpu_type == CPU_TYPE_HASWELL)
    {
        AddEvent(UncoreEvent("R2PCIE.RING_BL_USED.CW", UNCORE_R2PCIE, 0x09, 0x03));
        AddEvent(UncoreEvent("R2PCIE.RING_BL_USED.CCW", UNCORE_R2PCIE, 0x09, 0x0C));
        AddEvent(UncoreEvent("R2PCIE.CLOCKTICKS", UNCORE_R2PCIE, 0x01, 0));
    }

    // Integrated Memory Controller
    UncoreAgent *imc = new UncoreAgent;
    agentsList.push_back(imc);

    UncoreAgent *pcie = new UncoreAgent;
    agentsList.push_back(pcie);

    switch (cpu_type)
    {
    case CPU_TYPE_SANDY_BRIDGE:
        imc->AddUnit(0, 255, 16, 5);
        imc->AddUnit(1, 255, 16, 0);
        imc->AddUnit(2, 255, 16, 1);
        break;
    
    case CPU_TYPE_IVY_BRIDGE:
        imc->AddUnit(0, 255, 16, 4);
        imc->AddUnit(1, 255, 16, 5);
        imc->AddUnit(2, 255, 16, 0);
        imc->AddUnit(3, 255, 16, 1);
        break;
    case CPU_TYPE_HASWELL:
        imc->AddUnit(0, 255, 20, 0);
        imc->AddUnit(1, 255, 20, 1);
        imc->AddUnit(2, 255, 23, 0);
        
        io.pi_sel.pc_bus = 255;
        io.pi_sel.pc_dev = 23;
        io.pi_sel.pc_func = 1;
        error = ioctl(fd, PCIOCREAD, &io);
        
        if (error == 0)
            imc->AddUnit(3, 255, 23, 1);
        
        imc->AddUnit(4, 255, 21, 0);
        imc->AddUnit(5, 255, 21, 1);
        imc->AddUnit(6, 255, 24, 0);
        imc->AddUnit(7, 255, 24, 1);
        
        pcie->AddUnit(0, 255, 16, 1);
        break;
    }

    agents[UNCORE_IMC_FIXED] = imc;
    agents[UNCORE_IMC] = imc;
    agents[UNCORE_R2PCIE] = pcie;

    close(fd);
    fd = -1;
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

    switch (cpu_type)
    {
    case CPU_TYPE_SANDY_BRIDGE:
        return (3);
    
    case CPU_TYPE_IVY_BRIDGE:
        return (4);

    case CPU_TYPE_HASWELL:
        return (8);
    }
}

