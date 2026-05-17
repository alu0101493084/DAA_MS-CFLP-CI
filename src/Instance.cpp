#include "Instance.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <fstream>
#include <numeric>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace mscflp {

namespace {

std::string readAll(const std::string &path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("No se pudo abrir la instancia: " + path);
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int extractScalar(const std::string &text, const std::string &name) {
    const std::regex re(name + R"(\s*=\s*(-?\d+)\s*;)");
    std::smatch match;
    if (!std::regex_search(text, match, re)) {
        throw std::runtime_error("No se encontro el escalar " + name);
    }
    return std::stoi(match[1].str());
}

std::vector<int> extractNumbersFromSection(const std::string &text, const std::string &name) {
    const std::size_t key = text.find(name);
    if (key == std::string::npos) {
        throw std::runtime_error("No se encontro la seccion " + name);
    }
    const std::size_t eq = text.find('=', key);
    const std::size_t end = text.find("];", eq);
    if (eq == std::string::npos || end == std::string::npos) {
        throw std::runtime_error("Seccion mal formada: " + name);
    }

    const std::string block = text.substr(eq + 1, end - eq - 1);
    std::vector<int> values;
    const std::regex numberRe(R"(-?\d+)");
    for (auto it = std::sregex_iterator(block.begin(), block.end(), numberRe);
         it != std::sregex_iterator(); ++it) {
        values.push_back(std::stoi((*it).str()));
    }
    return values;
}

} // namespace

Instance Instance::load(const std::string &path) {
    const std::string text = readAll(path);
    Instance inst;
    inst.name = basenameNoExtension(path);
    inst.m = extractScalar(text, "Warehouses");
    inst.n = extractScalar(text, "Stores");
    inst.incompatibilities = extractScalar(text, "Incompatibilities");

    const std::vector<int> capacity = extractNumbersFromSection(text, "Capacity");
    const std::vector<int> fixed = extractNumbersFromSection(text, "FixedCost");
    const std::vector<int> goods = extractNumbersFromSection(text, "Goods");
    const std::vector<int> costs = extractNumbersFromSection(text, "SupplyCost");
    const std::vector<int> pairs = extractNumbersFromSection(text, "IncompatiblePairs");

    if (static_cast<int>(capacity.size()) != inst.m || static_cast<int>(fixed.size()) != inst.m) {
        throw std::runtime_error("Dimensiones invalidas en capacidad/coste fijo para " + path);
    }
    if (static_cast<int>(goods.size()) != inst.n) {
        throw std::runtime_error("Dimensiones invalidas en demandas para " + path);
    }
    if (static_cast<int>(costs.size()) != inst.n * inst.m) {
        throw std::runtime_error("La matriz SupplyCost no tiene n*m valores en " + path);
    }
    if (static_cast<int>(pairs.size()) != 2 * inst.incompatibilities) {
        throw std::runtime_error("IncompatiblePairs no coincide con Incompatibilities en " + path);
    }

    inst.capacity.assign(capacity.begin(), capacity.end());
    inst.fixedCost.assign(fixed.begin(), fixed.end());
    inst.demand.assign(goods.begin(), goods.end());
    inst.totalDemand = std::accumulate(inst.demand.begin(), inst.demand.end(), 0.0);

    inst.supplyCost.assign(inst.n, std::vector<int>(inst.m, 0));
    int p = 0;
    for (int i = 0; i < inst.n; ++i) {
        for (int j = 0; j < inst.m; ++j) {
            inst.supplyCost[i][j] = costs[p++];
        }
    }

    inst.incompatibleOf.assign(inst.n, {});
    inst.incompatiblePairs.reserve(inst.incompatibilities);
    for (int k = 0; k < inst.incompatibilities; ++k) {
        int a = pairs[2 * k] - 1;
        int b = pairs[2 * k + 1] - 1;
        if (a < 0 || a >= inst.n || b < 0 || b >= inst.n || a == b) {
            throw std::runtime_error("Par incompatible fuera de rango en " + path);
        }
        inst.incompatiblePairs.emplace_back(a, b);
        inst.incompatibleOf[a].push_back(b);
        inst.incompatibleOf[b].push_back(a);
    }

    inst.buildAuxiliaryOrders();
    return inst;
}

void Instance::buildAuxiliaryOrders() {
    facilitiesByFixedCost.resize(m);
    std::iota(facilitiesByFixedCost.begin(), facilitiesByFixedCost.end(), 0);
    std::stable_sort(facilitiesByFixedCost.begin(), facilitiesByFixedCost.end(),
                     [&](int a, int b) {
                         if (fixedCost[a] != fixedCost[b]) {
                             return fixedCost[a] < fixedCost[b];
                         }
                         return capacity[a] > capacity[b];
                     });

    facilitiesByCustomerCost.assign(n, std::vector<int>(m));
    for (int i = 0; i < n; ++i) {
        std::iota(facilitiesByCustomerCost[i].begin(), facilitiesByCustomerCost[i].end(), 0);
        std::stable_sort(facilitiesByCustomerCost[i].begin(), facilitiesByCustomerCost[i].end(),
                         [&](int a, int b) {
                             if (supplyCost[i][a] != supplyCost[i][b]) {
                                 return supplyCost[i][a] < supplyCost[i][b];
                             }
                             return fixedCost[a] < fixedCost[b];
                         });
    }
}

} // namespace mscflp
