
#include "UncoreUnit.h"
#include "StatContext.h"
#include "UncoreCounter.h"
#include "UncoreEvent.h"

#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/cpuctl.h>
#include <sys/pciio.h>

UncoreUnit::UncoreUnit()
{
    m_msr_fd = open("/dev/cpuctl0", O_RDWR);

    if (m_msr_fd < 0)
        throw StatException("Could not open /dev/cpuctl0");
}

UncoreUnit::~UncoreUnit()
{
    close(m_msr_fd);
}

void
UncoreUnit::SetMsrBit(uint32_t msr, uint32_t bit)
{
    cpuctl_msr_args_t args;
    int error;

    args.msr = msr;
    args.data = bit;

    error = ioctl(m_msr_fd, CPUCTL_MSRSBIT, &args);

    if (error)
        throw StatException("Error writing to MSR");
}

void
UncoreUnit::ClearMsrBit(uint32_t msr, uint32_t bit)
{
    cpuctl_msr_args_t args;
    int error;

    args.msr = msr;
    args.data = bit;

    error = ioctl(m_msr_fd, CPUCTL_MSRCBIT, &args);

    if (error)
        throw StatException("Error writing to MSR");
}

void
UncoreUnit::WriteMsr(uint32_t msr, uint64_t data)
{
    cpuctl_msr_args_t args;
    int error;

    args.msr = msr;
    args.data = data;

    error = ioctl(m_msr_fd, CPUCTL_WRMSR, &args);

    if (error)
        throw StatException("Error writing to MSR");
}

uint64_t
UncoreUnit::ReadMsr(uint32_t msr)
{
    cpuctl_msr_args_t args;
    int error;

    args.msr = msr;

    error = ioctl(m_msr_fd, CPUCTL_RDMSR, &args);

    if (error)
        throw StatException("Error writing to MSR");

    return (args.data);
}
