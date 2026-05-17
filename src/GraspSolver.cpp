#include "GraspSolver.hpp"
#include "Constants.hpp"

#include <utility>

namespace mscflp {

RunResult GraspSolver::run() {
    startTimer();

    Solution best = construct(false);
    rvnd(best);
    best.validate(false);

    for (int it = 0; it < opt.graspIterations && !timeExpired(); ++it) {
        Solution candidate = construct(true);
        if (!candidate.feasible) {
            continue;
        }
        rvnd(candidate);
        if (candidate.feasible && candidate.totalCost() + EPS < best.totalCost()) {
            best = std::move(candidate);
        }
    }

    best.validate(false);
    return makeResult("grasp", best, elapsedSeconds());
}

} // namespace mscflp
