#ifndef MS_CFLP_HEURISTIC_SOLVER_HPP
#define MS_CFLP_HEURISTIC_SOLVER_HPP

#include "RunTypes.hpp"

#include <chrono>
#include <random>
#include <string>
#include <vector>

namespace mscflp {

class HeuristicSolver {
  public:
    HeuristicSolver(const Instance &instance, RunOptions options);
    virtual ~HeuristicSolver() = default;

  protected:
    const Instance &inst;
    RunOptions opt;
    std::mt19937 rng;
    std::chrono::steady_clock::time_point startTime;

    void startTimer();
    double elapsedSeconds() const;
    bool timeExpired() const;

    Solution construct(bool randomized);
    void selectInitialFacilities(Solution &sol, bool randomized);
    bool assignCustomer(Solution &sol, int customer, double quantity, bool randomized, bool allowOpening);
    bool repairCustomers(Solution &sol, const std::vector<int> &customers, bool randomized, bool allowOpening);

    bool localShift(Solution &sol);
    bool localSwapCustomers(Solution &sol);
    bool localSwapFacilities(Solution &sol);
    bool localIncompatibilityRelief(Solution &sol);

    bool applyNeighborhood(Solution &sol, int neighborhood);
    void rvnd(Solution &sol);
    void vndReinforcement(Solution &sol);

    RunResult makeResult(const std::string &algorithm, const Solution &solution, double seconds) const;
};

} // namespace mscflp

#endif
