package com.example;

import java.util.Optional;

/**
 * Lightweight GVNS built from greedy construction, local search, and shaking.
 */
public class GvnsSolver {

    private final ConstructiveSolver constructive;
    private final LocalSearchSolver localSearch;

    public GvnsSolver() {
        this.constructive = new ConstructiveSolver();
        this.localSearch = new LocalSearchSolver();
    }

    public Optional<Solution> solve(ProblemInstance instance, int iterations) {
        Optional<Solution> base = constructive.build(instance);
        if (base.isEmpty()) {
            return Optional.empty();
        }

        Solution best = localSearch.improve(instance, base.get());
        double bestCost = best.totalCost(instance);

        Solution current = best.copy();
        for (int it = 0; it < iterations; it++) {
            Solution shaken = shake(instance, current.copy(), it);
            Solution improved = localSearch.improve(instance, shaken);
            double cost = improved.totalCost(instance);
            if (cost < bestCost) {
                best = improved;
                bestCost = cost;
                current = improved.copy();
            }
        }

        return Optional.of(best);
    }

    private Solution shake(ProblemInstance instance, Solution solution, int iteration) {
        // Minimal deterministic shake: move the first feasible customer to a different open facility.
        for (int customer = 0; customer < instance.customerCount(); customer++) {
            int from = solution.assignedFacility(customer);
            if (from < 0) {
                continue;
            }
            for (int to = 0; to < instance.facilityCount(); to++) {
                if (to == from) {
                    continue;
                }
                if (!ConstructiveSolver.canAssign(solution, instance, customer, to)) {
                    continue;
                }
                if ((customer + to + iteration) % 2 == 0) {
                    solution.assignCustomer(customer, to, instance);
                    return solution;
                }
            }
        }
        return solution;
    }
}
