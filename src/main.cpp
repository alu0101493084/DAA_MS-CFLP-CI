#include "ms_cflp_ci.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace mscflp;

namespace {

const std::string DEFAULT_INSTANCES_DIR = "Instaces_MS-CFLP-CI/Public";

struct Cli {
    std::string instance;
    std::string instancesDir;
    std::string algorithm = "greedy";
    std::string outputCsv;
    std::string solutionPath;
    RunOptions options;
};

struct TableSpec {
    std::string title;
    std::string algorithm;
    std::vector<std::string> headers;
    std::vector<int> widths;
    bool showRcl = false;
    bool showKmax = false;
};

void printHelp(const char *program) {
    std::cout
        << "Uso interactivo:\n"
        << "  " << program << "\n\n"
        << "Uso por argumentos:\n"
        << "  " << program << " --instance ruta.dzn [--algorithm greedy|grasp-constructive|grasp|gvns|all]\n"
        << "  " << program << " --instances-dir dir [--algorithm greedy|grasp-constructive|grasp|gvns|all]\n\n"
        << "Opciones principales:\n"
        << "  --seed N                 Semilla aleatoria (defecto: 1)\n"
        << "  --extra N                Facilities extra del constructivo voraz (defecto: 5)\n"
        << "  --lrc N                  Tamano de lista restringida de candidatos (defecto: 3)\n"
        << "  --grasp-iters N          Iteraciones GRASP (defecto: 30)\n"
        << "  --gvns-iters N           Iteraciones GVNS-RL (defecto: 100)\n"
        << "  --kmax N                 kmax de GVNS (defecto: 4)\n"
        << "  --ls-passes N            Maximo de mejoras locales por VND/RVND (defecto: 200)\n"
        << "  --time-limit S           Limite de tiempo por instancia y algoritmo en segundos\n"
        << "  --solution ruta.txt      Escribe la solucion si se ejecuta una sola instancia/algoritmo\n"
        << "  --help                   Muestra esta ayuda\n";
}

std::string requireValue(int &i, int argc, char **argv, const std::string &option) {
    if (i + 1 >= argc) {
        throw std::runtime_error("Falta valor para " + option);
    }
    return argv[++i];
}

Cli parseArgs(int argc, char **argv) {
    Cli cli;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printHelp(argv[0]);
            std::exit(0);
        } else if (arg == "--instance") {
            cli.instance = requireValue(i, argc, argv, arg);
        } else if (arg == "--instances-dir") {
            cli.instancesDir = requireValue(i, argc, argv, arg);
        } else if (arg == "--algorithm") {
            cli.algorithm = requireValue(i, argc, argv, arg);
            cli.options.algorithm = cli.algorithm;
        } else if (arg == "--seed") {
            cli.options.seed = std::stoi(requireValue(i, argc, argv, arg));
        } else if (arg == "--extra") {
            cli.options.extraFacilities = std::stoi(requireValue(i, argc, argv, arg));
        } else if (arg == "--lrc") {
            cli.options.rclSize = std::stoi(requireValue(i, argc, argv, arg));
        } else if (arg == "--grasp-iters") {
            cli.options.graspIterations = std::stoi(requireValue(i, argc, argv, arg));
        } else if (arg == "--gvns-iters") {
            cli.options.gvnsIterations = std::stoi(requireValue(i, argc, argv, arg));
        } else if (arg == "--kmax") {
            cli.options.kMax = std::stoi(requireValue(i, argc, argv, arg));
        } else if (arg == "--ls-passes") {
            cli.options.localSearchPasses = std::stoi(requireValue(i, argc, argv, arg));
        } else if (arg == "--time-limit") {
            cli.options.timeLimitSeconds = std::stod(requireValue(i, argc, argv, arg));
        } else if (arg == "--output") {
            cli.outputCsv = requireValue(i, argc, argv, arg);
        } else if (arg == "--solution") {
            cli.solutionPath = requireValue(i, argc, argv, arg);
        } else if (arg == "--verbose") {
            cli.options.verbose = true;
        } else {
            throw std::runtime_error("Opcion no reconocida: " + arg);
        }
    }

    if (cli.instance.empty() == cli.instancesDir.empty()) {
        throw std::runtime_error("Indica exactamente una opcion: --instance o --instances-dir");
    }
    if (cli.algorithm != "greedy" && cli.algorithm != "grasp-constructive" &&
        cli.algorithm != "grasp" && cli.algorithm != "gvns" &&
        cli.algorithm != "gvns-rl" && cli.algorithm != "all") {
        throw std::runtime_error("Algoritmo no reconocido: " + cli.algorithm);
    }
    return cli;
}

std::vector<std::string> algorithmsToRun(const std::string &algorithm) {
    if (algorithm == "all") {
        return {"greedy", "grasp-constructive", "grasp", "gvns"};
    }
    return {algorithm};
}

RunResult runSelectedAlgorithm(const Instance &inst, RunOptions options, const std::string &algorithm) {
    options.algorithm = algorithm;
    if (algorithm == "greedy") {
        GreedySolver solver(inst, options);
        return solver.run();
    }
    if (algorithm == "grasp-constructive") {
        GraspConstructiveSolver solver(inst, options);
        return solver.run();
    }
    if (algorithm == "grasp") {
        GraspSolver solver(inst, options);
        return solver.run();
    }
    if (algorithm == "gvns" || algorithm == "gvns-rl") {
        GvnsSolver solver(inst, options);
        return solver.run();
    }
    throw std::runtime_error("Algoritmo no reconocido: " + algorithm);
}

std::string fixedNumber(double value, int decimals = 0) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(decimals) << value;
    return out.str();
}

void printSeparator(const std::vector<int> &widths) {
    std::cout << '+';
    for (int width : widths) {
        std::cout << std::string(width + 2, '-') << '+';
    }
    std::cout << '\n';
}

void printCenteredTitle(const std::string &title, const std::vector<int> &widths) {
    int total = 1;
    for (int width : widths) {
        total += width + 3;
    }
    const int inner = total - 2;
    const int left = std::max(0, (inner - static_cast<int>(title.size())) / 2);
    const int right = std::max(0, inner - left - static_cast<int>(title.size()));
    std::cout << '+' << std::string(inner, '-') << "+\n";
    std::cout << '|' << std::string(left, ' ') << title << std::string(right, ' ') << "|\n";
}

void printRow(const std::vector<std::string> &cells, const std::vector<int> &widths) {
    std::cout << '|';
    for (std::size_t i = 0; i < widths.size(); ++i) {
        const bool left = (i == 0);
        std::cout << ' ';
        if (left) {
            std::cout << std::left << std::setw(widths[i]) << cells[i];
        } else {
            std::cout << std::right << std::setw(widths[i]) << cells[i];
        }
        std::cout << " |";
    }
    std::cout << '\n';
    std::cout.flush();
}

void printTableHeader(const TableSpec &spec) {
    std::cout << '\n';
    printCenteredTitle(spec.title, spec.widths);
    printSeparator(spec.widths);
    printRow(spec.headers, spec.widths);
    printSeparator(spec.widths);
}

std::vector<std::string> resultCells(const TableSpec &spec, const RunResult &result,
                                     const RunOptions &options, int execution) {
    std::vector<std::string> cells;
    cells.push_back(result.instance + ".dzn");
    if (spec.showRcl) {
        cells.push_back(std::to_string(options.rclSize));
        cells.push_back(std::to_string(execution));
    }
    if (spec.showKmax) {
        cells.push_back(std::to_string(options.kMax));
        cells.push_back(std::to_string(execution));
    }
    cells.push_back(std::to_string(result.openFacilities));
    cells.push_back(fixedNumber(result.fixedCost));
    cells.push_back(fixedNumber(result.transportCost));
    cells.push_back(fixedNumber(result.totalCost));
    cells.push_back(std::to_string(result.incompatibilityViolations));
    cells.push_back(fixedNumber(result.seconds, 3));
    return cells;
}

std::vector<std::string> averageCells(const TableSpec &spec, const std::vector<RunResult> &results,
                                      const RunOptions &options) {
    const double count = static_cast<double>(std::max<std::size_t>(1, results.size()));
    double open = 0.0;
    double fixed = 0.0;
    double transport = 0.0;
    double total = 0.0;
    double incomp = 0.0;
    double seconds = 0.0;
    for (const RunResult &result : results) {
        open += result.openFacilities;
        fixed += result.fixedCost;
        transport += result.transportCost;
        total += result.totalCost;
        incomp += result.incompatibilityViolations;
        seconds += result.seconds;
    }

    std::vector<std::string> cells;
    cells.push_back("Promedio");
    if (spec.showRcl) {
        cells.push_back(std::to_string(options.rclSize));
        cells.push_back("-");
    }
    if (spec.showKmax) {
        cells.push_back(std::to_string(options.kMax));
        cells.push_back("-");
    }
    cells.push_back(fixedNumber(open / count, 1));
    cells.push_back(fixedNumber(fixed / count));
    cells.push_back(fixedNumber(transport / count));
    cells.push_back(fixedNumber(total / count));
    cells.push_back(fixedNumber(incomp / count, 1));
    cells.push_back(fixedNumber(seconds / count, 3));
    return cells;
}

RunOptions interactiveOptionsFor(const TableSpec &spec) {
    RunOptions options;
    options.algorithm = spec.algorithm;
    options.seed = 1;
    options.extraFacilities = 5;
    options.rclSize = 3;
    options.kMax = 4;

    if (spec.algorithm == "grasp") {
        options.graspIterations = 8;
        options.localSearchPasses = 60;
        options.timeLimitSeconds = 2.0;
    } else if (spec.algorithm == "gvns") {
        options.gvnsIterations = 15;
        options.localSearchPasses = 60;
        options.timeLimitSeconds = 3.0;
    }
    return options;
}

TableSpec specForOption(int option) {
    if (option == 1) {
        return {
            "Algoritmo Voraz (MS-CFLP-CI)",
            "greedy",
            {"Instancia", "|Jopen|", "Coste Fijo", "Coste Asig.", "Coste Total", "Incomp.", "CPU_Time (s)"},
            {12, 8, 12, 13, 13, 8, 12},
            false,
            false
        };
    }
    if (option == 2) {
        return {
            "Algoritmo GRASP constructivo (Ajuste y Resultados)",
            "grasp-constructive",
            {"Instancia", "|LRC|", "Ejec.", "|Jopen|", "C. Fijo", "C. Asig.", "C. Total", "Incomp.", "CPU_Time"},
            {12, 6, 6, 8, 10, 11, 11, 8, 10},
            true,
            false
        };
    }
    if (option == 3) {
        return {
            "Algoritmo GRASP completo (Ajuste y Resultados)",
            "grasp",
            {"Instancia", "|LRC|", "Ejec.", "|Jopen|", "C. Fijo", "C. Asig.", "C. Total", "Incomp.", "CPU_Time"},
            {12, 6, 6, 8, 10, 11, 11, 8, 10},
            true,
            false
        };
    }
    return {
        "Algoritmo GVNS (Ajuste y Resultados)",
        "gvns",
        {"Instancia", "kmax", "Ejec.", "|Jopen|", "C. Fijo", "C. Asig.", "C. Total", "Incomp.", "CPU_Time"},
        {12, 6, 6, 8, 10, 11, 11, 8, 10},
        false,
        true
    };
}

void runInteractiveAlgorithm(const TableSpec &spec) {
    std::vector<std::string> instances = listInstances(DEFAULT_INSTANCES_DIR);
    if (instances.empty()) {
        throw std::runtime_error("No hay instancias .dzn en " + DEFAULT_INSTANCES_DIR);
    }

    RunOptions options = interactiveOptionsFor(spec);
    std::vector<RunResult> results;
    results.reserve(instances.size());

    std::cout << "\nDirectorio de instancias: " << DEFAULT_INSTANCES_DIR << '\n';
    std::cout << "Ejecutando " << spec.algorithm << " sobre " << instances.size()
              << " instancias...\n";
    if (options.timeLimitSeconds > 0.0) {
        std::cout << "Limite por instancia: " << fixedNumber(options.timeLimitSeconds, 1)
                  << " segundos\n";
    }

    printTableHeader(spec);
    int execution = 1;
    for (const std::string &path : instances) {
        Instance inst = Instance::load(path);
        RunResult result = runSelectedAlgorithm(inst, options, spec.algorithm);
        printRow(resultCells(spec, result, options, execution), spec.widths);
        results.push_back(std::move(result));
    }
    printSeparator(spec.widths);
    printRow(averageCells(spec, results, options), spec.widths);
    printSeparator(spec.widths);
    std::cout << '\n';
}

void interactiveMenu() {
    while (true) {
        std::cout
            << "\n================ MS-CFLP-CI ================\n"
            << "1) Greedy\n"
            << "2) GRASP constructivo\n"
            << "3) GRASP completo\n"
            << "4) GVNS\n"
            << "5) Exit\n"
            << "Seleccione una opcion: ";
        std::cout.flush();

        std::string input;
        if (!std::getline(std::cin, input)) {
            std::cout << '\n';
            return;
        }

        int option = 0;
        try {
            option = std::stoi(input);
        } catch (...) {
            std::cout << "Opcion invalida.\n";
            continue;
        }

        if (option == 5) {
            std::cout << "Saliendo.\n";
            return;
        }
        if (option < 1 || option > 4) {
            std::cout << "Opcion invalida.\n";
            continue;
        }

        try {
            runInteractiveAlgorithm(specForOption(option));
        } catch (const std::exception &ex) {
            std::cerr << "Error: " << ex.what() << '\n';
        }
    }
}

int runCliMode(int argc, char **argv) {
    const Cli cli = parseArgs(argc, argv);
    std::vector<std::string> instances;
    if (!cli.instance.empty()) {
        instances.push_back(cli.instance);
    } else {
        instances = listInstances(cli.instancesDir);
    }
    if (instances.empty()) {
        throw std::runtime_error("No hay instancias .dzn para ejecutar");
    }

    std::ofstream csv;
    if (!cli.outputCsv.empty()) {
        csv.open(cli.outputCsv);
        if (!csv) {
            throw std::runtime_error("No se pudo abrir el CSV de salida: " + cli.outputCsv);
        }
        csv << csvHeader() << '\n';
    }

    std::cout << csvHeader() << '\n';
    RunResult lastResult;
    int resultCount = 0;
    for (const std::string &path : instances) {
        Instance inst = Instance::load(path);
        for (const std::string &algorithm : algorithmsToRun(cli.algorithm)) {
            RunResult result = runSelectedAlgorithm(inst, cli.options, algorithm);
            const std::string line = csvLine(result);
            std::cout << line << '\n';
            if (csv) {
                csv << line << '\n';
            }
            lastResult = std::move(result);
            ++resultCount;
        }
    }

    if (!cli.solutionPath.empty()) {
        if (resultCount != 1) {
            throw std::runtime_error("--solution solo se admite con una instancia y un algoritmo");
        }
        writeSolution(lastResult.solution, cli.solutionPath);
    }
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    try {
        if (argc == 1) {
            interactiveMenu();
            return 0;
        }
        return runCliMode(argc, argv);
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
