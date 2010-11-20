#ifndef PMCCONTEXT_H
#define PMCCONTEXT_H

#include <map>
#include <string>
#include <stdexcept>

#include <pmc.h>

class PmcException : public std::runtime_error
{
public:
    PmcException(const std::string & msg)
      : std::runtime_error(msg)
    {
    }
};

class PmcNotLoaded : public std::exception
{
    const char * what() const throw ()
    {
        return "pmc not loaded";
    }
};

class PmcContext
{
    struct Pmc
    {
        pmc_id_t m_id;
        pmc_value_t m_lastAbsolute;
        pmc_value_t m_value;

        Pmc(pmc_id_t id)
          : m_id(id), m_lastAbsolute(0)
        {
        };
    };

    typedef std::map<int, Pmc> PmcCpuMap;
    typedef std::map<std::string, PmcCpuMap> PmcMap;

    PmcMap m_pmcs;
    int m_cpuMask;

    static std::string getPmcInitErrorMessage(int error);
    static std::string getPmcErrorMessage(const std::string & event, const char * function, int error);

    void clearCpuMap(PmcCpuMap & map);

    void readPmc(const PmcMap::iterator & it);
public:
    PmcContext();

    ~PmcContext()
    {
        clearPmcs();
    }

    void setCpuMask(int cpuMask)
    {
        m_cpuMask = cpuMask;
    }

    void loadPmc(const std::string & name);
    void clearPmcs();

    pmc_value_t getPmc(const std::string & name) throw (PmcNotLoaded);
	pmc_value_t getPmcCpu(const std::string & name, int cpu) throw (PmcNotLoaded);
    void readPmcs();
};

#endif
