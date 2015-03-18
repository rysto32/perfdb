
#include "UncoreEvent.h"

UncoreEvent::UncoreEvent(const std::string &name, uint32_t counter, uint32_t code, uint32_t umask)
    : m_name(name), m_counters(counter), m_code(code), m_umask(umask)
{
}

const std::string & UncoreEvent::getName() const
{
    return (m_name);
}

uint32_t UncoreEvent::getCounters() const
{
    return (m_counters);
}

uint32_t UncoreEvent::GetCode() const
{
    return (m_code);
}

uint32_t UncoreEvent::GetUmask() const
{
    return (m_umask);
}


