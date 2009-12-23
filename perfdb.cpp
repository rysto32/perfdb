#include <sys/types.h>
#include <sys/sysctl.h>
#include <pmc.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <libgen.h>

#include <curses.h>

#include "expression.h"
#include "pmccontext.h"
#include "statistic.h"
#include "pointervector.h"
#include "page.h"
#include "cpu.h"
#include "parser.cpp.h"

static int quit = 0;

#define HEADER_WIDTH 20
#define STAT_WIDTH 6

void quit_signal_handler(int sig)
{
    quit = 1;
}

class Curses
{
    short m_backgroundColour, m_foregroundColour;
public:
    Curses(int rate)
    {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        halfdelay(rate * 10);

        if(has_colors())
        {
            start_color();
            pair_content(0, &m_foregroundColour, &m_backgroundColour);

            init_pair(Statistic::STAT_GOOD, COLOR_GREEN, m_backgroundColour);
            init_pair(Statistic::STAT_OK, COLOR_MAGENTA, m_backgroundColour);
            init_pair(Statistic::STAT_BAD, COLOR_YELLOW, m_backgroundColour);
            init_pair(Statistic::STAT_TERRIBLE, COLOR_RED, m_backgroundColour);
        }
    }

    void startColor(Statistic::Status status)
    {
        if(has_colors())
            attron(COLOR_PAIR(status));
    }

    void endColor(Statistic::Status status)
    {
        if(has_colors())
            attroff(COLOR_PAIR(status));
    }

    ~Curses()
    {
        endwin();
    }
};

class ValidatePmcVisitor : public PostOrderExprVisitor
{
    PmcContext & m_pmc;

public:
    ValidatePmcVisitor(PmcContext & pmc)
      : m_pmc(pmc)
    {
    }

    virtual void visit(BinaryExpr & expr)
    {
    }

    virtual void visit(PmcExpr & expr)
    {
        m_pmc.loadPmc(expr.getPmc());
        m_pmc.clearPmcs();
    }

    virtual void visit(ConstExpr & expr)
    {
    }
};

class InitPmcVisitor : public PostOrderExprVisitor
{
    PmcContext & m_pmc;

public:
    InitPmcVisitor(PmcContext & pmc)
      : m_pmc(pmc)
    {
    }

    virtual void visit(BinaryExpr & expr)
    {
    }

    virtual void visit(PmcExpr & expr)
    {
        try
        {
            m_pmc.loadPmc(expr.getPmc());
        }
        catch(PmcException & e)
        {
            //assume that this is because we ran out of PMCs, so ignore it
        }
    }

    virtual void visit(ConstExpr & expr)
    {
    }
};

class ExprEvaluator : public PostOrderExprVisitor
{
    PmcContext & m_pmc;

public:
    ExprEvaluator(PmcContext & pmc)
      : m_pmc(pmc)
    {
    }

    virtual void visit(BinaryExpr & expr)
    {
        double left = expr.getLeft().getValue();
        double right = expr.getRight().getValue();
        double value;

        switch(expr.getOp())
        {
            case BinaryExpr::ADD:
                value = left + right;
                break;
            case BinaryExpr::SUB:
                value = left - right;
                break;
            case BinaryExpr::MULT:
                value = left * right;
                break;
            case BinaryExpr::DIV:
                value = left / right;
                break;
            default:
                abort();
        }

        expr.setValue(value);
    }

    virtual void visit(PmcExpr & expr)
    {
        expr.setValue(m_pmc.getPmc(expr.getPmc()));
    }

    virtual void visit(ConstExpr & expr)
    {
    }
};

class PerCpuExprEvaluator : public PostOrderExprVisitor
{
    PmcContext & m_pmc;
    int m_cpu;

public:
    PerCpuExprEvaluator(PmcContext & pmc, int cpu)
      : m_pmc(pmc), m_cpu(cpu)
    {
    }

    virtual void visit(BinaryExpr & expr)
    {
        double left = expr.getLeft().getValue();
        double right = expr.getRight().getValue();
        double value;

        switch(expr.getOp())
        {
            case BinaryExpr::ADD:
                value = left + right;
                break;
            case BinaryExpr::SUB:
                value = left - right;
                break;
            case BinaryExpr::MULT:
                value = left * right;
                break;
            case BinaryExpr::DIV:
                value = left / right;
                break;
            default:
                abort();
        }

        expr.setValue(value);
    }

    virtual void visit(PmcExpr & expr)
    {
        expr.setValue(m_pmc.getPmcCpu(expr.getPmc(), m_cpu));
    }

    virtual void visit(ConstExpr & expr)
    {
        /* nothing to do because expr.getValue() will already
         * return the right value for a ConstExpr
         */
    }
};

extern FILE * yyin;
int yyparse();
extern int yydebug;
extern YYSTYPE yyval;

void loadPage(PmcContext & pmc, Page * page, size_t startIndex)
{
    pmc.clearPmcs();
    PointerVector<Statistic>::iterator it;
    erase();
    size_t index = 0;
    for(it = page->getStats().begin(); it != page->getStats().end(); ++it, ++index)
    {
        if(index >= startIndex)
        {
            InitPmcVisitor initPmc(pmc);
            (*it)->getExpr().accept(initPmc);
        }
    }
}

void
usage(char * exe)
{
    fprintf(stderr, "usage: %s [-C -c cpuspec -w rate -f stats_file ]\n", exe);
}

int
parseCpuMask(char * optarg, int ncpu)
{
    int cpu;
    int cpuMask = 0;
    char * endp = optarg;

    while(*endp != '\0') {
        cpu = strtol(optarg, &endp, 10);

        if(*endp != ',' && *endp != '\0')
        {
            throw PmcException("Invalid character in cpuspec passed to -c option");
        }

        if(cpu < 0 || cpu >= ncpu)
        {
            throw new PmcException("Invalid cpu number in cpuspec passed to -c option");
        }

        cpuMask |= (1 << cpu);

        optarg = endp + 1;
    }

    return cpuMask;
}

PointerVector<Page> &
getPageList(PointerVector<CpuDef> & cpuDefs)
{
    PointerVector<CpuDef>::iterator it;
    const struct pmc_cpuinfo * cpuInfo;
    int error;

    error = pmc_cpuinfo(&cpuInfo);
    if(error)
    {
        std::string msg("error in pmc_cpuinfo: ");
        msg += strerror(errno);
        throw PmcException(msg);
    }

    const char * cpuName = pmc_name_of_cputype(cpuInfo->pm_cputype);

    for(it = cpuDefs.begin(); it != cpuDefs.end(); ++it)
    {
        if((*it)->getName() == cpuName)
        {
            return (*it)->getPageList();
        }
    }

    std::string msg("No definitions for CPU type ");
    msg += cpuName;
    throw PmcException(msg);
}

int main(int argc, char ** argv)
{
    int rate = 1;
    int error;
    size_t size;
    int ncpu;
    bool perCpu = false;
    int option;
    char * endp;
    int cpuMask;
    std::string statsFile(dirname(argv[0]));
    statsFile += "/stats.txt";

    size = sizeof(ncpu);
    if (sysctlbyname("hw.ncpu", &ncpu, &size, NULL, 0) < 0)
    {
        perror("Could not read sysctl hw.ncpu");
        return -1;
    }

    cpuMask = (1 << ncpu) - 1;

    try
    {
        while((option = getopt(argc, argv, "Cc:w:hf:")) != -1)
        {
            switch(option)
            {
                case 'C':
                    perCpu = true;
                    break;
                case 'c':
                    cpuMask = parseCpuMask(optarg, ncpu);
                    break;
                case 'w':
                    rate = strtol(optarg, &endp, 10);
                    if(*endp != '\0')
                    {
                        fprintf(stderr, "-w option requires numeric argument\n");
                        return -1;
                    }
                    break;
                case 'f':
                    statsFile = optarg;
                    break;
                case 'h':
                default:
                    usage(argv[0]);
                    return -1;
            }
        }

        PmcContext pmc(cpuMask);
        yyin = fopen(statsFile.c_str(), "r");

        if(!yyin)
        {
            fprintf(stderr, "Could not read %s\n", statsFile.c_str());
            return -1;
        }

        error = yyparse();
        if(error)
        {
            fprintf(stderr, "Could not parse %s\n", statsFile.c_str());
            return -1;
        }

        std::auto_ptr<PointerVector<CpuDef> > cpuList(yyval.cpuList);

        PointerVector<Page> & pageList = getPageList(*cpuList);
        std::map<char, int> pageMap;

        int index = 0;
        for(PointerVector<Page>::iterator it = pageList.begin(); it != pageList.end(); ++it, ++index)
        {
            Page * page = *it;
            const std::string & shortcut = page->getShortcut();

            if(shortcut.size() != 1)
            {
                fprintf(stderr, "Page %s has shortcut with more than one character(%s)\n",
                        page->getName().c_str(), shortcut.c_str());
                return -1;
            }

            char c = shortcut[0];

            if(c == 'q' || c == 'C')
            {
                fprintf(stderr, "Page %s using reserved key %c\n",
                        page->getName().c_str(), c);
            }

            if(isdigit(c))
            {
                fprintf(stderr, "Page %s using reserved key '%c'\n",
                        page->getName().c_str(), c);
                return -1;
            }

            std::pair<std::map<char, int>::iterator, bool> inserted =
                pageMap.insert(std::map<char, int>::value_type(c, index));

            if(!inserted.second)
            {
                fprintf(stderr, "Page %s defined to use shortcut %c, but that is already used by %s\n",
                       page->getName().c_str(), c, pageList[inserted.first->second]->getName().c_str());
                return -1;
            }

            if(index < 10)
            {
                pageMap['1' + index] = index;
            }
            else if(index == 10)
            {
                pageMap['0'] = index;
            }

            ValidatePmcVisitor validate(pmc);
            PointerVector<Statistic> & stats = page->getStats();
            PointerVector<Statistic>::iterator jt;

            for(jt = stats.begin(); jt != stats.end(); ++jt)
            {
                /* will throw exception if there is an invalid pmc */
                (*jt)->getExpr().accept(validate);
            }
        }

        signal(SIGBUS, quit_signal_handler);
        signal(SIGTERM, quit_signal_handler);

        size_t pageIndex = 0;
        size_t statIndex = 0;
        loadPage(pmc, pageList[pageIndex], statIndex);

        Curses curses(rate);
        time_t last_update = 0;

        while(!quit) {
            bool update;
            int ch = getch();

            if(ch != ERR)
            {
                update = (time(NULL) - last_update) >= rate;

                switch(ch)
                {
                    case 'q':
                    {
                        quit = true;
                        break;
                    }
                    case 'C':
                    {
                        perCpu = !perCpu;
                        erase();
                        update = true;
                        break;
                    }
                    case '>':
                    case KEY_RIGHT:
                    {
                        if(pageIndex != pageList.size() - 1)
                        {
                            pageIndex++;
                            statIndex = 0;
                            loadPage(pmc, pageList[pageIndex], statIndex);
                            update = true;
                        }
                        break;
                    }
                    case '<':
                    case KEY_LEFT:
                    {
                        if(pageIndex != 0)
                        {
                            pageIndex--;
                            statIndex = 0;
                            loadPage(pmc, pageList[pageIndex], statIndex);
                            update = true;
                        }
                        break;
                    }
                    case KEY_UP:
                    {
                        if(statIndex != 0)
                        {
                            statIndex--;
                            loadPage(pmc, pageList[pageIndex], statIndex);
                            update = true;
                        }
                        break;
                    }
                    case KEY_DOWN:
                    {
                        if(statIndex != pageList[pageIndex]->getStats().size() - 1)
                        {
                            statIndex++;
                            loadPage(pmc, pageList[pageIndex], statIndex);
                            update = true;
                        }
                        break;
                    }
                    default:
                    {
                        std::map<char, int>::iterator indexIt = pageMap.find(ch);
                        if(indexIt != pageMap.end())
                        {
                            pageIndex = indexIt->second;
                            statIndex = 0;
                            loadPage(pmc, pageList[pageIndex], statIndex);
                            update = true;
                        }
                        break;
                    }
                }
            }
            else
                update = true;

            if(update)
            {
                pmc.readPmcs();

                move(0, 0);
                printw("%s: (hotkey %c)", pageList[pageIndex]->getName().c_str(),
                       pageList[pageIndex]->getShortcut()[0]);

                int row = 1;
                int i;
                if(perCpu)
                {
                    for(i = 0; i < ncpu; i++)
                    {
                        move(row, HEADER_WIDTH + STAT_WIDTH * i);
                        printw("CPU%d", i);
                    }
                }
                row++;

                PointerVector<Statistic>::iterator it;
                PointerVector<Statistic> & stats = pageList[pageIndex]->getStats();
                for(it = stats.begin(); it != stats.end(); ++it, ++row)
                {
                    Statistic & stat = **it;
                    Expression & expr = stat.getExpr();

                    move(row, 0);
                    printw("%s: ", stat.getName().c_str());

                    if(perCpu)
                    {
                        for(i = 0; i < ncpu; i++)
                        {
                            PerCpuExprEvaluator eval(pmc, i);
                            move(row, HEADER_WIDTH + STAT_WIDTH * i);

                            try {
                                expr.accept(eval);
                                double value = expr.getValue();
                                Statistic::Status status = stat.getStatus(value);
                                curses.startColor(status);
                                attron(A_BOLD);
                                printw("%.2lf\n", value);
                                attroff(A_BOLD);
                                curses.endColor(status);
                            }
                            catch(PmcNotLoaded & e)
                            {
                                printw("N/A");
                            }
                        }
                    }
                    else
                    {
                        ExprEvaluator eval(pmc);
                        move(row, HEADER_WIDTH);

                        try {
                            expr.accept(eval);
                            double value = expr.getValue();
                            Statistic::Status status = stat.getStatus(value);
                            curses.startColor(status);
                            attron(A_BOLD);
                            printw("%.2lf\n", value);
                            attroff(A_BOLD);
                            curses.endColor(status);
                        }
                        catch(PmcNotLoaded & e)
                        {
                            printw("Not Available");
                        }
                    }
                }

                refresh();
                last_update = time(NULL);
            }
        }
    }
    catch (PmcException & e)
    {
        fprintf(stderr, "%s\n", e.what());
        return -1;
    }

    return 0;
}
