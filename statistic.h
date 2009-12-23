#ifndef STATISTIC_H
#define STATISTIC_H

#include <memory>
#include <math.h>

#include "expression.h"

class Statistic
{
    std::auto_ptr<std::string> m_name;
    std::auto_ptr<Expression> m_expr;

    double m_goodThreshold, m_okThreshold, m_badThreshold;

    /* true if a larger value is better */
    bool ascending() const
    {
        return m_goodThreshold > m_okThreshold;
    }

public:
    enum Status { STAT_GOOD = 1, STAT_OK, STAT_BAD, STAT_TERRIBLE };

    Statistic(std::string * name, Expression * e, double good, double ok, double bad)
      : m_name(name),
        m_expr(e),
        m_goodThreshold(good),
        m_okThreshold(ok),
        m_badThreshold(bad)
    {
    }

    Statistic(const std::string & name, Expression * e, double good, double ok, double bad)
      : m_name(new std::string(name)),
        m_expr(e),
        m_goodThreshold(good),
        m_okThreshold(ok),
        m_badThreshold(bad)
    {
    }

    const std::string & getName() const
    {
        return *m_name.get();
    }

    Expression & getExpr() const
    {
        return *m_expr.get();
    }

    Status getStatus(double value) const
    {
        if(ascending())
        {
            if(isgreater(value, m_goodThreshold))
                return STAT_GOOD;
            else if(isgreater(value, m_okThreshold))
                return STAT_OK;
            else if(isgreater(value, m_badThreshold))
                return STAT_BAD;
            else
                return STAT_TERRIBLE;
        }
        else
        {
            if(isless(value, m_goodThreshold))
                return STAT_GOOD;
            else if(isless(value, m_okThreshold))
                return STAT_OK;
            else if(isless(value, m_badThreshold))
                return STAT_BAD;
            else
                return STAT_TERRIBLE;
        }
    }
};

#endif
