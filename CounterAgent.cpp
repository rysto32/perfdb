
#include "CounterAgent.h"

CounterAgent CombineAgents(CounterAgent a, CounterAgent b)
{

    /*
     * If you combine an ANY_AGENT stat with any other stat, the resulting 
     * stat will be counted againt the agent of the other stat.
     */
    if (a == ANY_AGENT)
        return (b);
    if (b == ANY_AGENT)
        return (a);

    /* If the two stats are broken down by the same agent, then the combined
     * stat can also be broken down by that same agent.
     */
    if (a == b)
        return (a);

    /* Otherwise the two agents are incompatible and we cannot break down the
     * combined stat by any agent.
     */
    return (NO_AGENT);
}
