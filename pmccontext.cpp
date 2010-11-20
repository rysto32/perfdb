#include "pmccontext.h"
#include <strings.h>
#include <exception>

#include <errno.h>

std::string
PmcContext:: getPmcInitErrorMessage(int error)
{
    std::string message("error in pmc_init");
    switch(error) {
        case ENOENT:
            message += ": hwpmc module not loaded?";
            break;
        case EPROGMISMATCH:
            message += ": libpmc version number does not match hwpmc version number";
            break;
        case ENXIO:
            message += ": pmc hardware not supported";
            break;
    }
    message += "(";
    message += strerror(errno);
    message += ")";

    return message;
}

std::string
PmcContext::getPmcErrorMessage(const std::string & event, const char * function, int error)
{
    std::string message("error in ");
    message += function;
    message += " of ";
    message += event;
    message += ": ";
    message += strerror(errno);

    return message;
}

PmcContext::PmcContext()
  : m_cpuMask(~0)
{
    int error = pmc_init();

    if(error)
    {
        throw PmcException(getPmcInitErrorMessage(errno));
    }
}

void
PmcContext::loadPmc(const std::string & name)
{
    PmcMap::iterator it = m_pmcs.lower_bound(name);

    if(it == m_pmcs.end() || (m_pmcs.key_comp()(name, it->first) != 0)) {
        pmc_id_t pmcId;
        int error;
        int cpuMask = m_cpuMask;
        it = m_pmcs.insert(it, PmcMap::value_type(name, PmcCpuMap()));

        while(cpuMask) {
            int cpu = ffs(cpuMask) - 1;

            error = pmc_allocate(name.c_str(), PMC_MODE_SC, 0, cpu, &pmcId);

            if(error) {
                clearCpuMap(it->second);
                m_pmcs.erase(it);
                throw PmcException(getPmcErrorMessage(name, "pmc_allocate", errno));
            }

            error = pmc_start(pmcId);

            if(error) {
                clearCpuMap(it->second);
                m_pmcs.erase(it);
                throw PmcException(getPmcErrorMessage(name, "pmc_start", errno));
            }

            it->second.insert(PmcCpuMap::value_type(cpu, Pmc(pmcId)));

            cpuMask &= ~(1 << cpu);
        }

        /* 
         * Some pmcs start with a non-zero value(e.g. the tsc).  Most will start
         * at zero. Read the value now so we get a consistent delta when we go
         * to get the pmc value for the first time
         */
	readPmc(it);
    }
}

void PmcContext::clearCpuMap(PmcCpuMap & map)
{
    PmcCpuMap::iterator it;
    for(it = map.begin(); it != map.end(); ++it)
    {
        pmc_release(it->second.m_id);
    }
    map.clear();
}

void
PmcContext::clearPmcs()
{
    PmcMap::iterator it;
    for(it = m_pmcs.begin(); it != m_pmcs.end(); ++it)
    {
        clearCpuMap(it->second);
    }
    m_pmcs.clear();
}

void 
PmcContext::readPmc(const PmcMap::iterator & it)
{
    PmcCpuMap::iterator jt;
    for(jt = it->second.begin(); jt != it->second.end(); ++jt)
    {
        pmc_value_t v;
        pmc_read(jt->second.m_id, &v);

        jt->second.m_value = v - jt->second.m_lastAbsolute;
        jt->second.m_lastAbsolute = v;
    }
}

void
PmcContext::readPmcs()
{
    PmcMap::iterator it;
    for(it = m_pmcs.begin(); it != m_pmcs.end(); ++it)
    {
        readPmc(it);
    }
}


pmc_value_t
PmcContext::getPmc(const std::string & name) throw (PmcNotLoaded)
{
    pmc_value_t total = 0;

    PmcMap::iterator it = m_pmcs.find(name);

    if(it == m_pmcs.end())
        throw PmcNotLoaded();

    PmcCpuMap::iterator jt;
    for(jt = it->second.begin(); jt != it->second.end(); ++jt)
    {
        total += jt->second.m_value;
    }

    return total;
}


pmc_value_t
PmcContext::getPmcCpu(const std::string & name, int cpu) throw (PmcNotLoaded)
{
    PmcMap::iterator it = m_pmcs.find(name);

    if(it == m_pmcs.end())
        throw PmcNotLoaded();

    PmcCpuMap::iterator jt = it->second.find(cpu);

    if(jt == it->second.end())
        throw std::runtime_error("unknown cpu");

    return jt->second.m_value;
}

