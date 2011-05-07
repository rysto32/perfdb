#ifndef SCREENSTATE_H
#define SCREENSTATE_H

#include <map>

#include "pointervector.h"
#include "page.h"

typedef std::map<char, int> PageMap;

/* This is global so signal handlers can modify it. */
extern bool quit;

class ScreenState {
private:
	PointerVector<Page> &pageList;
	PageMap pageMap;
	
	unsigned pageIndex;
	unsigned statIndex;
	unsigned missedStatIndex;

	bool forceUpdate;
	time_t lastUpdate;
	int updateRate;
	
	bool perCpu;
	
	void SetupShortcuts(PmcContext &pmc);
public:
	ScreenState(PointerVector<Page> &pages, bool pcpu, PmcContext &pmc, 
	    int updateRate);

	void LoadPage(PmcContext &pmc);
	void WaitForKeypress(PmcContext &pmc);
	void MissedStat(int index);
	void CompleteUpdate(void);
	bool UpdateScreen() const;
	
	const char *ScreenName() const;
	const char *ScreenShortcut() const;
	PointerVector<Statistic> & ScreenStats();
	
	bool PerCPU() const
	{
		return perCpu;
	}
};

#endif
