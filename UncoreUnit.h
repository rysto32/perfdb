
#ifndef UNCORE_UNIT_H
#define UNCORE_UNIT_H

#include <stdint.h>

struct pci_io;
class UncoreCounter;
class UncoreEvent;

class UncoreUnit
{
private:
    const int m_bus;
    const int m_slot;
    const int m_func;

    int m_pci_fd;
    int m_msr_fd;

    static const int U_MSR_PMON_GLOBAL_CTL = 0xC00;
    static const uint32_t MSR_PMON_FRZ_ALL = (1 << 31);
    static const uint32_t MSR_PMON_UNFRZ_ALL = (1 << 29);

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
    void SetMsrBit(uint32_t msr, uint32_t bit);

    void FreezeCounters();
    void ThawCounters();

    UncoreUnit(const UncoreUnit &);
    UncoreUnit & operator=(const UncoreUnit &);

public:
    UncoreUnit(int bus, int slot, int f);
    ~UncoreUnit();

    int GetUnitNum() const;

    void ConfigureCounter(const UncoreCounter & counter, const UncoreEvent & ev);
    void UnconfigureCounter(const UncoreCounter &counter);

    uint64_t GetCounterValue(const UncoreCounter & counter);
};

#endif

