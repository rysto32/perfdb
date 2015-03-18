#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <memory>
#include <stdexcept>

#include "StatContext.h"

class BinaryExpr;
class ConstExpr;
class PmcExpr;

class ExprVisitor
{
public:
	virtual void visit(BinaryExpr &expr) = 0;
	virtual void visit(ConstExpr &expr) = 0;
	virtual void visit(PmcExpr &expr) = 0;
};

class PostOrderExprVisitor : public ExprVisitor
{
};

class Expression
{
	double value;
public:
	virtual void accept(PostOrderExprVisitor &v) = 0;

	virtual double getValue() const
	{
		return (value);
	}

	virtual void setValue(double v)
	{
		value = v;
	}
};

class BinaryExpr : public Expression
{
public:
	enum BinaryOp { ADD, SUB, MULT, DIV };

private:
	std::auto_ptr<Expression> left;
	BinaryOp op;
	std::auto_ptr<Expression> right;

public:
	BinaryExpr(Expression * l, BinaryOp o, Expression * r)
		: left(l), op(o), right(r)
	{
	}

	virtual void accept(PostOrderExprVisitor &v)
	{
		left.get()->accept(v);
		right.get()->accept(v);
		v.visit(*this);
	}

	Expression & getLeft() const
	{
		return (*left.get());
	}

	Expression & getRight() const
	{
		return (*right.get());
	}

	BinaryOp getOp() const
	{
		return (op);
	}
};

class PmcExpr : public Expression
{
	std::auto_ptr<std::string> pmc;

	public:
	PmcExpr(std::string *p)
		: pmc(p)
	{
	}

	PmcExpr(const std::string &p)
		: pmc(new std::string(p))
	{
	}

	void accept(PostOrderExprVisitor &v)
	{
		v.visit(*this);
	}

	const std::string &getPmc() const
	{
		return (*pmc.get());
	}
};

class ConstExpr : public Expression
{
	double constant;

public:
	ConstExpr(double c)
		: constant(c)
	{
	}

	void accept(PostOrderExprVisitor &v)
	{
		v.visit(*this);
	}

	double getValue() const
	{
		return (constant);
	}

	void setValue()
	{
		throw std::runtime_error("setValue called on a ConstExpr");
	}
};

#endif
