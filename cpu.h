#ifndef CPU_H
#define CPU_H

#include <memory>
#include <string>
#include "page.h"
#include "pointervector.h"

class CpuDef
{
    std::auto_ptr<std::string> name;
    std::auto_ptr<PointerVector<Page> > pageList;

public:
    CpuDef(std::string * n, PointerVector<Page> * list)
      : name(n), pageList(list)
    {
    }

    const std::string & getName() const
    {
        return *name;
    }

    PointerVector<Page> & getPageList() const
    {
        return *pageList;
    }
};

#endif
