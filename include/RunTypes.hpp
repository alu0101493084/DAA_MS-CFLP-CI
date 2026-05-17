#ifndef MS_CFLP_RUN_TYPES_HPP
#define MS_CFLP_RUN_TYPES_HPP

#include "Solution.hpp"

#include <string>

namespace mscflp {

struct RunOptions {
    std::string algorithm = "greedy";
    int seed = 1;
    int extraFacilities = 5;
    int rclSize = 3;
    int graspIterations = 30;
    int gvnsIterations = 100;
    int kMax = 4;
    int localSearchPasses = 200;
    double timeLimitSeconds = 0.0;
    bool verbose = false;
};

struct RunResult {
    std::string instance;
    std::string algorithm;
    int seed = 0;
    int openFacilities = 0;
    double fixedCost = 0.0;
    double transportCost = 0.0;
    double totalCost = 0.0;
    int incompatibilityViolations = 0;
    int capacityViolations = 0;
    bool feasible = false;
    double seconds = 0.0;
    Solution solution;
};

} // namespace mscflp

#endif
