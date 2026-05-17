#include "HeuristicSolver.hpp"
#include "Constants.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <unordered_set>

namespace mscflp {

namespace {

std::vector<int> shuffledIndices(int size, std::mt19937 &rng) {
    std::vector<int> v(size);
    std::iota(v.begin(), v.end(), 0);
    std::shuffle(v.begin(), v.end(), rng);
    return v;
}

template <typename T>
void makeUnique(std::vector<T> &values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

} // namespace

bool HeuristicSolver::localShift(Solution &sol) {
    std::vector<int> customers = shuffledIndices(inst.n, rng);
    const int maxTargets = std::min(inst.m, 30 + 5 * std::max(1, opt.rclSize));
    for (int customer : customers) {
        const std::vector<Assignment> assignments = sol.assignedToCustomer[customer];
        for (const Assignment &a : assignments) {
            if (a.quantity <= EPS) {
                continue;
            }
            const int from = a.facility;
            const double q = a.quantity;
            int inspected = 0;
            for (int to : inst.facilitiesByCustomerCost[customer]) {
                if (++inspected > maxTargets) {
                    break;
                }
                if (to == from || !sol.isOpen(to)) {
                    continue;
                }
                if (sol.residualCapacity(to) + EPS < q) {
                    continue;
                }
                if (!sol.canServe(customer, to)) {
                    continue;
                }
                double delta = q * (inst.supplyCost[customer][to] - inst.supplyCost[customer][from]);
                if (sol.load[from] - q <= EPS) {
                    delta -= inst.fixedCost[from];
                }
                if (delta < -EPS) {
                    sol.removeQuantity(customer, from, q);
                    sol.addQuantity(customer, to, q);
                    sol.validate(false);
                    return true;
                }
            }
        }
    }
    return false;
}

bool HeuristicSolver::localSwapCustomers(Solution &sol) {
    std::vector<int> single;
    single.reserve(inst.n);
    for (int i = 0; i < inst.n; ++i) {
        if (sol.assignedToCustomer[i].size() == 1) {
            single.push_back(i);
        }
    }
    if (single.size() < 2) {
        return false;
    }

    const int samples = std::min(20000, static_cast<int>(single.size()) * 20);
    std::uniform_int_distribution<int> dist(0, static_cast<int>(single.size()) - 1);
    for (int s = 0; s < samples; ++s) {
        int i1 = single[dist(rng)];
        int i2 = single[dist(rng)];
        if (i1 == i2) {
            continue;
        }
        const int j1 = sol.assignedToCustomer[i1][0].facility;
        const int j2 = sol.assignedToCustomer[i2][0].facility;
        if (j1 == j2) {
            continue;
        }
        const double d1 = inst.demand[i1];
        const double d2 = inst.demand[i2];
        if (sol.load[j1] - d1 + d2 > inst.capacity[j1] + EPS ||
            sol.load[j2] - d2 + d1 > inst.capacity[j2] + EPS) {
            continue;
        }
        if (!sol.canServeIgnoring(i2, j1, i1) || !sol.canServeIgnoring(i1, j2, i2)) {
            continue;
        }
        const double delta =
            d1 * (inst.supplyCost[i1][j2] - inst.supplyCost[i1][j1]) +
            d2 * (inst.supplyCost[i2][j1] - inst.supplyCost[i2][j2]);
        if (delta < -EPS) {
            sol.clearCustomer(i1);
            sol.clearCustomer(i2);
            sol.addQuantity(i1, j2, d1);
            sol.addQuantity(i2, j1, d2);
            sol.validate(false);
            return true;
        }
    }
    return false;
}

bool HeuristicSolver::localSwapFacilities(Solution &sol) {
    std::vector<int> openFacilities;
    std::vector<int> closedFacilities;
    for (int j = 0; j < inst.m; ++j) {
        if (sol.isOpen(j)) {
            openFacilities.push_back(j);
        } else {
            closedFacilities.push_back(j);
        }
    }
    if (openFacilities.empty() || closedFacilities.empty()) {
        return false;
    }
    std::stable_sort(openFacilities.begin(), openFacilities.end(), [&](int a, int b) {
        const double scoreA = inst.fixedCost[a] / std::max(1.0, sol.load[a]);
        const double scoreB = inst.fixedCost[b] / std::max(1.0, sol.load[b]);
        return scoreA > scoreB;
    });

    const int maxDropTries = std::min(25, static_cast<int>(openFacilities.size()));
    for (int oi = 0; oi < maxDropTries; ++oi) {
        const int oldFacility = openFacilities[oi];
        if (sol.clientsOfFacility[oldFacility].empty()) {
            continue;
        }
        Solution candidate = sol;
        std::vector<int> affected = candidate.clientsOfFacility[oldFacility];
        makeUnique(affected);
        for (int customer : affected) {
            candidate.clearCustomer(customer);
        }
        candidate.closeFacilityIfEmpty(oldFacility);
        std::stable_sort(affected.begin(), affected.end(), [&](int a, int b) {
            return inst.demand[a] > inst.demand[b];
        });
        bool ok = true;
        for (int customer : affected) {
            if (!assignCustomer(candidate, customer, inst.demand[customer], true, false)) {
                ok = false;
                break;
            }
        }
        candidate.cleanupEmptyFacilities();
        if (ok && candidate.validate(false) && candidate.totalCost() + EPS < sol.totalCost()) {
            sol = std::move(candidate);
            return true;
        }
    }

    std::vector<std::pair<double, int>> expensiveCustomers;
    expensiveCustomers.reserve(inst.n);
    for (int i = 0; i < inst.n; ++i) {
        if (sol.assignedToCustomer[i].empty()) {
            continue;
        }
        double cost = 0.0;
        for (const Assignment &a : sol.assignedToCustomer[i]) {
            cost += a.quantity * inst.supplyCost[i][a.facility];
        }
        expensiveCustomers.emplace_back(cost / std::max(1.0, inst.demand[i]), i);
    }
    std::stable_sort(expensiveCustomers.begin(), expensiveCustomers.end(),
                     [](const auto &a, const auto &b) { return a.first > b.first; });

    std::unordered_set<int> candidateSet;
    const int topCustomers = std::min(250, static_cast<int>(expensiveCustomers.size()));
    for (int idx = 0; idx < topCustomers; ++idx) {
        const int customer = expensiveCustomers[idx].second;
        int inspected = 0;
        for (int facility : inst.facilitiesByCustomerCost[customer]) {
            if (++inspected > 20) {
                break;
            }
            if (!sol.isOpen(facility)) {
                candidateSet.insert(facility);
            }
        }
    }

    std::vector<int> openingCandidates(candidateSet.begin(), candidateSet.end());
    std::shuffle(openingCandidates.begin(), openingCandidates.end(), rng);
    const int maxOpeningTries = std::min(60, static_cast<int>(openingCandidates.size()));
    for (int idx = 0; idx < maxOpeningTries; ++idx) {
        const int newFacility = openingCandidates[idx];
        Solution candidate = sol;
        candidate.openFacility(newFacility);

        struct MoveCandidate {
            double saving = 0.0;
            int customer = -1;
        };
        std::vector<MoveCandidate> moves;
        moves.reserve(inst.n);
        for (int i = 0; i < inst.n; ++i) {
            double currentCost = 0.0;
            for (const Assignment &a : candidate.assignedToCustomer[i]) {
                currentCost += a.quantity * inst.supplyCost[i][a.facility];
            }
            const double currentUnitCost = currentCost / std::max(1.0, inst.demand[i]);
            const double saving = (currentUnitCost - inst.supplyCost[i][newFacility]) * inst.demand[i];
            if (saving > EPS && inst.demand[i] <= inst.capacity[newFacility] + EPS) {
                moves.push_back({saving, i});
            }
        }
        std::stable_sort(moves.begin(), moves.end(),
                         [](const MoveCandidate &a, const MoveCandidate &b) {
                             return a.saving > b.saving;
                         });

        int moved = 0;
        for (const MoveCandidate &move : moves) {
            const int customer = move.customer;
            if (candidate.residualCapacity(newFacility) + EPS < inst.demand[customer]) {
                continue;
            }
            if (!candidate.canServe(customer, newFacility)) {
                continue;
            }
            candidate.clearCustomer(customer);
            candidate.addQuantity(customer, newFacility, inst.demand[customer]);
            ++moved;
            if (candidate.residualCapacity(newFacility) <= EPS) {
                break;
            }
        }
        candidate.cleanupEmptyFacilities();
        if (moved > 0 && candidate.validate(false) && candidate.totalCost() + EPS < sol.totalCost()) {
            sol = std::move(candidate);
            return true;
        }
    }

    std::shuffle(openFacilities.begin(), openFacilities.end(), rng);

    std::vector<int> closedByFixed;
    for (int j : inst.facilitiesByFixedCost) {
        if (!sol.isOpen(j)) {
            closedByFixed.push_back(j);
            if (static_cast<int>(closedByFixed.size()) >= 20) {
                break;
            }
        }
    }
    if (closedByFixed.empty()) {
        return false;
    }

    const int maxOpenTries = std::min(12, static_cast<int>(openFacilities.size()));
    const int maxClosedTries = std::min(12, static_cast<int>(closedByFixed.size()));
    for (int oi = 0; oi < maxOpenTries; ++oi) {
        const int oldFacility = openFacilities[oi];
        if (sol.clientsOfFacility[oldFacility].empty()) {
            continue;
        }
        for (int ci = 0; ci < maxClosedTries; ++ci) {
            const int newFacility = closedByFixed[ci];
            Solution candidate = sol;
            std::vector<int> affected = candidate.clientsOfFacility[oldFacility];
            makeUnique(affected);
            candidate.openFacility(newFacility);
            for (int customer : affected) {
                candidate.clearCustomer(customer);
            }
            candidate.closeFacilityIfEmpty(oldFacility);

            std::stable_sort(affected.begin(), affected.end(), [&](int a, int b) {
                return inst.demand[a] > inst.demand[b];
            });
            bool ok = true;
            for (int customer : affected) {
                if (!assignCustomer(candidate, customer, inst.demand[customer], true, false)) {
                    ok = false;
                    break;
                }
            }
            candidate.cleanupEmptyFacilities();
            if (ok && candidate.validate(false) && candidate.totalCost() + EPS < sol.totalCost()) {
                sol = std::move(candidate);
                return true;
            }
        }
    }
    return false;
}

bool HeuristicSolver::localIncompatibilityRelief(Solution &sol) {
    std::vector<int> customers = shuffledIndices(inst.n, rng);
    std::stable_sort(customers.begin(), customers.end(), [&](int a, int b) {
        return inst.incompatibleOf[a].size() > inst.incompatibleOf[b].size();
    });

    const int maxCustomers = std::min(inst.n, 400);
    const int maxTargets = std::min(inst.m, 60);
    for (int idx = 0; idx < maxCustomers; ++idx) {
        const int customer = customers[idx];
        if (sol.assignedToCustomer[customer].empty()) {
            continue;
        }
        std::vector<Assignment> assignments = sol.assignedToCustomer[customer];
        std::stable_sort(assignments.begin(), assignments.end(), [&](const Assignment &a, const Assignment &b) {
            return a.quantity * inst.supplyCost[customer][a.facility] >
                   b.quantity * inst.supplyCost[customer][b.facility];
        });
        for (const Assignment &a : assignments) {
            const int from = a.facility;
            const double q = a.quantity;
            int inspected = 0;
            for (int to : inst.facilitiesByCustomerCost[customer]) {
                if (++inspected > maxTargets) {
                    break;
                }
                if (to == from || !sol.isOpen(to)) {
                    continue;
                }
                if (inst.supplyCost[customer][to] >= inst.supplyCost[customer][from]) {
                    continue;
                }
                if (sol.residualCapacity(to) + EPS < q || !sol.canServe(customer, to)) {
                    continue;
                }
                double delta = q * (inst.supplyCost[customer][to] - inst.supplyCost[customer][from]);
                if (sol.load[from] - q <= EPS) {
                    delta -= inst.fixedCost[from];
                }
                if (delta < -EPS) {
                    sol.removeQuantity(customer, from, q);
                    sol.addQuantity(customer, to, q);
                    sol.validate(false);
                    return true;
                }
            }
        }
    }
    return false;
}

bool HeuristicSolver::applyNeighborhood(Solution &sol, int neighborhood) {
    switch (neighborhood) {
    case 0:
        return localShift(sol);
    case 1:
        return localSwapCustomers(sol);
    case 2:
        return localIncompatibilityRelief(sol);
    case 3:
        return localSwapFacilities(sol);
    default:
        return false;
    }
}

void HeuristicSolver::rvnd(Solution &sol) {
    std::vector<int> neighborhoods = {0, 1, 2, 3};
    int passes = 0;
    while (!timeExpired() && passes < opt.localSearchPasses) {
        std::shuffle(neighborhoods.begin(), neighborhoods.end(), rng);
        bool improved = false;
        for (int h : neighborhoods) {
            if (timeExpired()) {
                break;
            }
            if (applyNeighborhood(sol, h)) {
                improved = true;
                break;
            }
        }
        if (!improved) {
            break;
        }
        ++passes;
    }
}

void HeuristicSolver::vndReinforcement(Solution &sol) {
    std::vector<double> weights = {1.0, 1.0, 1.0, 1.0};
    int noImprove = 0;
    int passes = 0;
    while (!timeExpired() && passes < opt.localSearchPasses && noImprove < 16) {
        std::discrete_distribution<int> pick(weights.begin(), weights.end());
        const int h = pick(rng);
        const double before = sol.totalCost();
        const bool improved = applyNeighborhood(sol, h);
        if (improved) {
            const double gain = std::max(0.0, before - sol.totalCost());
            const double reward = 1.0 + 1000.0 * gain / std::max(1.0, before);
            weights[h] = 0.75 * weights[h] + 0.25 * reward;
            noImprove = 0;
        } else {
            weights[h] = std::max(0.05, 0.90 * weights[h]);
            ++noImprove;
        }
        ++passes;
    }
}

} // namespace mscflp
