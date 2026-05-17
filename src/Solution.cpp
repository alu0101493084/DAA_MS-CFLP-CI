#include "Solution.hpp"
#include "Constants.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <unordered_set>

namespace mscflp {

Solution::Solution(const Instance *instance) : inst(instance) {
    open.assign(inst->m, 0);
    load.assign(inst->m, 0.0);
    assignedToCustomer.assign(inst->n, {});
    clientsOfFacility.assign(inst->m, {});
    served.assign(static_cast<std::size_t>(inst->n) * static_cast<std::size_t>(inst->m), 0);
}

int Solution::openCount() const {
    return static_cast<int>(std::count(open.begin(), open.end(), static_cast<unsigned char>(1)));
}

bool Solution::isServed(int customer, int facility) const {
    return served[static_cast<std::size_t>(customer) * inst->m + facility] != 0;
}

double Solution::residualCapacity(int facility) const {
    return inst->capacity[facility] - load[facility];
}

bool Solution::canServe(int customer, int facility) const {
    if (isServed(customer, facility)) {
        return true;
    }
    for (int other : inst->incompatibleOf[customer]) {
        if (isServed(other, facility)) {
            return false;
        }
    }
    return true;
}

bool Solution::canServeIgnoring(int customer, int facility, int ignoredCustomer) const {
    if (isServed(customer, facility)) {
        return true;
    }
    for (int other : inst->incompatibleOf[customer]) {
        if (other != ignoredCustomer && isServed(other, facility)) {
            return false;
        }
    }
    return true;
}

void Solution::openFacility(int facility) {
    if (!isOpen(facility)) {
        open[facility] = 1;
        fixedCost += inst->fixedCost[facility];
    }
}

void Solution::closeFacilityIfEmpty(int facility) {
    if (isOpen(facility) && load[facility] <= EPS && clientsOfFacility[facility].empty()) {
        open[facility] = 0;
        load[facility] = 0.0;
        fixedCost -= inst->fixedCost[facility];
    }
}

void Solution::addQuantity(int customer, int facility, double quantity) {
    if (quantity <= EPS) {
        return;
    }
    openFacility(facility);
    bool alreadyAssigned = false;
    for (Assignment &a : assignedToCustomer[customer]) {
        if (a.facility == facility) {
            a.quantity += quantity;
            alreadyAssigned = true;
            break;
        }
    }
    if (!alreadyAssigned) {
        assignedToCustomer[customer].push_back({facility, quantity});
        served[static_cast<std::size_t>(customer) * inst->m + facility] = 1;
        clientsOfFacility[facility].push_back(customer);
    }
    load[facility] += quantity;
    transportCost += quantity * inst->supplyCost[customer][facility];
}

bool Solution::removeQuantity(int customer, int facility, double quantity) {
    if (quantity <= EPS) {
        return true;
    }
    auto &assignments = assignedToCustomer[customer];
    for (auto it = assignments.begin(); it != assignments.end(); ++it) {
        if (it->facility != facility) {
            continue;
        }
        const double removed = std::min(quantity, it->quantity);
        it->quantity -= removed;
        load[facility] -= removed;
        transportCost -= removed * inst->supplyCost[customer][facility];
        if (load[facility] < EPS) {
            load[facility] = 0.0;
        }
        if (it->quantity <= EPS) {
            assignments.erase(it);
            served[static_cast<std::size_t>(customer) * inst->m + facility] = 0;
            auto &clients = clientsOfFacility[facility];
            clients.erase(std::remove(clients.begin(), clients.end(), customer), clients.end());
        }
        closeFacilityIfEmpty(facility);
        return std::abs(removed - quantity) <= 1e-5;
    }
    return false;
}

void Solution::clearCustomer(int customer) {
    const std::vector<Assignment> copy = assignedToCustomer[customer];
    for (const Assignment &a : copy) {
        removeQuantity(customer, a.facility, a.quantity);
    }
}

void Solution::cleanupEmptyFacilities() {
    for (int j = 0; j < inst->m; ++j) {
        closeFacilityIfEmpty(j);
    }
}

int Solution::capacityViolations() const {
    int violations = 0;
    for (int j = 0; j < inst->m; ++j) {
        if (load[j] > inst->capacity[j] + 1e-5) {
            ++violations;
        }
    }
    return violations;
}

int Solution::incompatibilityViolations() const {
    int violations = 0;
    for (int j = 0; j < inst->m; ++j) {
        for (int i : clientsOfFacility[j]) {
            for (int other : inst->incompatibleOf[i]) {
                if (i < other && isServed(other, j)) {
                    ++violations;
                }
            }
        }
    }
    return violations;
}

bool Solution::validate(bool verbose) {
    bool ok = true;
    for (int i = 0; i < inst->n; ++i) {
        double sum = 0.0;
        std::unordered_set<int> seen;
        for (const Assignment &a : assignedToCustomer[i]) {
            sum += a.quantity;
            if (a.quantity <= EPS || a.facility < 0 || a.facility >= inst->m) {
                ok = false;
            }
            if (!isOpen(a.facility) || !isServed(i, a.facility)) {
                ok = false;
            }
            if (!seen.insert(a.facility).second) {
                ok = false;
            }
        }
        if (std::abs(sum - inst->demand[i]) > 1e-5) {
            ok = false;
            if (verbose) {
                std::cerr << "Demanda incumplida cliente " << i << ": " << sum
                          << " != " << inst->demand[i] << '\n';
            }
        }
    }
    for (int j = 0; j < inst->m; ++j) {
        if (load[j] > inst->capacity[j] + 1e-5) {
            ok = false;
            if (verbose) {
                std::cerr << "Capacidad excedida facility " << j << ": " << load[j]
                          << " > " << inst->capacity[j] << '\n';
            }
        }
        if (load[j] > EPS && !isOpen(j)) {
            ok = false;
        }
    }
    const int inc = incompatibilityViolations();
    if (inc != 0) {
        ok = false;
        if (verbose) {
            std::cerr << "Violaciones de incompatibilidad: " << inc << '\n';
        }
    }
    feasible = ok;
    return ok;
}

} // namespace mscflp
