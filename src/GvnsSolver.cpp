#include "GvnsSolver.hpp"
#include "Constants.hpp"

#include <algorithm>
#include <random>
#include <utility>

namespace mscflp {

void GvnsSolver::shake(Solution &sol, int k) {
    if (inst.n == 0) {
        return;
    }
    std::uniform_int_distribution<int> customerDist(0, inst.n - 1);
    std::vector<int> selected;
    const int amount = std::max(1, k * std::max(1, opt.rclSize));
    selected.reserve(amount);
    for (int r = 0; r < amount; ++r) {
        selected.push_back(customerDist(rng));
    }
    repairCustomers(sol, selected, true, true);

    if (k >= 2) {
        localShift(sol);
    }
    if (k >= 3) {
        localSwapCustomers(sol);
    }
}

RunResult GvnsSolver::run() {
    startTimer();

    Solution current = construct(false);
    vndReinforcement(current);
    current.validate(false);

    if (!timeExpired()) {
        Solution randomized = construct(true);
        if (randomized.feasible) {
            vndReinforcement(randomized);
            randomized.validate(false);
            if (randomized.feasible && randomized.totalCost() + EPS < current.totalCost()) {
                current = std::move(randomized);
            }
        }
    }

    Solution best = current;

    int iterations = 0;
    while (!timeExpired() && iterations < opt.gvnsIterations) {
        int k = 1;
        while (k <= opt.kMax && !timeExpired()) {
            Solution candidate = current;
            shake(candidate, k);
            if (!candidate.feasible) {
                ++k;
                continue;
            }
            vndReinforcement(candidate);
            candidate.validate(false);
            if (candidate.feasible && candidate.totalCost() + EPS < current.totalCost()) {
                current = std::move(candidate);
                if (current.totalCost() + EPS < best.totalCost()) {
                    best = current;
                }
                k = 1;
            } else {
                ++k;
            }
        }
        ++iterations;
    }

    best.validate(false);
    return makeResult("gvns", best, elapsedSeconds());
}

} // namespace mscflp
