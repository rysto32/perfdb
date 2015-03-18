
#include "UncoreAgent.h"

#include "StatContext.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/pciio.h>

#include <iostream>

UncoreAgent::UncoreAgent(CounterAgent agent)
  : agentType(agent)
{
}

UncoreAgent::~UncoreAgent()
{
    UnitMap::iterator it;

    for (it = units.begin(); it != units.end(); ++it) {
        delete *it;
    }
    units.clear();
}

void
UncoreAgent::AddUnit(int bus, int slot, int f)
{
    int fd = open("/dev/pci", O_RDWR);
    if (fd < 0)
        throw StatException("Could not open /dev/pci");

    struct pci_io io;
    io.pi_sel.pc_domain = 0;
    io.pi_sel.pc_bus = bus;
    io.pi_sel.pc_dev = slot;
    io.pi_sel.pc_func = f;

    io.pi_reg = 0;
    io.pi_width = 4;
    int error = ioctl(fd, PCIOCREAD, &io);

    if (error == 0)
        units.push_back(new UncoreUnit(bus, slot, f));
    else
        std::cerr << "Skip unit " << bus << ":" << slot << ":" << f << std::endl;

    close(fd);
}

#include <iostream>

void
UncoreAgent::ConfigureCounter(const UncoreCounter & counter, const UncoreEvent & ev)
{
    UnitMap::iterator it;

    for (it = units.begin(); it != units.end(); ++it) {
        (*it)->ConfigureCounter(counter, ev);
    }
}

void
UncoreAgent::UnconfigureCounter(const UncoreCounter &counter)
{
    UnitMap::iterator it;

    for (it = units.begin(); it != units.end(); ++it) {
        (*it)->UnconfigureCounter(counter);
    }
}

uint64_t
UncoreAgent::GetCounterValue(int unit, const UncoreCounter & counter)
{
    return units.at(unit)->GetCounterValue(counter);
}


CounterAgent
UncoreAgent::GetCounterAgent() const
{

    return (agentType);
}

int
UncoreAgent::GetNumAgents() const
{
    return units.size();
}
