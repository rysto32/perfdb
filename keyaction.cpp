#include "keyaction.h"


const char *
QuitAction::Name()
{
	return "quit action";
}

void
QuitAction::Perform(PmcContext &pmc, ScreenState &state)
{
	quit = true;
}

const char *
PerCpuAction::Name()
{
	return "toggle per-CPU action";
}

void
PerCpuAction::Perform(PmcContext &pmc, ScreenState &state)
{
	state.TogglePerCpu();
}

ChoosePageAction::ChoosePageAction(int index, const std::string &pageName)
	: pageIndex(index)
{
	actionName = pageName + " page";
}

const char *
ChoosePageAction::Name()
{
	return actionName.c_str();
}

void
ChoosePageAction::Perform(PmcContext &pmc, ScreenState &state)
{
	state.ChangePage(pmc, pageIndex);
}

IncrementPageAction::IncrementPageAction(int incr)
	: increment(incr)
{
}

const char *
IncrementPageAction::Name()
{
	return "increment page action";
}

void
IncrementPageAction::Perform(PmcContext &pmc, ScreenState &state)
{
	state.IncrementPage(pmc, increment);
}
