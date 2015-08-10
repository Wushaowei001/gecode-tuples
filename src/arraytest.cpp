/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *
 *  Last modified:
 *     $Date: 2013-07-08 22:22:40 +1000 (Mon, 08 Jul 2013) $ by $Author: schulte $
 *     $Revision: 13820 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/* -------------------------------
 *  Modified by:
 *  Farshid Hassani Bijarbooneh
 *
 *  This example was modified to simplify the compilation on Unix and
 *  Windows lab at Uppsala university and provide you with a customize output results.
 *
 *  -------------------------------
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <cstring>

using namespace std;
using namespace Gecode;
using namespace Gecode::Driver;

class Queens;

template<class BaseSpace>
class ScriptBaseCustom : public BaseSpace {
public:
    /// Default constructor
    ScriptBaseCustom(void) {}
    /// Constructor used for cloning
    ScriptBaseCustom(bool share, ScriptBaseCustom& e) : BaseSpace(share,e) {}
    /// Print a solution to \a os
    virtual void print(std::ostream& os) const { (void) os; }
    /// Compare with \a s
    virtual void compare(const Space&, std::ostream& os) const {
        (void) os;
    }
    /// Choose output stream according to \a name
    static std::ostream& select_ostream(const char* name, std::ofstream& ofs);
    
    template<class Script, template<class> class Engine, class Options>
    static void run(const Options& opt, Script* s=NULL);
private:
    template<class Script, template<class> class Engine, class Options,
    template<template<class> class,class> class Meta>
    static void runMeta(const Options& opt, Script* s);
    /// Catch wrong definitions of copy constructor
    explicit ScriptBaseCustom(ScriptBaseCustom& e);
};

typedef ScriptBaseCustom<Space> ScriptOutput;

template<class Space>
std::ostream&
ScriptBaseCustom<Space>::select_ostream(const char* name, std::ofstream& ofs) {
    if (strcmp(name, "stdout") == 0) {
        return std::cout;
    } else if (strcmp(name, "stdlog") == 0) {
        return std::clog;
    } else if (strcmp(name, "stderr") == 0) {
        return std::cerr;
    } else {
        ofs.open(name);
        return ofs;
    }
}

template<class Space>
template<class Script, template<class> class Engine, class Options>
void
ScriptBaseCustom<Space>::run(const Options& o, Script* s) {
    if (o.restart()==RM_NONE) {
        runMeta<Script,Engine,Options,EngineToMeta>(o,s);
    } else {
        runMeta<Script,Engine,Options,RBS>(o,s);
    }
}

template<class Space>
template<class Script, template<class> class Engine, class Options,
template<template<class> class,class> class Meta>
void
ScriptBaseCustom<Space>::runMeta(const Options& o, Script* s) {
    using namespace std;
    
    ofstream sol_file, log_file;
    
    ostream& s_out = select_ostream(o.out_file(), sol_file);
    ostream& l_out = select_ostream(o.log_file(), log_file);
    
    try {
        switch (o.mode()) {
            case SM_SOLUTION:
            {
//                l_out << o.name() << endl;
                Support::Timer t;
                int i = o.solutions();
                t.start();
                if (s == NULL)
                    s = new Script(o);
//                unsigned int n_p = s->propagators();
//                unsigned int n_b = s->branchers();
                Search::Options so;
                so.threads = o.threads();
                so.c_d     = o.c_d();
                so.a_d     = o.a_d();
                so.stop    = CombinedStop::create(o.node(),o.fail(), o.time(),
                                                  o.interrupt());
                so.cutoff  = createCutoff(o);
                so.clone   = false;
                
				if(o.interrupt()) CombinedStop::installCtrlHandler(true);
                
				{
                    Meta<Engine,Script> e(s,so);
                    if (o.print_last()) {
                        Script* px = NULL;
                        do {
                            Script* ex = e.next();
                            if (ex == NULL) {
                                if (px != NULL) {
                                    px->print(s_out);
                                    delete px;
                                }
                                break;
                            } else {
                                delete px;
                                px = ex;
                            }
                        } while (--i != 0);
                    } else {
                        do {
                            Script* ex = e.next();
                            if (ex == NULL)
                                break;
                            ex->print(s_out);
                            delete ex;
                        } while (--i != 0);
                    }
                    if (o.interrupt())
                        CombinedStop::installCtrlHandler(false);
                    Search::Statistics stat = e.statistics();

					cout << o.size() << " & ";

                    //                    s_out << endl;
                    if (e.stopped()) {
                        //                        l_out << "Search engine stopped..." << endl
                        //                        << "\treason: ";
                        int r = static_cast<CombinedStop*>(so.stop)->reason(stat,so);
                        if (r & CombinedStop::SR_INT)
                            l_out << "user interrupt " << endl;
                        else {
                            if (r & CombinedStop::SR_NODE)
                                l_out << "node ";
                            if (r & CombinedStop::SR_FAIL)
                                l_out << "fail ";
                            if (r & CombinedStop::SR_TIME)
                                l_out << "time ";
                            //                            l_out << "limit reached" << endl << endl;
                            l_out << "$>\\Timeout$";
                        }
                    }
                    else {
                        double runtime_msec = t.stop();
                        double runtime_sec = runtime_msec/1000;
                        //l_out.width(8);
//                    cout.fill(' ');
                        l_out << showpoint << fixed
                        << setprecision(3) << runtime_sec ;
                        //                    cout << " (";
                        //                    cout.width(9);
                        //                    cout.fill('0');
                        //                    cout << runtime_msec << " ms)";
                    }
                    cout << " & " << stat.fail << " \\\\" << endl;
                }
                delete so.stop;
            }
                break;
            default:
                Script::template run<Queens,DFS,SizeOptions>(o);
        }
    } catch (Exception& e) {
        cerr << "Exception: " << e.what() << "." << endl
        << "Stopping..." << endl;
        if (sol_file.is_open())
            sol_file.close();
        if (log_file.is_open())
            log_file.close();
        exit(EXIT_FAILURE);
    }
    if (sol_file.is_open())
        sol_file.close();
    if (log_file.is_open())
        log_file.close();
}

std::vector<int> queens_results;

class Queens : public Script {
public:
    /// Position of queens on boards
	BoolVarArray b;
	int size; //used for printing tables

    /// The actual problem
    Queens(const SizeOptions& opt) : b(*this, opt.size()*opt.size(), 0, 1)
	{
        const int n = opt.size();
		size = n;
		
		for (int i = 0; i < n; i++) {
			linear(*this, b.slice(i*n, 1, n), IRT_EQ, 1); //Rows
			linear(*this, b.slice(i, n, n), IRT_EQ, 1); //Columns
		}
		
		linear(*this, b.slice(0, n+1, n), IRT_LQ, 1); //Middle descend. diag.
		linear(*this, b.slice(n-1, n-1, n), IRT_LQ, 1); //Middle asscend. diag.	

		for (int i = 1; i < n-1; i++) {
			linear(*this, b.slice(i, n+1, n-i), IRT_LQ, 1); //Upper descend. diag.
			linear(*this, b.slice(n*i, n+1, n-i), IRT_LQ, 1); //Lower descend. diag.
			linear(*this, b.slice(n-(i+1), n-1, n-i), IRT_LQ, 1); //Upper ascend. diag.
			linear(*this, b.slice(n*(i+1)-1, n-1, n-i), IRT_LQ, 1); //Lower ascend. diag.
		}

		branch(*this, b, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    }
    
    /// Constructor for cloning \a s
    Queens(bool share, Queens& s) : Script(share,s), size(s.size) {
       b.update(*this, share, s.b);
    }
    
    /// Perform copying during cloning
    virtual Space*
    copy(bool share) {
        return new Queens(share,*this);
    }
    
    /// Print solution
    virtual void print(std::ostream& os) const {
		int n = size;
        // comment out the following lines to output the solution
        os << "queens\t\n";
        for (int i = 0; i < b.size(); i++) {
            queens_results.push_back (b[i].val());
        }
        os << std::endl;
    }
};

int main(int argc, char* argv[]) {
    // "It should ..."
    // The arraytest.cpp file should do this or that.

    SizeOptions opt("Queens");
    opt.iterations(500);
    opt.size(8);
    
    // comment out the following line to get a graphical view of the search tree
	//opt.mode(Gecode::SM_GIST);
    
    opt.parse(argc,argv);
    ScriptOutput::run<Queens,DFS,SizeOptions>(opt);

    int gold[] = {
        0, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 1, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 1, 0, 0, 0 };
    if (0 == memcmp(&queens_results[0],gold,8*8*sizeof(int)))
    {
        cout << "Ok" << endl;
        return 0;
    }
    else
    {
        cout << "Fail" << endl;
        return 1;
    }
}

// STATISTICS: example-any
