
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
    cpu_type = ProbeCpuType();

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

    if (cpu_type == CPU_TYPE_HASWELL)
    {
        counters.push_back(UncoreCounter(UNCORE_R2PCIE, 0xD8, 0xA0));
        counters.push_back(UncoreCounter(UNCORE_R2PCIE, 0xDC, 0xA8));
        counters.push_back(UncoreCounter(UNCORE_R2PCIE, 0xE0, 0xB0));
        counters.push_back(UncoreCounter(UNCORE_R2PCIE, 0xE4, 0xB8));

        AddEvent(UncoreEvent("R2PCIE.RING_BL_USED.CW", UNCORE_R2PCIE, 0x09, 0x03));
        AddEvent(UncoreEvent("R2PCIE.RING_BL_USED.CCW", UNCORE_R2PCIE, 0x09, 0x0C));
        AddEvent(UncoreEvent("R2PCIE.CLOCKTICKS", UNCORE_R2PCIE, 0x01, 0));

        counters.push_back(UncoreCounter(UNCORE_CBOXN, 0x02, 0x09));
        counters.push_back(UncoreCounter(UNCORE_CBOXN, 0x03, 0x0A));
        counters.push_back(UncoreCounter(UNCORE_CBOXN, 0x04, 0x0B));
        counters.push_back(UncoreCounter(UNCORE_CBOX0, 0x01, 0x08));

        AddEvent(UncoreEvent("CBO.CLOCKTICKS", UNCORE_CBOX_ANY, 0x00, 0x00));
        AddEvent(UncoreEvent("CBO.RING_BL_USED.UP", UNCORE_CBOX_ANY, 0x1d, 0x03));
        AddEvent(UncoreEvent("CBO.RING_BL_USED.DOWN", UNCORE_CBOX_ANY, 0x1d, 0x0C));
    }

    // Integrated Memory Controller
    UncoreAgent *imc = new UncoreAgent(IMC_AGENT);
    agentsList.push_back(imc);
    agents[UNCORE_IMC_FIXED] = imc;
    agents[UNCORE_IMC] = imc;

    UncoreAgent *pcie = new UncoreAgent(R2PCIE_AGENT);
    agentsList.push_back(pcie);
    agents[UNCORE_R2PCIE] = pcie;

    UncoreAgent *cbox = new UncoreAgent(CBOX_AGENT);
    agentsList.push_back(cbox);
    agents[UNCORE_CBOX0] = cbox;
    agents[UNCORE_CBOXN] = cbox;

    switch (cpu_type)
    {
    case CPU_TYPE_SANDY_BRIDGE:
        imc->AddPciUnit(255, 16, 5);
        imc->AddPciUnit(255, 16, 0);
        imc->AddPciUnit(255, 16, 1);
        break;

    case CPU_TYPE_IVY_BRIDGE:
        imc->AddPciUnit(255, 16, 4);
        imc->AddPciUnit(255, 16, 5);
        imc->AddPciUnit(255, 16, 0);
        imc->AddPciUnit(255, 16, 1);
        break;
    case CPU_TYPE_HASWELL:
        imc->AddPciUnit(255, 20, 0);
        imc->AddPciUnit(255, 20, 1);
        imc->AddPciUnit(255, 21, 0);
        imc->AddPciUnit(255, 21, 1);

        imc->AddPciUnit(255, 23, 0);
        imc->AddPciUnit(255, 23, 1);
        imc->AddPciUnit(255, 24, 0);
        imc->AddPciUnit(255, 24, 1);

        pcie->AddPciUnit(255, 16, 1);

        for (int i = 0; i < 18; i++)
            cbox->AddMsrUnit(0x0E00 + 0x10 * i);
        break;
    }
}

UncoreContext::~UncoreContext()
{
    clearStats();
}

UncoreContext::CpuType UncoreContext::ProbeCpuType()
{
    CpuType type;

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
        type = CPU_TYPE_IVY_BRIDGE;
        break;
    case 0x2f008086:
        type = CPU_TYPE_HASWELL;
        break;
    case 0x3c008086:
        type = CPU_TYPE_SANDY_BRIDGE;
        break;
    default:
        type = CPU_TYPE_UNKNOWN;
        break;
    }

    close(fd);
    return (type);
}

CounterAgent
UncoreContext::getAgent(const std::string & stat)
{
    EventMap::iterator ev;

    ev = events.find(stat);

    if (ev == events.end()) {
        std::ostringstream msg;

        msg << "Event '" << stat << "' does not exist";
        throw StatException(msg.str());
    }

    uint32_t counters = ev->second.getCounters();
    int shift = ffs(counters) - 1;
    uint32_t agent = (1 << shift);

    return agents[agent]->GetCounterAgent();
}

void
UncoreContext::AddEvent(const UncoreEvent & event)
{
    events.insert(std::pair<std::string, UncoreEvent>(event.getName(), event));
}

void
UncoreContext::AllocateCounter(UncoreEvent & ev, UncoreCounter & counter)
{
    UncoreAgent *agent = agents.at(counter.GetAgentType());
    allocatedCounters[ev.getName()] = new UncoreAllocatedCounter(counter,
        agent->GetNumAgents());

    agent->ConfigureCounter(counter, ev);
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

    UncoreCounter & counter = it->second->GetCounter();
    UncoreAgent *agent = agents.at(counter.GetAgentType());
    total = 0;
    for (i = 0; i < agent->GetNumAgents(); i++) {
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
        UncoreAgent *agent = agents.at(counter.GetAgentType());
        for (unit = 0; unit < agent->GetNumAgents(); unit++) {
            value = agent->GetCounterValue(unit, counter);
            it->second->UpdateValue(unit, value);
        }
    }
}

int
UncoreContext::getNumAgents(CounterAgent agent) const
{
    uint32_t agentMask;

    switch (agent)
    {
    case CPU_CORE_AGENT:
        abort();
    case IMC_AGENT:
        agentMask = UNCORE_IMC;
        break;
    case R2PCIE_AGENT:
        agentMask = UNCORE_R2PCIE;
        break;
    case CBOX_AGENT:
        agentMask = UNCORE_CBOX0;
        break;

    case ANY_AGENT:
    case NO_AGENT:
        return (0);
    }

    return agents.at(agentMask)->GetNumAgents();
}

