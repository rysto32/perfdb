#ifndef PAGE_H
#define PAGE_H

#include "statistic.h"
#include "pointervector.h"

class Page
{
    std::auto_ptr<std::string> m_name;
    std::auto_ptr<std::string> m_shortcut;
    std::auto_ptr<PointerVector<Statistic> > m_stats;

public:
    Page(std::string * name, std::string * shortcut, PointerVector<Statistic> * stats)
      : m_name(name), m_shortcut(shortcut), m_stats(stats)
    {
    }

    const std::string & getName() const
    {
        return *m_name.get();
    }

    const std::string & getShortcut() const
    {
        return *m_shortcut.get();
    }

    PointerVector<Statistic> & getStats() const
    {
        return *m_stats.get();
    }
};

#endif
