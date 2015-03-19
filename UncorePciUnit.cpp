
#include "UncorePciUnit.h"
#include "StatContext.h"
#include "UncoreCounter.h"
#include "UncoreEvent.h"

#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/cpuctl.h>
#include <sys/pciio.h>


UncorePciUnit::UncorePciUnit(int b, int s, int f)
    : m_bus(b), m_slot(s), m_func(f)
{

    m_pci_fd = open("/dev/pci", O_RDWR);

    if (m_pci_fd < 0)
        throw StatException("Could not open /dev/pci");
}

UncorePciUnit::~UncorePciUnit()
{
    close(m_pci_fd);
}

void
UncorePciUnit::FillPciIo(struct pci_io & io, int bus, int slot, int func, uint32_t reg)
{

    io.pi_sel.pc_domain = 0;
    io.pi_sel.pc_bus = bus;
    io.pi_sel.pc_dev = slot;
    io.pi_sel.pc_func = func;

    io.pi_reg = reg;
    io.pi_width = 4;
}

uint32_t
UncorePciUnit::ReadPci(uint32_t reg)
{
    return ReadPci(m_bus, m_slot, m_func, reg);
}

uint32_t
UncorePciUnit::ReadPci(int bus, int slot, int func, uint32_t reg)
{
    struct pci_io io;
    int error;

    FillPciIo(io, bus, slot, func, reg);

    error = ioctl(m_pci_fd, PCIOCREAD, &io);

    if (error)
        throw StatException("Error reading from pci device");

    return io.pi_data;
}

void
UncorePciUnit::WritePci(uint32_t reg, uint32_t val)
{
    WritePci(m_bus, m_slot, m_func, reg, val);
}

void
UncorePciUnit::WritePci(int bus, int slot, int func, uint32_t reg, uint32_t val)
{
    struct pci_io io;
    int error;

    FillPciIo(io, bus, slot, func, reg);
    io.pi_data = val;

    error = ioctl(m_pci_fd, PCIOCWRITE, &io);

    if (error)
        throw StatException("Error writing to pci device");
}

void
UncorePciUnit::FreezeCounters()
{
    //SetMsrBit(U_MSR_PMON_GLOBAL_CTL, MSR_PMON_FRZ_ALL);
    WritePci(PMON_BOX_CTL, PMON_BOX_CTL_RSV | PMON_BOX_CTL_FRZ);
}

void
UncorePciUnit::ThawCounters()
{
    //SetMsrBit(U_MSR_PMON_GLOBAL_CTL, MSR_PMON_UNFRZ_ALL);
    WritePci(PMON_BOX_CTL, PMON_BOX_CTL_RSV);
}

void
UncorePciUnit::ConfigureCounter(const UncoreCounter & counter, const UncoreEvent & ev)
{
    FreezeCounters();

    uint32_t ctl;

    ctl  = PMON_CTL_EN | PMON_CTL_RESET;
    ctl |= ev.GetUmask() << PMON_CTL_UMASK_SHIFT;
    ctl |= ev.GetCode() << PMON_CTL_EV_SEL_SHIFT;

    WritePci(counter.GetCtlOffset(), ctl);

    ThawCounters();
}

void
UncorePciUnit::UnconfigureCounter(const UncoreCounter &counter)
{

    // Clear the enable bit and reset the counter
    WritePci(counter.GetCtlOffset(), PMON_CTL_RESET);
}

uint64_t
UncorePciUnit::GetCounterValue(const UncoreCounter & counter)
{
    FreezeCounters();
    uint32_t low;
    uint64_t high;

    low = ReadPci(counter.GetCtrOffset());
    high = ReadPci(counter.GetCtrOffset() + 4);

    ThawCounters();

    return (high << 32) | low;
}


