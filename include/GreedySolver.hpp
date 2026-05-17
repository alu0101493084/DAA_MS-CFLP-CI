#ifndef MS_CFLP_GREEDY_SOLVER_HPP
#define MS_CFLP_GREEDY_SOLVER_HPP

#include "HeuristicSolver.hpp"

namespace mscflp {

class GreedySolver : public HeuristicSolver {
  public:
    using HeuristicSolver::HeuristicSolver;

    RunResult run();
};

} // namespace mscflp

#endif
