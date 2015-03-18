
#include "DetermineAgentVisitor.h"

DetermineAgentVisitor::DetermineAgentVisitor(StatContext & context)
  : pmc(context)
{
}

void DetermineAgentVisitor::visit(BinaryExpr &expr)
{
    CounterAgent left = agentMap.at(&expr.getLeft());
    CounterAgent right = agentMap.at(&expr.getRight());

    agentMap[&expr] = CombineAgents(left, right);
}

void DetermineAgentVisitor::visit(ConstExpr &expr)
{

    agentMap[&expr] = ANY_AGENT;
}

void DetermineAgentVisitor::visit(PmcExpr &expr)
{

    agentMap[&expr] = pmc.getAgent(expr.getPmc());
}
    
CounterAgent DetermineAgentVisitor::GetAgent(Expression & expr)
{

    return agentMap.at(&expr);
}
