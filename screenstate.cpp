
#include <curses.h>

#include "screenstate.h"
#include "keyaction.h"

class ValidatePmcVisitor : public PostOrderExprVisitor
{
	StatContext &m_pmc;

	public:
	ValidatePmcVisitor(StatContext &pmc)
	    : m_pmc(pmc)
	{
	}

	virtual void visit(BinaryExpr &expr)
	{
	}

	virtual void visit(PmcExpr &expr)
	{
		m_pmc.loadStat(expr.getPmc());
		m_pmc.clearStats();
	}

	virtual void visit(ConstExpr &expr)
	{
	}
};

class InitPmcVisitor : public PostOrderExprVisitor
{
	StatContext &m_pmc;

	public:
	InitPmcVisitor(StatContext &pmc)
	    : m_pmc(pmc)
	{
	}

	virtual void visit(BinaryExpr &expr)
	{
	}

	virtual void visit(PmcExpr &expr)
	{
		try {
			m_pmc.loadStat(expr.getPmc());
		} catch (StatException &e) {
			/* 
			 * This is probably because we ran out of PMCs, so 
			 * ignore it.
			 */
		}
	}

	virtual void visit(ConstExpr & expr)
	{
	}
};

ScreenState::ScreenState(PointerVector<Page> &pages, bool pcpu, 
    StatContext &pmc, int rate)
	: pageList(pages), pageIndex(0), statIndex(0), missedStatIndex(0), 
	  forceUpdate(true), lastUpdate(0), updateRate(rate), perCpu(pcpu)
{
	SetupShortcuts(pmc);
}

ScreenState::~ScreenState()
{
	KeyMap::iterator it;
	
	for (it = keyMap.begin(); it != keyMap.end(); ++it)
		delete it->second;
}

void 
ScreenState::SetupShortcuts(StatContext &pmc)
{
	PointerVector<Page>::iterator it;
	Page *page;
	ChoosePageAction *choosePage;
	int index;
	
	keyMap['q'] = new QuitAction;
	keyMap['C'] = new PerCpuAction;
	keyMap[KEY_LEFT] = new IncrementPageAction(-1);
	keyMap[KEY_RIGHT] = new IncrementPageAction(1);

	index = 0;
	for (it = pageList.begin(); it != pageList.end(); ++it, ++index) {
		page = *it;
		const std::string &shortcut = page->getShortcut();

		if (shortcut.size() != 1) {
			std::string msg("Page ");
			msg += page->getName();
			msg += " has shortcut with more than one character(";
			msg += shortcut.c_str();
			msg += ")";
		
			throw StatException(msg);
		}

		char c = shortcut[0];
		choosePage = new ChoosePageAction(index, page->getName());

		std::pair<KeyMap::iterator, bool> inserted =
		    keyMap.insert(KeyMap::value_type(c, choosePage));

		if (!inserted.second) {
			delete choosePage;

			std::string msg("Page ");
			msg += page->getName();
			msg += " defined to use shortcut key ";
			msg += c;
			msg += ", but that is already used by ";
			msg += inserted.first->second->Name();
			throw StatException(msg);
		}

		ValidatePmcVisitor validate(pmc);
		PointerVector<Statistic> &stats = page->getStats();
		PointerVector<Statistic>::iterator jt;

		for (jt = stats.begin(); jt != stats.end(); ++jt)
			/* Will throw exception if there is an invalid pmc. */
			(*jt)->getExpr().accept(validate);
	}
}

void
ScreenState::WaitForKeypress(StatContext &pmc)
{
	KeyAction *action;
	int ch;

	if (missedStatIndex != statIndex) {
		statIndex = missedStatIndex;
		LoadPage(pmc);
	}
	missedStatIndex = 0;

	/* 
	 * Wait for the next keypress.  If we time out, this will return an 
	 * error.  In that case, it's time to update the screen again.
	 */
	ch = getch();
	if (ch == ERR) {
		forceUpdate = true;
		return;
	}
	
	action = keyMap[ch];
	
	if (action != NULL)
		action->Perform(pmc, *this);
}

void 
ScreenState::TogglePerCpu()
{
	perCpu = !perCpu;
	erase();
	forceUpdate = true;
}

void 
ScreenState::ChangePage(StatContext &pmc, int newIndex)
{
	pageIndex = newIndex;
	statIndex = 0;
	missedStatIndex = 0;
	LoadPage(pmc);
	erase();
	forceUpdate = true;
}

void 
ScreenState::IncrementPage(StatContext &pmc, int increment)
{
	int newPage;
	
	newPage = pageIndex + increment;
	
	if (newPage >= 0 && newPage < pageList.size())
		ChangePage(pmc, newPage);
}

void
ScreenState::LoadPage(StatContext &pmc)
{
	PointerVector<Statistic> &stats = ScreenStats();
	PointerVector<Statistic>::iterator it;
	size_t startIndex;
	size_t index;
	
	pmc.clearStats();
	index = 0;
	for (it = stats.begin(); it != stats.end(); ++it, ++index) {
		if (index >= statIndex) {
			InitPmcVisitor initPmc(pmc);
			(*it)->getExpr().accept(initPmc);
		}
	}
}

void
ScreenState::MissedStat(int curIndex)
{
	if (missedStatIndex == 0 && curIndex > statIndex)
		missedStatIndex = curIndex;
}

bool
ScreenState::UpdateScreen() const
{
	return (forceUpdate || ((time(NULL) - lastUpdate) > updateRate));
}

void
ScreenState::CompleteUpdate()
{
	lastUpdate = time(NULL);
}

const char *
ScreenState::ScreenName() const
{
	return (pageList[pageIndex]->getName().c_str());
}

const char *
ScreenState::ScreenShortcut() const
{
	return (pageList[pageIndex]->getShortcut().c_str());
}

PointerVector<Statistic> &
ScreenState::ScreenStats()
{
	return (pageList[pageIndex]->getStats());
}
