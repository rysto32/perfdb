
#ifndef UNCORE_EVENT_H
#define UNCORE_EVENT_H

#include "UncoreCounter.h"

#include <string>

class UncoreEvent
{
private:
	const std::string m_name;
	const uint32_t m_counters; /* The counter types that this event is compatible with. */
	const uint32_t m_code;
	const uint32_t m_umask;

public:
	UncoreEvent(const std::string &name, uint32_t counter, uint32_t code, uint32_t umask);

	const std::string & getName() const;
	uint32_t getCounters() const;

    uint32_t GetCode() const;
    uint32_t GetUmask() const;
};

#endif

