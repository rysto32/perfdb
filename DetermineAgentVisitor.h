
#ifndef DETERMINE_AGENT_VISITOR
#define DETERMINE_AGENT_VISITOR

#include "CounterAgent.h"
#include "expression.h"
#include "StatContext.h"

#include <map>

class DetermineAgentVisitor : public PostOrderExprVisitor
{
    typedef std::map<const Expression *, CounterAgent> ExprAgentMap;
	StatContext & pmc;
    ExprAgentMap agentMap;

public:
    DetermineAgentVisitor(StatContext & pmc);

	virtual void visit(BinaryExpr &expr);
	virtual void visit(ConstExpr &expr);
	virtual void visit(PmcExpr &expr);

    CounterAgent GetAgent(Expression & expr);
};

#endif
