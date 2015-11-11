#include <gecode/kernel.hh>
#include "../intpairapprox.h"
#include <vector>
#include <stdlib.h>
#include "../common/solutionok.h"
#include <gecode/gist.hh>
#include "../testsrc/_testbase.cpp"

int noSolutions;

using namespace MPG::IntPair;
using namespace MPG;

#include "../common/dfaimp.h"

Dfa *df;

int seed, nostates, notokens, maxcost, maxcosttotal, nosteps;

class Test : public Script {
public:
  /// The actual problem
  IntPairApproxVarArray p;
  IntVarArray z;
  IntPairApproxVar init;
  
  Test(const SizeOptions& opt) : p(*this, nosteps+1,1,nostates,0,maxcosttotal),
				 z(*this, nosteps,1,notokens),
				 init(*this, 1,1,0,0)
  {
    eq(*this, p[0], init);
    for(int i=0; i<nosteps; i++)
      mydfa(*this, p[i+1],p[i],z[i],df);
    branch(*this, z, INT_VAR_NONE(), INT_VAL_MIN());
    //    nonenone(*this, p);
  }

  
  /// Constructor for cloning \a s
#ifndef GC_UPDATE
#define GC_UPDATE(var) var.update(*this, share, s.var)
#endif
  
  Test(bool share, Test& s) : Script(share,s) {
    // To update a variable var use:
    // GC_UPDATE(var)
    GC_UPDATE(p);
    GC_UPDATE(z);
    GC_UPDATE(init);
  }
    
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
        return new Test(share,*this);
  }

  /// Print solution (originally, now it's just for updating number of solutions)
  virtual void print(std::ostream& os) const {
    // Strange place to put this, but since this functions is called once for every solution ...
    //    for(int i=0; i<nosteps; i++)
    //    assert(solutionOk(df, p[i+1].val().x, p[i+1].val().y, p[i].val().x, p[i].val().y, z[i].val()));

    //    for(int i=0; i<nosteps; i++)
    //      cout << z[i].val() << " " << p[i].val().x << " " << p[i].val().y << " ";
    //    cout << p[nosteps].val().x << " " << p[nosteps].val().y << endl;
    //  cout << a[1] << " "  << a[0] << " " << z << endl;
    noSolutions++;
  }
};

#include "_main.cpp"
/*
int main(int argc, char* argv[]) {
    SizeOptions opt("Queens");
    opt.solutions(0); // Calculate all solutions
    noSolutions=0;

    if(argc < 9) {
      std::cout << "Usage: " << argv[0] << " <seed> <no states> <no tokens> <max cost per path> <max total cost> <nosteps>" << std::endl;
      return -1;
    }

    const int offset = 2;
    seed = atoi(argv[1+offset]);
    nostates = atoi(argv[2+offset]);
    notokens = atoi(argv[3+offset]);
    maxcost = atoi(argv[4+offset]);
    maxcosttotal = atoi(argv[5+offset]);
    nosteps = atoi(argv[6+offset]);
    if(seed == 0 || nostates == 0 || notokens == 0 || maxcost == 0 || maxcosttotal==0 || nosteps == 0) {
      std::cout << "Wrong parameters" << std::endl;
      return -1;
    }
      
    df = new Dfa(seed, nostates, notokens, maxcost);
    //    df->print();
        opt.mode(Gecode::SM_GIST);
            opt.parse(argc,argv);
	    //    ScriptOutput::run<Test,DFS,SizeOptions>(opt);
	    Test *T = new Test(opt);
	    Gist::dfs(T);

    cout << "No solutions: " << noSolutions << endl;
    return 0;
    } */

// STATISTICS: example-any

