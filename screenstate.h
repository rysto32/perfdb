#ifndef SCREENSTATE_H
#define SCREENSTATE_H

#include <map>

#include "pointervector.h"
#include "page.h"

/* This is global so signal handlers can modify it. */
extern bool quit;

class KeyAction;
class StatContext;

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
	
	void SetupShortcuts(StatContext &pmc);
public:
	ScreenState(PointerVector<Page> &pages, bool pcpu, StatContext &pmc, 
	    int updateRate);
	~ScreenState();

	void LoadPage(StatContext &pmc);
	void WaitForKeypress(StatContext &pmc);
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
	void ChangePage(StatContext &pmc, int newIndex);
	void IncrementPage(StatContext &pmc, int increment);
};

#endif
