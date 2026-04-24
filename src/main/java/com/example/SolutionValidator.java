package com.example;

/**
 * Feasibility checks for capacities and incompatibilities.
 */
public final class SolutionValidator {

    private SolutionValidator() {
    }

    public static boolean isFeasible(ProblemInstance instance, Solution solution) {
        int customers = instance.customerCount();
        int facilities = instance.facilityCount();

        for (int c = 0; c < customers; c++) {
            if (solution.demandServedValue(c) != instance.demand(c)) {
                return false;
            }
        }

        for (int f = 0; f < facilities; f++) {
            if (solution.load(f) > instance.capacity(f)) {
                return false;
            }
        }

        for (int i = 0; i < customers; i++) {
            for (int j = i + 1; j < customers; j++) {
                if (!instance.incompatible(i, j)) {
                    continue;
                }
                for (int f = 0; f < facilities; f++) {
                    if (solution.x_ij(i, f) > 0 && solution.x_ij(j, f) > 0) {
                        return false;
                    }
                }
            }
        }

        return true;
    }
}
