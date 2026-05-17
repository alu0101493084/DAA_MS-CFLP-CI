#ifndef MS_CFLP_GVNS_SOLVER_HPP
#define MS_CFLP_GVNS_SOLVER_HPP

#include "HeuristicSolver.hpp"

namespace mscflp {

class GvnsSolver : public HeuristicSolver {
  public:
    using HeuristicSolver::HeuristicSolver;

    RunResult run();

  private:
    void shake(Solution &sol, int k);
};

} // namespace mscflp

#endif
