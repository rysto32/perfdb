
#ifndef UNCORE_PCI_UNIT_H
#define UNCORE_PCI_UNIT_H

#include <stdint.h>

#include "UncoreUnit.h"

struct pci_io;
class UncoreCounter;
class UncoreEvent;

class UncorePciUnit : public UncoreUnit
{
private:
    const int m_bus;
    const int m_slot;
    const int m_func;

    int m_pci_fd;

    static const uint32_t PMON_CTL_EN = (1 << 22);
    static const uint32_t PMON_CTL_RESET = (1 << 17);
    static const uint32_t PMON_CTL_UMASK_SHIFT = 8;
    static const uint32_t PMON_CTL_EV_SEL_SHIFT = 0;

    static const uint32_t PMON_BOX_CTL = 0xF4;
    static const uint32_t PMON_BOX_CTL_RSV = 0x3000;
    static const uint32_t PMON_BOX_CTL_FRZ = (1 << 8);

    static void FillPciIo(struct pci_io & io, int bus, int slot, int func, uint32_t reg);
    uint32_t ReadPci(uint32_t reg);
    uint32_t ReadPci(int bus, int slot, int func, uint32_t reg);
    void WritePci(uint32_t reg, uint32_t val);
    void WritePci(int bus, int slot, int func, uint32_t reg, uint32_t val);

    void FreezeCounters();
    void ThawCounters();

    UncorePciUnit(const UncoreUnit &);
    UncorePciUnit & operator=(const UncoreUnit &);

public:
    UncorePciUnit(int bus, int slot, int f);
    ~UncorePciUnit();

    void ConfigureCounter(const UncoreCounter & counter, const UncoreEvent & ev);
    void UnconfigureCounter(const UncoreCounter &counter);

    uint64_t GetCounterValue(const UncoreCounter & counter);
};

#endif

