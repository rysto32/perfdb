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

#include "DetermineAgentVisitor.h"
#include "expression.h"
#include "pmccontext.h"
#include "statistic.h"
#include "pointervector.h"
#include "page.h"
#include "cpu.h"
#include "screenstate.h"
#include "parser.h"
#include "UncoreContext.h"

bool quit = 0;

#define HEADER_WIDTH 20
#define STAT_WIDTH 6

void
quit_signal_handler(int sig)
{
	quit = 1;
}

class Curses
{
private:
	short backgroundColour;
	short foregroundColour;
public:
	Curses(int rate)
	{
		initscr();
		cbreak();
		noecho();
		keypad(stdscr, TRUE);
		halfdelay(10);

		if (has_colors()) {
			start_color();
			pair_content(0, &foregroundColour, &backgroundColour);

			init_pair(Statistic::STAT_GOOD, COLOR_CYAN,
			    backgroundColour);
			init_pair(Statistic::STAT_OK, COLOR_GREEN,
			    backgroundColour);
			init_pair(Statistic::STAT_BAD, COLOR_YELLOW,
			    backgroundColour);
			init_pair(Statistic::STAT_TERRIBLE, COLOR_RED,
			    backgroundColour);
		}
	}

	void startColor(Statistic::Status status)
	{
		if (has_colors())
			attron(COLOR_PAIR(status));
	}

	void endColor(Statistic::Status status)
	{
		if (has_colors())
			attroff(COLOR_PAIR(status));
	}

	~Curses()
	{
		endwin();
	}
};

class ExprEvaluator : public PostOrderExprVisitor
{
	StatContext &m_pmc;

	public:
	ExprEvaluator(StatContext &pmc)
	    : m_pmc(pmc)
	{
	}

	virtual void visit(BinaryExpr &expr)
	{
		double left = expr.getLeft().getValue();
		double right = expr.getRight().getValue();
		double value;

		switch (expr.getOp()) {
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

	virtual void visit(PmcExpr &expr)
	{
		expr.setValue(m_pmc.getStat(expr.getPmc()));
	}

	virtual void visit(ConstExpr &expr)
	{
	}
};

class PerCpuExprEvaluator : public PostOrderExprVisitor
{
	StatContext & m_pmc;
	int m_cpu;

	public:
	PerCpuExprEvaluator(StatContext &pmc, int cpu)
	    : m_pmc(pmc), m_cpu(cpu)
	{
	}

	virtual void visit(BinaryExpr &expr)
	{
		double left = expr.getLeft().getValue();
		double right = expr.getRight().getValue();
		double value;

		switch (expr.getOp()) {
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
			throw std::runtime_error("Unrecognized operator type");
		}

		expr.setValue(value);
	}

	virtual void visit(PmcExpr &expr)
	{
		expr.setValue(m_pmc.getStatCpu(expr.getPmc(), m_cpu));
	}

	virtual void visit(ConstExpr &expr)
	{
		/* 
		 * Nothing to do because expr.getValue() will already
		 * return the right value for a ConstExpr
		 */
	}
};

extern FILE *yyin;
FILE *logfd;
int yyparse();
extern int yydebug;
extern YYSTYPE yyval;

void
usage(char * exe)
{
	fprintf(stderr, "usage: %s [-C -c cpuspec -w rate -f stats_file ]\n", 
	    exe);
}

int
parseCpuMask(char * optarg)
{
	int cpu;
	int cpuMask = 0;
	char *endp = optarg;

	while (*endp != '\0') {
		cpu = strtol(optarg, &endp, 10);

		if (*endp != ',' && *endp != '\0') {
			throw StatException(
			    "Invalid character in cpuspec passed to -c option");
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
	const struct pmc_cpuinfo *cpuInfo;
	const char *cpuName;
	int error;

	error = pmc_cpuinfo(&cpuInfo);
	if (error) {
		std::string msg("error in pmc_cpuinfo: ");
		msg += strerror(errno);
		throw StatException(msg);
	}

	cpuName = pmc_name_of_cputype(cpuInfo->pm_cputype);

	for (it = cpuDefs.begin(); it != cpuDefs.end(); ++it) {
		if ((*it)->getName() == cpuName)
			return (*it)->getPageList();
	}

	std::string msg("No definitions for CPU type ");
	msg += cpuName;
	throw StatException(msg);
}

int
GetNumCols(StatContext &pmc, PointerVector<Statistic> &stats)
{
    CounterAgent agent;
    DetermineAgentVisitor v(pmc);

    PointerVector<Statistic>::iterator it;
    agent = ANY_AGENT;
    for (it = stats.begin(); it != stats.end(); ++it) {
        Statistic & stat = **it;
		
        stat.getExpr().accept(v);
        agent = CombineAgents(agent, v.GetAgent(stat.getExpr()));
    }
    
    return pmc.getNumAgents(agent);
}

int
main(int argc, char **argv)
{
	int rate = 1;
	int error;
	int ncols;
	bool perCpu = true;
	int option;
	char *endp;
	uint32_t cpuMask;
	std::string statsFile(dirname(argv[0]));

	bool logToFile = false;
	std::string logFile(dirname(argv[0]));
	time_t curTime;
	struct tm * timeinfo;

    PmcContext fake;
	
	statsFile += "/stats.txt";
	try {
		UncoreContext pmc;

		while ((option = getopt(argc, argv, "Cc:w:hf:l:")) != -1) {
			switch (option) {
			case 'C':
				perCpu = !perCpu;
				break;
			case 'c':
				cpuMask = parseCpuMask(optarg);
				break;
			case 'w':
				rate = strtol(optarg, &endp, 10);
				if (*endp != '\0') {
					fprintf(stderr, 
					    "-w requires numeric argument\n");
					return (-1);
				}
				break;
			case 'f':
				statsFile = optarg;
				break;
			case 'l':
				logToFile = true;
				logFile = optarg;
				break;
			case 'h':
			default:
				usage(argv[0]);
				return (-1);
			}
		}

		//pmc.setCpuMask(cpuMask);

		if (logToFile) {
			logfd = fopen(logFile.c_str(), "a");
			if (!logfd) {
				fprintf(stderr, "Unable to open log file.\n");
				return(-1);
			}
		}

		yyin = fopen(statsFile.c_str(), "r");

		if (!yyin) {
			fprintf(stderr, "Could not read %s\n", 
			    statsFile.c_str());
			return (-1);
		}

		error = yyparse();
		if (error) {
			fprintf(stderr, "Could not parse %s\n", 
			    statsFile.c_str());
			return (-1);
		}

		std::auto_ptr<PointerVector<CpuDef> > cpuList(yyval.cpuList);

		ScreenState state(getPageList(*cpuList), perCpu, pmc, rate);

		signal(SIGBUS, quit_signal_handler);
		signal(SIGTERM, quit_signal_handler);

		state.LoadPage(pmc);

		Curses curses(rate);

		while (!quit) {

			state.WaitForKeypress(pmc);

			if (state.UpdateScreen()) {
				pmc.readStats();
				time(&curTime);
				timeinfo = localtime(&curTime);
                
                ncols = GetNumCols(pmc, state.ScreenStats());

				move(0, 0);
				printw("%s: (hotkey %s)", state.ScreenName(),
				    state.ScreenShortcut());

				int row = 1;
				int i;
				if (state.PerCPU() && ncols > 0) {
					for (i = 0; i < ncols; i++) {
						move(row, HEADER_WIDTH + STAT_WIDTH * i);
						printw("CH%d", i);
					}
				}
				row++;
				if (logToFile) fprintf(logfd, "%s", asctime(timeinfo));

				PointerVector<Statistic>::iterator it;
				PointerVector<Statistic> &stats = state.ScreenStats();
				int statsRowsStart = row;
				for (it = stats.begin(); it != stats.end(); ++it, ++row) {
					Statistic & stat = **it;
					Expression & expr = stat.getExpr();

					move(row, 0);
					printw("%s: ", stat.getName().c_str());
					if (logToFile) fprintf(logfd, "%15s:\t", stat.getName().c_str());

					if (state.PerCPU() && ncols > 0) {
						for (int cpu = 0; cpu < ncols; cpu++) {
							PerCpuExprEvaluator eval(pmc, cpu);
							move(row, HEADER_WIDTH + STAT_WIDTH * cpu - 1);

							try {
								double value;
								try {
									expr.accept(eval);
									value = expr.getValue();
									stat.setLastValue(value, cpu);
								} catch (StatNotLoaded & e) {
									state.MissedStat(row - statsRowsStart);
									value = stat.getLastValue(cpu);
								}
								Statistic::Status status = stat.getStatus(value);
								curses.startColor(status);
								attron(A_BOLD);
								printw(" %.2lf\n", value);
								if (logToFile) fprintf(logfd, " %.2lf\t", value);
								attroff(A_BOLD);
								curses.endColor(status);
							} catch (StatNotLoaded & e) {
								printw(" N/A");
								if (logToFile) fprintf(logfd, " N/A");
							}
						}
					} else {
						ExprEvaluator eval(pmc);
						move(row, HEADER_WIDTH);
						if (logToFile) fprintf(logfd, "\n");

						try {
							double value;
							try {
								expr.accept(eval);
								value = expr.getValue();
								stat.setLastValue(value);
							} catch (StatNotLoaded & e) {
								state.MissedStat(row - statsRowsStart);
								value = stat.getLastValue();
							}
							Statistic::Status status = stat.getStatus(value);
							curses.startColor(status);
							attron(A_BOLD);
							printw("%.2lf\n", value);
							if (logToFile) fprintf(logfd, "%.2lf\n", value);
							attroff(A_BOLD);
							curses.endColor(status);
						} catch(StatNotLoaded & e) {
							printw("Not Available");
							if (logToFile) fprintf(logfd, "Not Available");
						}
					}
					if (logToFile) fprintf(logfd, "\n");
				}

				refresh();
				state.CompleteUpdate();
				if (logToFile) {
					fprintf(logfd, "\n");
					fflush(logfd);
				}
			}
		}
	}
	catch (StatException & e)
	{
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}

	return 0;
}
