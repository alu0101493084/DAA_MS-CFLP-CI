#include "Utils.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace mscflp {

std::string basenameNoExtension(const std::string &path) {
    fs::path p(path);
    return p.stem().string();
}

std::vector<std::string> listInstances(const std::string &directory) {
    std::vector<std::string> files;
    for (const auto &entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".dzn") {
            files.push_back(entry.path().string());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

void writeSolution(const Solution &solution, const std::string &path) {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("No se pudo escribir la solucion: " + path);
    }
    const Instance &inst = *solution.inst;
    out << std::fixed << std::setprecision(6);
    out << "instance=" << inst.name << '\n';
    out << "feasible=" << (solution.feasible ? "true" : "false") << '\n';
    out << "open_facilities=" << solution.openCount() << '\n';
    out << "fixed_cost=" << solution.fixedCost << '\n';
    out << "transport_cost=" << solution.transportCost << '\n';
    out << "total_cost=" << solution.totalCost() << '\n';
    out << "open=[";
    bool first = true;
    for (int j = 0; j < inst.m; ++j) {
        if (solution.isOpen(j)) {
            if (!first) {
                out << ',';
            }
            out << (j + 1);
            first = false;
        }
    }
    out << "]\n";
    out << "assignments customer facility quantity fraction\n";
    for (int i = 0; i < inst.n; ++i) {
        for (const Assignment &a : solution.assignedToCustomer[i]) {
            out << (i + 1) << ' ' << (a.facility + 1) << ' ' << a.quantity << ' '
                << (a.quantity / inst.demand[i]) << '\n';
        }
    }
}

std::string csvHeader() {
    return "instance,algorithm,seed,open_facilities,fixed_cost,transport_cost,total_cost,"
           "incompatibility_violations,capacity_violations,feasible,seconds";
}

std::string csvLine(const RunResult &result) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(6);
    out << result.instance << ',' << result.algorithm << ',' << result.seed << ','
        << result.openFacilities << ',' << result.fixedCost << ',' << result.transportCost << ','
        << result.totalCost << ',' << result.incompatibilityViolations << ','
        << result.capacityViolations << ',' << (result.feasible ? "true" : "false") << ','
        << result.seconds;
    return out.str();
}

} // namespace mscflp
