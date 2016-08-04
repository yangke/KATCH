//===-- TimingSolver.h ------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_TIMINGSOLVER_H
#define KLEE_TIMINGSOLVER_H

#include "klee/Expr.h"
#include "klee/Solver.h"

#include <vector>
#include <map>
namespace klee {
  class ExecutionState;
  class Solver;
  class STPSolver;
  class SeedInfo;

  /// TimingSolver - A simple class which wraps a solver and handles
  /// tracking the statistics that we care about.
  class TimingSolver {
  public:
    Solver *solver;
    STPSolver *stpSolver;
    bool simplifyExprs;

    std::map<ExecutionState*, std::vector<SeedInfo> > *seedMap;

  public:
    /// TimingSolver - Construct a new timing solver.
    ///
    /// \param _simplifyExprs - Whether expressions should be
    /// simplified (via the constraint manager interface) prior to
    /// querying.
    TimingSolver(Solver *_solver, STPSolver *_stpSolver, 
                 bool _simplifyExprs = true,
                 std::map<ExecutionState*, std::vector<SeedInfo> > *_seedMap = NULL) 
      : solver(_solver), stpSolver(_stpSolver), simplifyExprs(_simplifyExprs), seedMap(_seedMap) {}
    ~TimingSolver() {
      delete solver;
    }

    void setTimeout(double t) {
      stpSolver->setTimeout(t);
    }

    bool evaluate(const ExecutionState&, ref<Expr>, Solver::Validity &result, bool useSeeds = true);

    bool mustBeTrue(const ExecutionState&, ref<Expr>, bool &result, bool useSeeds = true);

    bool mustBeFalse(const ExecutionState&, ref<Expr>, bool &result, bool useSeeds = true);

    bool mayBeTrue(const ExecutionState&, ref<Expr>, bool &result, bool useSeeds = true);

    bool mayBeFalse(const ExecutionState&, ref<Expr>, bool &result, bool useSeeds = true);

    bool getValue(const ExecutionState &, ref<Expr> expr, 
                  ref<ConstantExpr> &result, bool useSeeds = true);

    bool getInitialValues(const ExecutionState&, 
                          const std::vector<const Array*> &objects,
                          std::vector< std::vector<unsigned char> > &result);

    std::pair< ref<Expr>, ref<Expr> >
    getRange(const ExecutionState&, ref<Expr> query, bool useSeeds = true);
  private:
    ref<Expr> ZESTEvaluate(const ExecutionState& state, ref<Expr> expr);
  };

}

#endif
