#include "HeuristicSolver.hpp"
#include "Constants.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <numeric>
#include <random>
#include <utility>

namespace mscflp {

namespace {

template <typename T>
void makeUnique(std::vector<T> &values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

} // namespace

HeuristicSolver::HeuristicSolver(const Instance &instance, RunOptions options)
    : inst(instance), opt(std::move(options)), rng(opt.seed) {}

void HeuristicSolver::startTimer() {
    startTime = std::chrono::steady_clock::now();
}

double HeuristicSolver::elapsedSeconds() const {
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - startTime).count();
}

bool HeuristicSolver::timeExpired() const {
    if (opt.timeLimitSeconds <= 0.0) {
        return false;
    }
    return elapsedSeconds() >= opt.timeLimitSeconds;
}

void HeuristicSolver::selectInitialFacilities(Solution &sol, bool randomized) {
    std::vector<int> pool = inst.facilitiesByFixedCost;
    double cumulativeCapacity = 0.0;

    auto choosePosition = [&](int limit) {
        if (!randomized) {
            return 0;
        }
        const int top = std::max(1, std::min(limit, opt.rclSize));
        std::uniform_int_distribution<int> dist(0, top - 1);
        return dist(rng);
    };

    while (cumulativeCapacity + EPS < inst.totalDemand && !pool.empty()) {
        const int pos = choosePosition(static_cast<int>(pool.size()));
        const int facility = pool[pos];
        pool.erase(pool.begin() + pos);
        sol.openFacility(facility);
        cumulativeCapacity += inst.capacity[facility];
    }

    for (int k = 0; k < opt.extraFacilities && !pool.empty(); ++k) {
        const int pos = choosePosition(static_cast<int>(pool.size()));
        const int facility = pool[pos];
        pool.erase(pool.begin() + pos);
        sol.openFacility(facility);
    }
}

bool HeuristicSolver::assignCustomer(Solution &sol, int customer, double quantity, bool randomized, bool allowOpening) {
    double remaining = quantity;
    int guard = 0;
    while (remaining > EPS && guard++ < inst.m + 5) {
        std::vector<int> candidates;
        for (int facility : inst.facilitiesByCustomerCost[customer]) {
            if (!sol.isOpen(facility)) {
                continue;
            }
            if (sol.residualCapacity(facility) <= EPS) {
                continue;
            }
            if (!sol.canServe(customer, facility)) {
                continue;
            }
            candidates.push_back(facility);
            if (static_cast<int>(candidates.size()) >= std::max(1, opt.rclSize) && randomized) {
                break;
            }
            if (!randomized && static_cast<int>(candidates.size()) >= 1) {
                break;
            }
        }

        if (candidates.empty()) {
            if (!allowOpening) {
                return false;
            }
            std::vector<int> closed;
            for (int facility : inst.facilitiesByCustomerCost[customer]) {
                if (!sol.isOpen(facility) && inst.capacity[facility] > EPS) {
                    closed.push_back(facility);
                    if (static_cast<int>(closed.size()) >= std::max(1, opt.rclSize) && randomized) {
                        break;
                    }
                    if (!randomized && static_cast<int>(closed.size()) >= 1) {
                        break;
                    }
                }
            }
            if (closed.empty()) {
                return false;
            }
            int chosen = closed.front();
            if (randomized && closed.size() > 1) {
                std::uniform_int_distribution<int> dist(0, static_cast<int>(closed.size()) - 1);
                chosen = closed[dist(rng)];
            }
            sol.openFacility(chosen);
            continue;
        }

        int chosen = candidates.front();
        if (randomized && candidates.size() > 1) {
            std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size()) - 1);
            chosen = candidates[dist(rng)];
        }
        const double q = std::min(remaining, sol.residualCapacity(chosen));
        if (q <= EPS) {
            return false;
        }
        sol.addQuantity(customer, chosen, q);
        remaining -= q;
    }
    return remaining <= EPS;
}

Solution HeuristicSolver::construct(bool randomized) {
    Solution sol(&inst);
    selectInitialFacilities(sol, randomized);

    std::vector<int> customers(inst.n);
    std::iota(customers.begin(), customers.end(), 0);
    if (randomized) {
        std::uniform_int_distribution<int> modeDist(0, 2);
        const int mode = modeDist(rng);
        if (mode == 0) {
            std::shuffle(customers.begin(), customers.end(), rng);
        } else if (mode == 1) {
            std::stable_sort(customers.begin(), customers.end(), [&](int a, int b) {
                return inst.demand[a] > inst.demand[b];
            });
        } else {
            std::stable_sort(customers.begin(), customers.end(), [&](int a, int b) {
                const auto regret = [&](int c) {
                    int best = inst.facilitiesByCustomerCost[c][0];
                    int second = inst.facilitiesByCustomerCost[c][std::min(1, inst.m - 1)];
                    return std::abs(inst.supplyCost[c][second] - inst.supplyCost[c][best]);
                };
                return regret(a) > regret(b);
            });
        }
    }

    for (int customer : customers) {
        if (!assignCustomer(sol, customer, inst.demand[customer], randomized, true)) {
            sol.note = "No se pudo construir una solucion completa";
            sol.validate(false);
            return sol;
        }
    }

    sol.cleanupEmptyFacilities();
    sol.validate(false);
    return sol;
}

bool HeuristicSolver::repairCustomers(Solution &sol, const std::vector<int> &customers, bool randomized, bool allowOpening) {
    std::vector<int> uniqueCustomers = customers;
    makeUnique(uniqueCustomers);
    for (int customer : uniqueCustomers) {
        sol.clearCustomer(customer);
    }
    if (randomized) {
        std::shuffle(uniqueCustomers.begin(), uniqueCustomers.end(), rng);
    } else {
        std::stable_sort(uniqueCustomers.begin(), uniqueCustomers.end(), [&](int a, int b) {
            return inst.demand[a] > inst.demand[b];
        });
    }
    for (int customer : uniqueCustomers) {
        if (!assignCustomer(sol, customer, inst.demand[customer], randomized, allowOpening)) {
            return false;
        }
    }
    sol.cleanupEmptyFacilities();
    return sol.validate(false);
}

RunResult HeuristicSolver::makeResult(const std::string &algorithm, const Solution &solution, double seconds) const {
    RunResult result;
    result.instance = inst.name;
    result.algorithm = algorithm;
    result.seed = opt.seed;
    result.openFacilities = solution.openCount();
    result.fixedCost = solution.fixedCost;
    result.transportCost = solution.transportCost;
    result.totalCost = solution.totalCost();
    result.incompatibilityViolations = solution.incompatibilityViolations();
    result.capacityViolations = solution.capacityViolations();
    result.feasible = solution.feasible;
    result.seconds = seconds;
    result.solution = solution;
    return result;
}

} // namespace mscflp
