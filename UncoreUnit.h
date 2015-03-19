
#ifndef UNCORE_UNIT_H
#define UNCORE_UNIT_H

#include <stdint.h>

struct pci_io;
class UncoreCounter;
class UncoreEvent;

class UncoreUnit
{
private:
    int m_msr_fd;

    static const int U_MSR_PMON_GLOBAL_CTL = 0xC00;
    static const uint32_t MSR_PMON_FRZ_ALL = (1 << 31);
    static const uint32_t MSR_PMON_UNFRZ_ALL = (1 << 29);

    UncoreUnit(const UncoreUnit &);
    UncoreUnit & operator=(const UncoreUnit &);

protected:
    void SetMsrBit(uint32_t msr, uint32_t bit);
    void ClearMsrBit(uint32_t msr, uint32_t bit);
    void WriteMsr(uint32_t msr, uint64_t data);
    uint64_t ReadMsr(uint32_t msr);

public:
    UncoreUnit();
    virtual ~UncoreUnit();

    virtual void ConfigureCounter(const UncoreCounter & counter, const UncoreEvent & ev) = 0;
    virtual void UnconfigureCounter(const UncoreCounter &counter) = 0;

    virtual uint64_t GetCounterValue(const UncoreCounter & counter) = 0;
};

#endif

