#ifndef MS_CFLP_UTILS_HPP
#define MS_CFLP_UTILS_HPP

#include "RunTypes.hpp"

#include <string>
#include <vector>

namespace mscflp {

std::string basenameNoExtension(const std::string &path);
std::vector<std::string> listInstances(const std::string &directory);
void writeSolution(const Solution &solution, const std::string &path);
std::string csvHeader();
std::string csvLine(const RunResult &result);

} // namespace mscflp

#endif
