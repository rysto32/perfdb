

#ifndef UNCORE_COUNTER_H
#define UNCORE_COUNTER_H

#include <stdint.h>

#define UNCORE_IMC_FIXED	(1 << 0)
#define UNCORE_IMC		(1 << 1)
#define UNCORE_R2PCIE	(1 << 2)
#define UNCORE_CBOX0	(1 << 3)
#define UNCORE_CBOXN	(1 << 4)

#define UNCORE_CBOX_ANY (UNCORE_CBOX0 | UNCORE_CBOXN)

class UncoreEvent;

class UncoreCounter
{
private:
	uint32_t agent_type;

	uint32_t ctl_offset;
	uint32_t ctr_offset;

	bool free;

public:
	UncoreCounter(uint32_t type, uint32_t ctl, uint32_t ctr);

	bool AllocateFor(UncoreEvent & event);
	void Release();

    uint32_t GetAgentType() const;
    uint32_t GetCtlOffset() const;
    uint32_t GetCtrOffset() const;
};

#endif

