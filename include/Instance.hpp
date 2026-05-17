#ifndef MS_CFLP_INSTANCE_HPP
#define MS_CFLP_INSTANCE_HPP

#include <string>
#include <utility>
#include <vector>

namespace mscflp {

struct Instance {
    std::string name;
    int m = 0; // facilities / warehouses
    int n = 0; // customers / stores
    int incompatibilities = 0;

    std::vector<double> capacity;
    std::vector<double> fixedCost;
    std::vector<double> demand;
    std::vector<std::vector<int>> supplyCost; // [customer][facility]
    std::vector<std::pair<int, int>> incompatiblePairs;
    std::vector<std::vector<int>> incompatibleOf;

    std::vector<int> facilitiesByFixedCost;
    std::vector<std::vector<int>> facilitiesByCustomerCost;

    double totalDemand = 0.0;

    static Instance load(const std::string &path);
    void buildAuxiliaryOrders();
};

} // namespace mscflp

#endif
