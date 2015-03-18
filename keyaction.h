#ifndef KEYACTION_H
#define KEYACTION_H

#include "screenstate.h"

class KeyAction {
public:
	virtual ~KeyAction() {}
	virtual const char *Name() = 0;
	virtual void Perform(PmcContext &pmc, ScreenState &state) = 0;
};

class QuitAction : public KeyAction {
public:
	virtual const char *Name();
	virtual void Perform(PmcContext &pmc, ScreenState &state);
};

class PerCpuAction : public KeyAction {
public:
	virtual const char *Name();
	virtual void Perform(PmcContext &pmc, ScreenState &state);
};

class ChoosePageAction : public KeyAction {
private:
	int pageIndex;
	std::string actionName;

public:
	ChoosePageAction(int index, const std::string &pageName);
	virtual const char *Name();
	virtual void Perform(PmcContext &pmc, ScreenState &state);
};

class IncrementPageAction : public KeyAction {
private:
	int increment;
	
public:
	IncrementPageAction(int incr);
	virtual const char *Name();
	virtual void Perform(PmcContext &pmc, ScreenState &state);
};

#endif
