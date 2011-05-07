#ifndef PAGE_H
#define PAGE_H

#include "statistic.h"
#include "pointervector.h"

class Page
{
private:
	std::auto_ptr<std::string> name;
	std::auto_ptr<std::string> shortcut;
	std::auto_ptr<PointerVector<Statistic> > stats;

public:
	Page(std::string *n, std::string *cut, PointerVector<Statistic> * stat)
		: name(n), shortcut(cut), stats(stat)
	{
	}

	const std::string &getName() const
	{
		return *name.get();
	}

	const std::string &getShortcut() const
	{
		return *shortcut.get();
	}

	PointerVector<Statistic> &getStats() const
	{
		return *stats.get();
	}
};

#endif
