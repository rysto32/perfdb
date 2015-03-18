

#ifndef COUNTER_AGENT_H
#define COUNTER_AGENT_H

enum CounterAgent
{
    CPU_CORE_AGENT,     /* Stat is counted per-CPU core. */
    IMC_AGENT,          /* Stat is counted per-memory channel. */
    R2PCIE_AGENT,       /* Stat is counted per-R2PCIe agent. */
    CBOX_AGENT,         /* Stat is counted per-CBox agent. */
    
    ANY_AGENT,          /* Stat is not associated with any agent. */
    NO_AGENT            /* Stat cannot be broken out per-agent. */
};

/*
 * Figure out what agent we would count a stat against if two stats counted by
 * the specified agents were combined in an operation (e.g. added together).
 */
CounterAgent CombineAgents(CounterAgent a, CounterAgent b);

#endif
