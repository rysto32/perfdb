#ifndef SCREENSTATE_H
#define SCREENSTATE_H

#include <map>

#include "pointervector.h"
#include "page.h"

/* This is global so signal handlers can modify it. */
extern bool quit;

class KeyAction;

class ScreenState {
private:
	typedef std::map<int, KeyAction*> KeyMap;
	PointerVector<Page> &pageList;
	KeyMap keyMap;
	
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
	~ScreenState();

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
	
	void TogglePerCpu();
	void ChangePage(PmcContext &pmc, int newIndex);
	void IncrementPage(PmcContext &pmc, int increment);
};

#endif
