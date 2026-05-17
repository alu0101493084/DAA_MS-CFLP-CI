#include "GraspConstructiveSolver.hpp"

namespace mscflp {

RunResult GraspConstructiveSolver::run() {
    startTimer();
    Solution sol = construct(true);
    sol.validate(false);
    return makeResult("grasp-constructive", sol, elapsedSeconds());
}

} // namespace mscflp
