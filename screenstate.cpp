
#include <curses.h>

#include "screenstate.h"

class ValidatePmcVisitor : public PostOrderExprVisitor
{
	PmcContext &m_pmc;

	public:
	ValidatePmcVisitor(PmcContext &pmc)
	    : m_pmc(pmc)
	{
	}

	virtual void visit(BinaryExpr &expr)
	{
	}

	virtual void visit(PmcExpr &expr)
	{
		m_pmc.loadPmc(expr.getPmc());
		m_pmc.clearPmcs();
	}

	virtual void visit(ConstExpr &expr)
	{
	}
};

class InitPmcVisitor : public PostOrderExprVisitor
{
	PmcContext &m_pmc;

	public:
	InitPmcVisitor(PmcContext &pmc)
	    : m_pmc(pmc)
	{
	}

	virtual void visit(BinaryExpr &expr)
	{
	}

	virtual void visit(PmcExpr &expr)
	{
		try {
			m_pmc.loadPmc(expr.getPmc());
		} catch (PmcException &e) {
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
    PmcContext &pmc, int rate)
	: pageList(pages), pageIndex(0), statIndex(0), missedStatIndex(0), 
	  forceUpdate(true), lastUpdate(0), updateRate(rate), perCpu(pcpu)
{
	SetupShortcuts(pmc);
}

void 
ScreenState::SetupShortcuts(PmcContext &pmc)
{
	PointerVector<Page>::iterator it;
	Page *page;
	int index;

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
		
			throw PmcException(msg);
		}

		char c = shortcut[0];

		if (c == 'q' || c == 'C' || isdigit(c)) {
			std::string msg("Page ");
			msg += page->getName();
			msg += "using reserved key '";
			msg += c;
			msg += "'";
			throw PmcException(msg);
		}

		std::pair<std::map<char, int>::iterator, bool> inserted =
		    pageMap.insert(std::map<char, int>::value_type(c, index));

		if (!inserted.second) {
			std::string msg("Page ");
			msg += page->getName();
			msg += "defined to use shortcut ";
			msg += c;
			msg += ", but that is already used by";
			msg += pageList[inserted.first->second]->getName();
			throw PmcException(msg);
		}

		if (index < 10)
			pageMap['1' + index] = index;
		else if (index == 10)
			pageMap['0'] = index;

		ValidatePmcVisitor validate(pmc);
		PointerVector<Statistic> &stats = page->getStats();
		PointerVector<Statistic>::iterator jt;

		for (jt = stats.begin(); jt != stats.end(); ++jt)
			/* Will throw exception if there is an invalid pmc. */
			(*jt)->getExpr().accept(validate);
	}
}

void
ScreenState::WaitForKeypress(PmcContext &pmc)
{
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
	
	switch (ch) {
	case 'q':
		quit = true;
		break;
	case 'C':
		perCpu = !perCpu;
		erase();
		forceUpdate = true;
		break;
	case '>':
	case KEY_RIGHT:
		if (pageIndex != pageList.size() - 1) {
			pageIndex++;
			statIndex = 0;
			missedStatIndex = 0;
			LoadPage(pmc);
			erase();
			forceUpdate = true;
		}
		break;
	case '<':
	case KEY_LEFT:
		if (pageIndex != 0) {
			pageIndex--;
			statIndex = 0;
			missedStatIndex = 0;
			LoadPage(pmc);
			erase();
			forceUpdate = true;
		}
		break;
	default:
		{
			PageMap::iterator indexIt = pageMap.find(ch);
			if (indexIt != pageMap.end()) {
				pageIndex = indexIt->second;
				statIndex = 0;
				missedStatIndex = 0;
				LoadPage(pmc);
				erase();
				forceUpdate = true;
			}
			break;
		}
	}
}

void
ScreenState::LoadPage(PmcContext &pmc)
{
	PointerVector<Statistic> &stats = ScreenStats();
	PointerVector<Statistic>::iterator it;
	size_t startIndex;
	size_t index;
	
	pmc.clearPmcs();
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
