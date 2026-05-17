#ifndef MS_CFLP_SOLUTION_HPP
#define MS_CFLP_SOLUTION_HPP

#include "Instance.hpp"

#include <string>
#include <vector>

namespace mscflp {

struct Assignment {
    int facility = -1;
    double quantity = 0.0;
};

struct Solution {
    const Instance *inst = nullptr;
    std::vector<unsigned char> open;
    std::vector<double> load;
    std::vector<std::vector<Assignment>> assignedToCustomer;
    std::vector<std::vector<int>> clientsOfFacility;
    std::vector<unsigned char> served; // flattened [customer * m + facility]

    double fixedCost = 0.0;
    double transportCost = 0.0;
    bool feasible = false;
    std::string note;

    Solution() = default;
    explicit Solution(const Instance *instance);

    double totalCost() const { return fixedCost + transportCost; }
    int openCount() const;
    int incompatibilityViolations() const;
    int capacityViolations() const;
    bool validate(bool verbose = false);

    bool isOpen(int facility) const { return open[facility] != 0; }
    bool isServed(int customer, int facility) const;
    bool canServe(int customer, int facility) const;
    bool canServeIgnoring(int customer, int facility, int ignoredCustomer) const;
    double residualCapacity(int facility) const;

    void openFacility(int facility);
    void closeFacilityIfEmpty(int facility);
    void addQuantity(int customer, int facility, double quantity);
    bool removeQuantity(int customer, int facility, double quantity);
    void clearCustomer(int customer);
    void cleanupEmptyFacilities();
};

} // namespace mscflp

#endif
