#include "GreedySolver.hpp"

namespace mscflp {

RunResult GreedySolver::run() {
    startTimer();
    Solution sol = construct(false);
    sol.validate(false);
    return makeResult("greedy", sol, elapsedSeconds());
}

} // namespace mscflp
