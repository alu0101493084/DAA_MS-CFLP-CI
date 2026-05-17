#ifndef MS_CFLP_GRASP_CONSTRUCTIVE_SOLVER_HPP
#define MS_CFLP_GRASP_CONSTRUCTIVE_SOLVER_HPP

#include "HeuristicSolver.hpp"

namespace mscflp {

class GraspConstructiveSolver : public HeuristicSolver {
  public:
    using HeuristicSolver::HeuristicSolver;

    RunResult run();
};

} // namespace mscflp

#endif
