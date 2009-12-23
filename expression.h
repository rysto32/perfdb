#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <memory>

#include "pmccontext.h"

class BinaryExpr;
class ConstExpr;
class PmcExpr;

class ExprVisitor
{
public:
    virtual void visit(BinaryExpr & expr) = 0;
    virtual void visit(ConstExpr & expr) = 0;
    virtual void visit(PmcExpr & expr) = 0;
};

class PostOrderExprVisitor : public ExprVisitor
{
};

class Expression
{
    double m_value;
public:
    virtual void accept(PostOrderExprVisitor & v) = 0;

    virtual double getValue() const
    {
        return m_value;
    }

    virtual void setValue(double v)
    {
        m_value = v;
    }
};

class BinaryExpr : public Expression
{
public:
    enum BinaryOp { ADD, SUB, MULT, DIV };

private:
    std::auto_ptr<Expression> m_left;
    BinaryOp m_op;
    std::auto_ptr<Expression> m_right;

public:
    BinaryExpr(Expression * l, BinaryOp op, Expression * r)
      : m_left(l), m_op(op), m_right(r)
    {
    }

    virtual void accept(PostOrderExprVisitor & v)
    {
        m_left.get()->accept(v);
        m_right.get()->accept(v);
        v.visit(*this);
    }

    Expression & getLeft() const
    {
        return *m_left.get();
    }

    Expression & getRight() const
    {
        return *m_right.get();
    }

    BinaryOp getOp() const
    {
        return m_op;
    }
};

class PmcExpr : public Expression
{
    std::auto_ptr<std::string> m_pmc;

public:
    PmcExpr(std::string * pmc)
      : m_pmc(pmc)
    {
    }

    PmcExpr(const std::string & pmc)
      : m_pmc(new std::string(pmc))
    {
    }

    void accept(PostOrderExprVisitor & v)
    {
        v.visit(*this);
    }

    const std::string & getPmc() const
    {
        return *m_pmc.get();
    }
};

class ConstExpr : public Expression
{
    double m_constant;

public:
    ConstExpr(double c)
      : m_constant(c)
    {
    }

    void accept(PostOrderExprVisitor & v)
    {
        v.visit(*this);
    }

    double getValue() const
    {
        return m_constant;
    }

    void setValue()
    {
        throw std::runtime_error("setValue called on a ConstExpr");
    }
};

#endif
