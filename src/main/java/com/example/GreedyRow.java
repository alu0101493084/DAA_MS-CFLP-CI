package com.example;

import java.util.Optional;

/**
 * Row model for greedy table output.
 */
public record GreedyRow(
        String name,
        int openCount,
        double fixedCost,
        double transportCost,
        double totalCost,
        int incompatibilities,
        double cpuSeconds
) {
    public static GreedyRow from(String name, ProblemInstance instance, Optional<Solution> maybeSolution, double cpuSeconds) {
        if (maybeSolution.isEmpty()) {
            return new GreedyRow(name, 0, 0.0, 0.0, 0.0, 0, cpuSeconds);
        }

        Solution solution = maybeSolution.get();
        solution.totalCost(instance);
        return new GreedyRow(
                name,
                solution.openCount(),
                solution.fixedCostValue(),
                solution.transportCostValue(),
                solution.totalCostValue(),
                solution.incompatibilityCount(instance),
                cpuSeconds
        );
    }
}
