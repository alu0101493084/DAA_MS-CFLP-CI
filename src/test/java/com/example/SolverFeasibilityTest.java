package com.example;

import org.junit.jupiter.api.Test;

import java.util.Optional;

import static org.junit.jupiter.api.Assertions.*;

class SolverFeasibilityTest {

    @Test
    void solverReturnsFeasibleSolutionOnSimpleInstance() {
        int[] capacities = {10, 10};
        int[] demands = {4, 4, 2};
        double[] fixed = {4.0, 4.0};
        double[][] service = {
                {1.0, 6.0, 1.0},
                {6.0, 1.0, 1.0}
        };
        boolean[][] incompatible = new boolean[3][3];
        incompatible[0][1] = true;
        incompatible[1][0] = true;

        ProblemInstance instance = new ProblemInstance(capacities, demands, fixed, service, incompatible);
        Optional<Solution> maybe = new MsCflpCiSolver().solve(instance);

        assertTrue(maybe.isPresent(), "Solver should find a feasible solution");
        assertTrue(SolutionValidator.isFeasible(instance, maybe.get()), "Solution must satisfy constraints");
    }

    @Test
    void validatorDetectsIncompatibilityViolation() {
        int[] capacities = {10};
        int[] demands = {3, 3};
        double[] fixed = {1.0};
        double[][] service = {{1.0, 1.0}};
        boolean[][] incompatible = {
                {false, true},
                {true, false}
        };
        ProblemInstance instance = new ProblemInstance(capacities, demands, fixed, service, incompatible);

        Solution invalid = new Solution(1, 2);
        invalid.assignCustomer(0, 0, instance);
        invalid.assignCustomer(1, 0, instance);

        assertFalse(SolutionValidator.isFeasible(instance, invalid));
    }
}
